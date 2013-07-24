/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/FiducialDetectorPluginForQua **
**          ds.h                                                   **
** Module : ICLMarkers                                             **
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

#include <ICLMarkers/FiducialDetectorPlugin.h>
#include <ICLMarkers/QuadDetector.h>

namespace icl{
  namespace markers{
    
    /** \cond */
    class FiducialDetector;
    /** \endcond */
    
    
    /// FiducialDetectorPlugin for quad-markers like ARToolkit and BCH-Code markers \ingroup PLUGINS
    class FiducialDetectorPluginForQuads : public FiducialDetectorPlugin{
      /// Internal Data
      struct Data;
      
      /// Internal data pointer
      Data *data;
  
      protected:
  
      /// only the FiducialDetector can instantiate this class
      FiducialDetectorPluginForQuads();
      public:
  
      /// This class cannot be used 
      friend class icl::markers::FiducialDetector;
  
      /// Destructor
      ~FiducialDetectorPluginForQuads();
      
      // use the edges
      virtual void getKeyPoints2D(std::vector<Fiducial::KeyPoint> &dst, FiducialImpl &impl);
      virtual void getFeatures(Fiducial::FeatureSet &dst);
      virtual void detect(std::vector<FiducialImpl*> &dst, const core::Img8u &image);
  
      /// this plugin uses the binarisation from the internally used quad-detector
      virtual SourceImageType getPreProcessing() const {  return Gray;  }
      
      /// loads markers ID's (also implemented in the subclasses)
      /** @param add
          @param which this any instance can be ...
          * of type int (then, only the corresponding marker ID is loaded) 
          * of type utils::Range32s "[a,b]", (then all markers within the range are loaded)
          * of something like {a,b,c,d,...} then all marker IDs in the list are loaded
  
          Please note that other types might be interpreted in the wrong way.
          Mandatory parameter is "size". Please refer to the 
          documentation of icl::markers::FiducialDetector::loadMarkers for more details
         @param params
      */
      virtual void addOrRemoveMarkers(bool add, const utils::Any &which, const utils::ParamList &params) = 0;
  
      /// this plugin provides some extra intermediate images
      std::string getIntermediateImageNames() const;
      
      /// returns the intermediate image, that is associated with the given name
      /** @see getIntermediateImageNames for more details */
      const core::ImgBase *getIntermediateImage(const std::string &name) const throw (utils::ICLException);
  
      /// this method is called before the patch classification loop is started
      /** this function can be used to avoid property extraction at runtime. Usually,
          a certain implementation can read out and store all property values that are used
          in classify patch once in a whole image processing cylce */
      virtual void prepareForPatchClassification(){}
  
      /// this method must be called in the subclasses
      virtual FiducialImpl *classifyPatch(const core::Img8u &image, int *rot, bool returnRejectedQuads, cv::ImageRegion r) = 0;
      
      /// this method is also implemented in the subclasses
      /** The method describes the parameters for the marker rectification */
      virtual void getQuadRectificationParameters(utils::Size &markerSizeWithBorder,
                                                  utils::Size &markerSizeWithoutBorder) = 0;
      
      /// returns the internal quad-detector
      QuadDetector& getQuadDetector();

    };
  } // namespace markers
}

