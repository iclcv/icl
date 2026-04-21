// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/markers/MultiCamFiducial.h>
#include <icl/utils/Configurable.h>


namespace icl::markers {
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
  class ICLMarkers_API MultiCamFiducialDetector : public utils::Configurable{
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
                             const utils::Any &markersToLoad,
                             const utils::ParamList &params,
                             const std::vector<geom::Camera*> &cams,
                             bool syncProperties=true,
                             bool deepCopyCams=false);

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
              const utils::Any &markersToLoad,
              const utils::ParamList &params,
              const std::vector<geom::Camera*> &cams,
              bool syncProperties=true,
              bool deepCopyCams=false);


    /// detects fiducials in all images and returns the combined results
    /** Please note, that images[i] must correspond to cams[i] in the constructor/init method*/
    const std::vector<MultiCamFiducial> &detect(const std::vector<const core::ImgBase*> &images,
                                                int minCamsFound=1);

    /// returns the internal number of cameras
    int getNumCameras() const;

    /// returns the internal 2D FiducialDetector at given camera index
    const FiducialDetector &getFiducialDetector(int idx) const;

    /// returns the internal 2D FiducialDetector at given camera index (const)
    FiducialDetector &getFiducialDetector(int idx);

    /// loads additional markers (passed to all 2D detectors)
    void loadMarkers(const utils::Any &which, const utils::ParamList &params);

    /// unloads markers (passed to all 2D detectors)
    void unloadMarkers(const utils::Any &which);

    /// provides a comma separated list of intermediate images
    /** The returned list can simply be added to a view-selection combo box.
        e.g.
        <pre>
        icl::markers::MultiCamFiducialDetector fd;
        fd.init(...);
        gui << Combo(fd.getIntermediateImageNames()).label("current image").handle("vis")
            << Canvas3D().handle("image")
            << Show();

        ...

        gui["image"] = fd.getIntermediateImage( gui["vis"] );
        gui["image"].render();
        </pre>
        */
    std::string getIntermediateImageNames() const;

    /// returns a named intermeted image
    const core::ImgBase *getIntermediateImage(const std::string &name) const;

    /// extract the current camera from the given intermediate image name
    static int getCameraIDFromIntermediteImageName(const std::string &name);
  };

  } // namespace icl::markers