/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/FixedConverter.h                   **
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
#include <ICLCore/Converter.h>
#include <ICLUtils/Uncopyable.h>

namespace icl{
  namespace core{

    /// Special converter "producing" images with fixed parameters
    /** This class can be used to convert images with arbitrary params
        and depth to well defined fixed params and depth. Its functionality
        bases on the icl::Converter class which is created as a member of
        the FixedConverter.
        @see icl::Converter
    **/
    class ICLCore_API FixedConverter : public utils::Uncopyable{
      public:
      /// Create a new FixedConverter Object with given destination params and depth
      /** @param p output image parameters
          @param d output image depth
          @param applyToROIOnly decides wheater to apply the conversion on the
                                whole source image or only on the source images ROI
      **/
      FixedConverter(const ImgParams &p, depth d=depth8u, bool applyToROIOnly=false);

      /// Converts the source image into the given destination image
      /** The given destination image pointer is adapted in its depth and parameters
          before apply function of the Converter member is called.
          @param poSrc source image
          @param ppoDst destination image, null is not allowed! If it points to NULL,
                        the a appropriate destination image is created at *ppoDst
                        else it is adapted to the parameters of the FixedConverter object
      **/
      void apply(const ImgBase *poSrc, ImgBase **ppoDst);

      /// set up wheater to apply on the source image or on the source images ROI
      /** @param applyToROIOnly given flag
      **/
      void setApplyToROIOnly(bool applyToROIOnly){ m_oConverter.setApplyToROIOnly(applyToROIOnly); }

      /// sets up the output image parameters
      /** @param p output image parameters
      **/
      void setParams(const ImgParams &p) { m_oParams = p; }

      /// sets up the order to use when applying several conversion internally
      /** @param o new operation order
          @see Converter
      **/
      void setOperationOrder(Converter::oporder o){ m_oConverter.setOperationOrder(o); }

      /// sets a new scalemode (default is interpolateNN)
      void setScaleMode(scalemode scaleMode){ m_oConverter.setScaleMode(scaleMode); }
      private:

      /// destination image parameters
      ImgParams m_oParams;

      /// Converter to apply conversion
      Converter m_oConverter;

      /// destination image depth
      depth m_eDepth;
    };

  } // namespace core
}
