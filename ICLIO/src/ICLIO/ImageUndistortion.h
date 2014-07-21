/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/ImageUndistortion.h                    **
** Module : ICLIO                                                  **
** Authors: Christian Groszewski                                   **
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

namespace icl{
  namespace io{
  
    class ICLIO_API ImageUndistortion{
      public:
      struct Impl; //!< internal impl
      
      private:
      Impl *impl;  //!< internal impl pointer
      
      
      public:
      /// creates a null instance
      ImageUndistortion();
      
      /// creates an Undistortion instance given parameters
      /** @param model distortion mode possible values are MatlabModel5Params and SimpleARTBased
          @param params parameters for the given model
            (MatlabModel5Params needs 10 parameters: fx, fy, ix, iy, skew, k1, k2, k3, k4, k5; 
            SimpleARTBased needs 4 parameters: x, y, f, scale)
          @param imageSize underlying image size */
      ImageUndistortion(const std::string &model, const std::vector<double> &params,
                        const utils::Size &imageSize);
      
      /// copy constructor
      ImageUndistortion(const ImageUndistortion &other);
      
      /// assignment operator
      ImageUndistortion &operator=(const ImageUndistortion &other);
      
      /// loads ImageUndistortion from file using the istream operator
      ImageUndistortion(const std::string &filename);
      
      /// returns current image size
      const utils::Size &getImageSize() const;
      const std::vector<double> &getParams() const;
      const std::string &getModel() const;
      const utils::Point32f operator()(const utils::Point32f &distortedPos) const;
      const core::Img32f &createWarpMap() const;
      
      inline bool isNull() const { return !impl; }
    };
    
    /// overloaded ostream operator for ImageUndistortion instances 
    ICLIO_API std::istream &operator>>(std::istream &is, ImageUndistortion &udist);
    
    /// overloaded istream operator for ImageUndistortion instances 
    ICLIO_API std::ostream &operator<<(std::ostream &s, const ImageUndistortion &udist);
  } // namespace io
}

