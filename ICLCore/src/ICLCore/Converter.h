/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/Converter.h                        **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
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
#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>


namespace icl{
  namespace core{
    /// General Image Converter
    /** The Converter wraps and summarizes all image conversion routines,
        including depth change, scaling and color conversion. It provides all
        necessary buffers to do several of these changes in series. Simply
        provide the desired output format to dst of the apply function,
        and this method will select the appropriate conversion steps.\n
        If demanded, the converter provides an interface to specify the order
        of the different operations.
    **/
    class ICLCore_API Converter : public utils::Uncopyable{
      public:
      /// enum to define internal used operation order
      enum oporder{
        orderScaleConvertCC = 12,
        orderScaleCCConvert = 21,
        orderConvertScaleCC = 102,
        orderConvertCCScale = 201,
        orderCCScaleConvert = 120,
        orderCCConvertScale = 210
      };

      /// constructor
      /** @param applyToROIOnly if true, the source images ROI only is
                                used instead of the whole src image.
      */
      Converter(bool applyToROIOnly=false);

      /// other constructor
      /** @param o operation order
          @param applyToROIOnly if true, the source images ROI only is
                                used instead of the whole src image.
      **/
      Converter(oporder o, bool applyToROIOnly=false);

      /// creates a converter, and converts the srcImage to dstImage immediately
      Converter(const ImgBase *srcImage, ImgBase *dstImage, bool applyToROIOnly=false);

      /// destructor
      ~Converter();

      /// image conversion function
      /**
          Although this function looks like the iclcc function located in the
          iclcc.h,  it brings some additional functionalities (see class description).
          @param src source image
          @param dst destination image
      **/

      /// transfers the image data from the source image into the image data of the destination image
      /** @param src source image
          @param dst destination image
      **/
      void apply(const ImgBase *src, ImgBase *dst);

      /// sets up the converter to apply operations on the source images roi only
      /** @see Converter(bool,bool) */
      void setApplyToROIOnly(bool applyToROIOnly){ m_bROIOnly = applyToROIOnly; }


      /// Sets up the operation order for this converter
      /** In many cases it is necessary to apply two or three different operations
          after another to convert the source image into the given destination image.
          Possible operation are:
          - scaling
          - depth conversion
          - color conversion
          In some cases it might be useful to force the Converter to begin the
          conversion procedure with scaling before e.g. a depth conversion is
          applied. This can speed up the performance much if the image is scaled
          down befor applying the depth conversion. In another scenario, the image
          is scaled up: In this case, the depth conversion should have been made
          before the scaling operation.
          <b>Note:</b> As the color conversion is able to apply an implicit depth
          conversion, this two step are ordered internally by regarding the currently
          given oporder.
          @param o new operation order definition
      **/
      void setOperationOrder(oporder o){ m_eOpOrder = o; }

      /// sets a new scale interpolation method (default is interpolateNN)
      void setScaleMode(scalemode scaleMode);

      private:
      /// converts dependent on the destination images depth
      void dynamicConvert(const ImgBase *src, ImgBase *dst);

      /// internally used conversion function
      /** This function wraps the icl::cc function and optimizes its
          performance by using the Converter objects internally hold
          color conversion buffer for "emulated" color conversions.
          (E.g. HLStoYUV, is emulated by HLSToRGB followed by RGBToYUV.
          icl::cc stores the result of HLSToRGB in a temporarily
          allocated and released image buffer. The Converter uses an
          persistent buffer of the Converter object.) This will speed
          up cross format conversions in looped applications.
          @param src source image
          @param dst destination image
      */
      void cc(const ImgBase *src, ImgBase *dst);

      /// Buffer for size conversion
      ImgBase *m_poSizeBuffer;

      /// Buffer for emulated color conversin using Converter::cc(...)
      ImgBase *m_poCCBuffer;

      /// Buffer for depth conversion
      ImgBase *m_poDepthBuffer;

      /// Buffer for ROI extraction
      ImgBase *m_poROIBuffer;

      /// Buffer for color conversion
      ImgBase *m_poColorBuffer;

      /// flag that indicates whether to work on source images ROI or on the whole sorce image
      bool  m_bROIOnly;

      /// currently set operation order
      /** @see setOperationOrder */
      oporder m_eOpOrder;

      /// internal scalemode
      scalemode m_scaleMode;
    };
  } // namespace core
}
