// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Andre Justus

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>
#include <ICLFilter/NeighborhoodOp.h>
#include <ICLCore/Image.h>
#include <ICLCore/ImageBackendDispatching.h>


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
    class ICLFilter_API MorphologicalOp : public NeighborhoodOp, public core::ImageBackendDispatching {
    public:
    MorphologicalOp(const MorphologicalOp&) = delete;
    MorphologicalOp& operator=(const MorphologicalOp&) = delete;


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

      /// Backend selector keys
      enum class Op : int { apply };

      /// Dispatch signature: src, dst, op reference
      using MorphSig = void(const core::Image&, core::Image&, MorphologicalOp&);

      /// Class-level prototype — owns selectors, populated during static init
      static core::ImageBackendDispatching& prototype();

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
      const icl8u* getMask() const;

      /// returns mask size
      utils::Size getMaskSize() const;

      void setOptype(optype type);

      /// returns the type of the selected morphological operation
      optype getOptype() const;

      /// Performs morph of an image with given optype and mask.
      void apply(const core::Image &src, core::Image &dst) override;

      /// Import unaryOps apply function without destination image
      using UnaryOp::apply;

      /// Monotonically increasing counter, incremented on mask/optype changes.
      /// Used by backends to detect when state needs reinitialization.
      unsigned maskVersion() const { return m_maskVersion; }

      // Internal buffer access for backend files
      core::Image& openingBuffer() { return m_openingAndClosingBuffer; }
      core::Image& gradientBuffer1() { return m_gradientBorderBuffer_1; }
      core::Image& gradientBuffer2() { return m_gradientBorderBuffer_2; }

    private:
      icl8u *m_pcMask;
      utils::Size m_oMaskSizeMorphOp;
      optype m_eType;
      unsigned m_maskVersion = 0;

      // C++ composite operation buffers
      core::Image m_openingAndClosingBuffer;
      core::Image m_gradientBorderBuffer_1;
      core::Image m_gradientBorderBuffer_2;
    };

    /// ADL-visible toString for MorphologicalOp::Op (defined in MorphologicalOp.cpp)
    ICLFilter_API const char* toString(MorphologicalOp::Op op);

  } // namespace filter
} // namespace icl
