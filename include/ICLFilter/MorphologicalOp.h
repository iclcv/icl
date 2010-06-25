/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/MorphologicalOp.h                    **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef MORPHOLOGICAL_OP_H
#define MORPHOLOGICAL_OP_H

#include <ICLFilter/NeighborhoodOp.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>


namespace icl {
  
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
  class MorphologicalOp : public NeighborhoodOp, public Uncopyable {
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
  MorphologicalOp (optype t, const Size &maskSize=Size(3,3), const icl8u *mask=0);
    
    /// Destructor
    ~MorphologicalOp ();
  
    /// Change mask
    void setMask (Size size, const icl8u* pcMask=0);
    
    /// returns mask
    /** 
      @return mask
    */
    const icl8u* getMask() const;

    /// returns mask size
    /** 
      @return mask size
    */
    Size getMaskSize() const;
    
    void setOptype(optype type);
    
    /// returns the type of the selected morphological operation
    /** 
      @return optype
    */
    optype getOptype() const;
    
    /// Performs morph of an image with given optype and mask.
    void apply (const ImgBase *poSrc, ImgBase **ppoDst);
    
    /// Import unaryOps apply function without destination image
    UnaryOp::apply;
    
#ifdef HAVE_IPP
  private:

    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, const Ipp8u*, IppiSize, IppiPoint)>
    IppStatus ippiMorphologicalCall (const Img<T> *src, Img<T> *dst);
    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize)>
    IppStatus ippiMorphologicalCall3x3 (const Img<T> *src, Img<T> *dst);

    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, _IppiBorderType, IppiMorphState*)>
    IppStatus ippiMorphologicalBorderReplicateCall (const Img<T> *src, Img<T> *dst,IppiMorphState *state);

    template<typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, IppiBorderType, IppiMorphAdvState*)>
    IppStatus ippiMorphologicalBorderCall (const Img<T> *src, Img<T> *dst, IppiMorphAdvState *advState);

    typedef IppiMorphState ICLMorphState ;
    typedef IppiMorphAdvState ICLMorphAdvState;
#else
    typedef void ICLMorphState;
    typedef void ICLMorphAdvState;
    ImgBase *m_openingAndClosingBuffer;
    ImgBase *m_gradientBorderBuffer_1;
    ImgBase *m_gradientBorderBuffer_2;

    template<class T>
    void apply_t(const ImgBase *src, ImgBase **dst);
#endif
  private:
    icl8u * m_pcMask;
    Size m_oMaskSizeMorphOp; // actually masksize of NeighborhoodOp and MorphOp may be different
    ICLMorphState* m_pState8u;
    ICLMorphState* m_pState32f;
    ICLMorphAdvState* m_pAdvState8u;
    ICLMorphAdvState* m_pAdvState32f;
    bool m_bMorphState8u;
    bool m_bMorphState32f;
    bool m_bMorphAdvState8u;
    bool m_bMorphAdvState32f;
    bool m_bHas_changed;
    bool m_bHas_changedAdv;
    void deleteMorphStates();
    void checkMorphAdvState8u(const Size roiSize);
    void checkMorphAdvState32f(const Size roiSize);
    void checkMorphState8u(const Size roiSize);
    void checkMorphState32f(const Size roiSize);
  
    
    optype m_eType;

  };
} // namespace icl

#endif
