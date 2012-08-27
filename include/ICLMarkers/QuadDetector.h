/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/QuadDetector.h                      **
** Module : ICLCV                                                **
** Authors: Christof Elbrechter                                    **
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

#ifndef ICL_QUAD_DETECTOR_H
#define ICL_QUAD_DETECTOR_H

#include <ICLUtils/Configurable.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/ImgBase.h>

#include <ICLMarkers/TiltedQuad.h>

namespace icl{
  namespace markers{
    
    /// Tool-class for detecting tilted quads in images
    /** The quad detector combines usual steps that are used
        to find quad-like structures in input images.\n
        
        \section GEN General Information
  
        The QuadDetector combines a local threshold preprocessor, optional
        further preprocessing steps such as median or morphological
        operations with an icl::RegionDetector based search for
        regions with 4 corners.
        
        \section CONF Configurable interface
        The QuadDetector forwards the local-threshold and the 
        RegionDetector options. It also adds some extra properties
        for the post-processing the local-threshold result image before
        it's passed to the region detector internally
    */
    class QuadDetector : public Configurable, public Uncopyable{
      
      /// Internal Data class
      class Data;
      
      /// Internal data pointer (hidden)
      Data *data;
  
      public:
      
      /// enum, that helps to specify what quads are searched in the threshold-result image
      enum QuadColor{
        BlackOnly,    //!< only quads that are black (default, value 0)
        WhiteOnly,    //!< only quads that are white (value 255)
        BlackAndWhite //!< white and black quads
      };
  
      /// Base constructor
      /** @param c the detected quads binary value 
          @param dynamic if this is set to true, there will be a changable
                         property for the quad color, otherwise, the initial
                         value will remain fixed
  
          */
      QuadDetector(QuadColor c = BlackOnly, bool dynamic=false);
      
      /// Destructor
      ~QuadDetector();
  
      /// apply-method, that extracts quad-like structures in the input image
      /** This method first applys a local threshold to the given input image, 
          which results in a binary icl8u-image. This image is then optionally
          processed by a median and/or by some morphological operations */
      const std::vector<TiltedQuad> &detect(const ImgBase *image);
  
      /// returns the last binary image that was produced internally
      const Img8u &getLastBinaryImage() const;
    };
    
    
    /// ostream operator for QuadDetector::QuadColor instances
    std::ostream &operator<<(std::ostream &s, const QuadDetector::QuadColor &c);
  
    /// istream operator for QuadDetector::QuadColor instances
    std::istream &operator>>(std::istream &s, QuadDetector::QuadColor &c);
  
  } // namespace markers
}

#endif
