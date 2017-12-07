/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/MorphologicalOp.h              **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>
#include <ICLFilter/NeighborhoodOp.h>


namespace icl {
  namespace filter{

    /// Class for Morphological operations  \ingroup UNARY \ingroup NBH
    /** (Only available for Img8u and Img32f, IPP and fallback implementation)

        \section DST_SIZE Destination Image Sizes
        Destination image ROI size depends not only on given input ROI size
        and mask-size, but also on the used optype.
        In case of default operations (dilate, erode, ... destination ROI
        size is calculated as in the top level NeighborhoodOp class.
        <b>But Note:</b> for dilate and erode border, destination image roi
        size becomes <b>equal</b> to the source images one if using IPP
        support. The fallback C++ implementation uses internally other instaces
        of MorphologicalOp to apply e.g. an erosion operation, hence,
        destination roi size is adapted in that case as one might expect.


        \section OP Operations
        The basic operations dilatation and erosion are not implemented as
        hit-or-miss transformation, but as gray-level morphologic operators.
        Each operator works with a binary mask which is moved successively
        through the source images ROI.
        At each pixel location. All pixel values within the mask boundaries,
        where the corresponding mask-entry differs from zero are evaluated
        as follows:
        -# <b>dilatation</b>: destination image pixel becomes the the maximum
           pixel of all pixels within mask
        -# <b>erosion</b>: destination image pixel becomes the the minimum
           pixel of all pixels within mask
        -# <b>erosion3x3 and dilatation3x3</b>: this is just a shortcut
           for using a 3x3 mask where all entries are set to 1. IPP obviously
           does a lot of optimizations here, fallback doesn't
        -# <b>dilate/erode border replicate</b>: as standard operation, except
           copying border pixels from closes valid computed pixels (not tested
           well in fallback case)
        -# <b>opening</b> erosion followed by a dilatation-step
        -# <b>closing</b> dilatation followed by an erosion-step
        -# <b>tophat</b> source image  minus opening result
        -# <b>blackhat</b> closing result - source image
        -# <b>gradient</b> closing result - opened result

        \section EX Examples
        As a useful help, some example images are shown here:

        <b>left: binary image results, right: gray image results</b>
        \image html  morphologic_operator_results.png
    */
    class ICLFilter_API MorphologicalOp : public NeighborhoodOp, public utils::Uncopyable {
    public:

    /// this enum specifiy all possible morphological operations
    enum optype {
      dilate=0,
      erode=1,
      dilate3x3=2,
      erode3x3=3,
      dilateBorderReplicate=4,
      erodeBorderReplicate=5,
      openBorder=6,
      closeBorder=7,
      tophatBorder=8,
      blackhatBorder=9,
      gradientBorder=10
    };
      /// Constructor that creates a Morphological object, with specified mask size
      /** @param t operation type if(dilate3x3 or erode3x3), further arguments can be
                   left out
          @param maskSize not used if t is dilate3x3 or erode3x3. maskSie must be
                          positive in width and height. If widht or height is even,
                          the next larger odd integer is used (otherwise IPP fails)

          @param mask  If != NULL, only pixels within that mask that are not 0 are
                       are used
      */
    MorphologicalOp (optype t, const utils::Size &maskSize=utils::Size(3,3), const icl8u *mask=0);

    /// constructor with given optype as string
    /** The string ids are identical to the optype enumeration value names */
    MorphologicalOp (const std::string &optype="erode", const utils::Size &maskSize=utils::Size(3,3), const icl8u *mask=0);

      /// Destructor
      ~MorphologicalOp ();

      /// Change mask
      void setMask (utils::Size size, const icl8u* pcMask=0);

      /// returns mask
      /**
        @return mask
      */
      const icl8u* getMask() const;

      /// returns mask size
      /**
        @return mask size
      */
      utils::Size getMaskSize() const;

      void setOptype(optype type);

      /// returns the type of the selected morphological operation
      /**
        @return optype
      */
      optype getOptype() const;

      /// Performs morph of an image with given optype and mask.
      void apply (const core::ImgBase *poSrc, core::ImgBase **ppoDst);

      /// Import unaryOps apply function without destination image
      using UnaryOp::apply;

  #ifdef ICL_HAVE_IPP
    private:

/*
      template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphAdvState*)>
      IppStatus ippiMorphologicalBorderCall (const core::Img<T> *src, core::Img<T> *dst, IppiMorphAdvState *advState);
*/
      typedef IppiMorphState ICLMorphState ;
      typedef IppiMorphAdvState ICLMorphAdvState;
  #else
      typedef void ICLMorphState;
      typedef void ICLMorphAdvState;
      core::ImgBase *m_openingAndClosingBuffer;
      core::ImgBase *m_gradientBorderBuffer_1;
      core::ImgBase *m_gradientBorderBuffer_2;

    private:
      template<class T>
      void apply_t(const core::ImgBase *src, core::ImgBase **dst);
  #endif
    private:
      icl8u * m_pcMask;
      utils::Size m_oMaskSizeMorphOp; // actually masksize of NeighborhoodOp and MorphOp may be different
      ICLMorphState* m_pState8u;
      ICLMorphState* m_pState32f;
      ICLMorphAdvState* m_pAdvState8u;
      ICLMorphAdvState* m_pAdvState32f;
  #ifdef ICL_HAVE_IPP
      Ipp8u *m_pAdvBuf;
      Ipp8u *m_pBuf;
  #endif
      bool m_bMorphState8u;
      bool m_bMorphState32f;
      bool m_bMorphAdvState8u;
      bool m_bMorphAdvState32f;
      bool m_bHas_changed;
      bool m_bHas_changedAdv;
      void deleteMorphStates();
      void checkMorphAdvState8u(const utils::Size roiSize);
      void checkMorphAdvState32f(const utils::Size roiSize);
      void checkMorphState8u(const utils::Size roiSize);
      void checkMorphState32f(const utils::Size roiSize);


      optype m_eType;

    };
  } // namespace filter
} // namespace icl

