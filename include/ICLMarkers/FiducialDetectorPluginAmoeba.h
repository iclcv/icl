/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/FiducialDetectorPluginAmoeba.h      **
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

#pragma once

#include <ICLMarkers/FiducialDetectorPluginHierarchical.h>

namespace icl{
  namespace markers{
    
    /// FiducialDetectorPlugin for reacTIVision's 'amoeba' markers\ingroup PLUGINS
    class FiducialDetectorPluginAmoeba : public FiducialDetectorPluginHierarchical{
      /// internal data class
      class Data;
      
      /// hidden data pointers
      Data *data;
      
      /// avoid instantiation except for friends
      FiducialDetectorPluginAmoeba();
      public:
      /// this class can only be instantiated by the FiducialDetector class
      friend class FiducialDetector;
  
      /// Destructor
      ~FiducialDetectorPluginAmoeba();
      
      /// this is the only feature that is computed in a deferred way
      /** Returns the region boundary */
      virtual void getCorners2D(std::vector<utils::Point32f> &dst, FiducialImpl &impl);
  
      /// deferred rotation calculation
      virtual void getRotation2D(float &dst, FiducialImpl &impl);
  
      /// defines which features are supported
      virtual void getFeatures(Fiducial::FeatureSet &dst);
  
      /// defines how to find makers in the given vector of regions
      virtual void detect(std::vector<FiducialImpl*> &dst, const std::vector<ImageRegion> &regions);
      
      /// defines how to load/remove marker definitions
      /** The Any paramter 'which' can either be a filename to a file that contains
          TwoLevelRegionStructure codes per row,
          or a newline or comma or space separated list of 
          TwoLevelRegionStructure codes. The ParamList params is not used here.
      */
      virtual void addOrRemoveMarkers(bool add, const Any &which, const ParamList &params);
    };
  } // namespace markers
}

