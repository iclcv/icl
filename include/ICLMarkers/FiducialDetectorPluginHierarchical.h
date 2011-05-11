/**********************************************************************
**                Image Component Library (ICL)                      **
**                                                                   **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld            **
**                         Neuroinformatics Group                    **
** Website: www.iclcv.org and                                        **
**          http://opensource.cit-ec.de/projects/icl                 **
**                                                                   **
** File   : include/ICLMarkers/FiducialDetectorPluginHierarchical.h  **
** Module : ICLBlob                                                  **
** Authors: Christof Elbrechter                                      **
**                                                                   **
**                                                                   **
** Commercial License                                                **
** ICL can be used commercially, please refer to our website         **
** www.iclcv.org for more details.                                   **
**                                                                   **
** GNU General Public License Usage                                  **
** Alternatively, this file may be used under the terms of the       **
** GNU General Public License version 3.0 as published by the        **
** Free Software Foundation and appearing in the file LICENSE.GPL    **
** included in the packaging of this file.  Please review the        **
** following information to ensure the GNU General Public License    **
** version 3.0 requirements will be met:                             **
** http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                   **
** The development of this software was supported by the             **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.      **
** The Excellence Cluster EXC 277 is a grant of the Deutsche         **
** Forschungsgemeinschaft (DFG) in the context of the German         **
** Excellence Initiative.                                            **
**                                                                   **
***********************************************************************/

#ifndef ICL_FIDUCIAL_DETECTOR_PLUGIN_HIERARCHICAL_H
#define ICL_FIDUCIAL_DETECTOR_PLUGIN_HIERARCHICAL_H

#include <ICLBlob/ImageRegion.h>

#include <ICLMarkers/FiducialDetectorPlugin.h>
#include <ICLMarkers/RegionStructure.h>

namespace icl{
  

  /// Extra abstraction layer that defines a basic skeleton for the detection of hierarchical fiducials \ingroup PLUGINS
  class FiducialDetectorPluginHierarchical : public FiducialDetectorPlugin{
    /// internal data class
    struct Data;
    
    /// internal hidden data pointer
    Data *data;
    
    protected:
    
    /// Constructor
    FiducialDetectorPluginHierarchical();
    public:
    
    /// destructor
    ~FiducialDetectorPluginHierarchical();
    
    /// defines which features are supported
    virtual void getFeatures(Fiducial::FeatureSet &dst)=0;
    
    /// defines how to detect markers from a given image
    /** In this case, regions are detected using the internal region detector.
        The regions are then passed to the other detect method */
    virtual void detect(std::vector<FiducialImpl*> &dst, const Img8u &image);

    /// defines how to find makers in the given vector of regions
    virtual void detect(std::vector<FiducialImpl*> &dst, const std::vector<ImageRegion> &regions) = 0;
    
    /// defines how to load/remove marker definitions
    virtual void addOrRemoveMarkers(bool add, const Any &which, const ParamList &params)=0;
  };
  
}

#endif
