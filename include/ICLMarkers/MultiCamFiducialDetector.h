/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/MultiCamFiducialDetector.h          **
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

#ifndef ICL_MULTI_CAM_FIDUCIAL_DETECTOR_H
#define ICL_MULTI_CAM_FIDUCIAL_DETECTOR_H

#include <ICLMarkers/MultiCamFiducial.h>
#include <ICLUtils/Configurable.h>


namespace icl{

  /// Fiducial detector class that allows for using multiple cameras at once for fiducial detection
  /** By using more than one camera at once, the 3D Pose estimation accurrace can 
      be increased significantly. The MultiCamFiducialDetector detects Fiducials in several 
      input images and uses corresponding camera calibration files to provide highly accurate
      3D marker poses.
      If a fiducial is only detectable in a single camera image, single-camera pose-estimation
      is used automatically.
      
      \section __HOW__ How it works
      Internall, for each camera view, a dedicated FiducialDetector is instantiated. If a new
      set of input images is given, all of these FiducialDetectors are applied to the corresponding
      image. Then, the 2D fiducial detection results are sorted by marker ID and combined.
      A MultiCamFiducial with ID x combines all fiducials with ID x that were detection in all views
      
      \section __RES__ Restriction
      Due to the fact, that the markers are combined by ID, It is not allowed to have
      several markers with Identical IDs in a scene. If you have, they will be mixed up and the
      resulting 3D pose estimation becomes completly wrong.
      
      @see MultiCamFiducial
      @see FiducialDetector
  */
  class MultiCamFiducialDetector : public Configurable{
    struct Data;  //!< internal data structure
    Data *m_data; //!< internal data pointer
    
    /// internally used property callback
    void property_callback(const Property &p);
    
    public:
    
    /// creates an uninitialized instance
    MultiCamFiducialDetector();
    
    /// creates an initialized instance with given parameters
    /** @see MultiCamFiducialDetector::init */
    MultiCamFiducialDetector(const std::string &pluginType,
                             const Any &markersToLoad,
                             const ParamList &params,
                             const std::vector<Camera*> &cams,
                             bool syncProperties=true,
                             bool deepCopyCams=false) throw (ICLException);

    /// (re-)initializes the detector with given parameters
    /** @param pluginType this option directly passed to all internal 2D FiducialDetector instances
        @param markersToLoad this option directly passed to all internal 2D FiducialDetector instances
        @param params this option directly passed to all internal 2D FiducialDetector instances
        @param cams Contains the list of calibrated cameras. Please note that cams[i] must belong
               to images[i] in the MultiCamFiducialDetector::detect method
        @param syncProperties if this is set to false, each internal 2D FiducialDetector get's its own
               property list. If syncProperties is true (default), the 2D FiducialDetector properties
               are synchronized internally
        @param deepCopyCams by default, the given cameras are just copied shallowly (by pointer)
               if the cameras are moved at runtime, and the MultiCamFiducialDetector shall use the
               adapted camera parameters then, deepCopyCams must be left false (default)
               <b>Please Note:</b> you can obtain all cameras of a scene using Scene::getAllCameras()
    */
    void init(const std::string &pluginType,
              const Any &markersToLoad,
              const ParamList &params,
              const std::vector<Camera*> &cams,
              bool syncProperties=true,
              bool deepCopyCams=false) throw (ICLException);
    
    
    /// detects fiducials in all images and returns the combined results
    /** Please note, that images[i] must correspond to cams[i] in the constructor/init method*/
    const std::vector<MultiCamFiducial> &detect(const std::vector<ImgBase*> &images, 
                                                int minCamsFound=1) throw (ICLException);
    
    /// returns the internal number of cameras
    int getNumCameras() const;
    
    /// returns the internal 2D FiducialDetector at given camera index
    const FiducialDetector &getFiducialDetector(int idx) const;

    /// returns the internal 2D FiducialDetector at given camera index (const)
    FiducialDetector &getFiducialDetector(int idx);
    
    /// loads additional markers (passed to all 2D detectors)
    void loadMarkers(const Any &which, const ParamList &params) throw (ICLException);

    /// unloads markers (passed to all 2D detectors)
    void unloadMarkers(const Any &which);
    
    /// provides a comma separated list of intermediate images
    /** The returned list can simply be added to a view-selection combo box.
        e.g.
        <pre>
        MultiCamFiducialDetector fd;
        fd.init(...);        
        gui << "combo(" + fd.getIntermediateImageNames() + ")[@label=current image@handle=vis]"
            << draw3D()[@handle=image]
            << "!show";
        
        ...
        
        
        gui["image"] = fd.getIntermediateImage( gui["vis"] );
        gui["image"].update();
        </pre>
        */
    std::string getIntermediateImageNames() const;
    
    /// returns a named intermeted image
    const ImgBase *getIntermediateImage(const std::string &name) const throw (ICLException);
    
    /// extract the current camera from the given intermediate image name
    static int getCameraIDFromIntermediteImageName(const std::string &name);
  };  

}
#endif
