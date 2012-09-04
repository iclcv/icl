/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLIO/ImageUndistortion.h                      **
 ** Module : ICLIO                                                  **
 ** Authors: Christian Groszewski                                   **
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
#pragma once

#include <ICLCore/Img.h>

namespace icl{
  namespace io{
  
    class ImageUndistortion{
      public:
      struct Impl; //!< internal impl
      
      private:
      Impl *impl;  //!< internal impl pointer
      
      
      public:
      /// creates a null instance
      ImageUndistortion();
      
      /// creates an Undistortion instance given parameters
      /** @param model distortion mode possible values are MatlabModel5Params and SimpleARTBased
          @param params parameters for the given model (MatlabModel5Params needs 5 parameters, 
          SimpleARTBased needs 3 parameters)
          @param imageSize underlying image size */
      ImageUndistortion(const std::string &model, const std::vector<double> &params,
                        const utils::Size &imageSize);
      
      /// copy constructor
      ImageUndistortion(const ImageUndistortion &other);
      
      /// assignment operator
      ImageUndistortion &operator=(const ImageUndistortion &other);
      
      /// loads ImageUndistortion from file using the istream operator
      ImageUndistortion(const std::string &filename);
      
      /// returns curren timage size
      const utils::Size &getImageSize() const;
      const std::vector<double> &getParams() const;
      const std::string &getModel() const;
      const utils::Point32f operator()(const utils::Point32f &distortedPos) const;
      const core::Img32f &createWarpMap() const;
      
      inline bool isNull() const { return !impl; }
    };
    
    /// overloaded ostream operator for ImageUndistortion instances 
    std::istream &operator>>(std::istream &is, ImageUndistortion &udist);
    
    /// overloaded istream operator for ImageUndistortion instances 
    std::ostream &operator<<(std::ostream &s, const ImageUndistortion &udist);
  } // namespace io
}

