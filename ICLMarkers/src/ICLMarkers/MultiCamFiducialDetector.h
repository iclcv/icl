/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/MultiCamFiducialDetector.h   **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLMarkers/MultiCamFiducial.h>
#include <ICLUtils/Configurable.h>


namespace icl{
  namespace markers{

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
                               bool deepCopyCams=false) throw (utils::ICLException);

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
                bool deepCopyCams=false) throw (utils::ICLException);


      /// detects fiducials in all images and returns the combined results
      /** Please note, that images[i] must correspond to cams[i] in the constructor/init method*/
      const std::vector<MultiCamFiducial> &detect(const std::vector<const core::ImgBase*> &images,
                                                  int minCamsFound=1) throw (utils::ICLException);

      /// returns the internal number of cameras
      int getNumCameras() const;

      /// returns the internal 2D FiducialDetector at given camera index
      const FiducialDetector &getFiducialDetector(int idx) const;

      /// returns the internal 2D FiducialDetector at given camera index (const)
      FiducialDetector &getFiducialDetector(int idx);

      /// loads additional markers (passed to all 2D detectors)
      void loadMarkers(const utils::Any &which, const utils::ParamList &params) throw (utils::ICLException);

      /// unloads markers (passed to all 2D detectors)
      void unloadMarkers(const utils::Any &which);

      /// provides a comma separated list of intermediate images
      /** The returned list can simply be added to a view-selection combo box.
          e.g.
          <pre>
          icl::markers::MultiCamFiducialDetector fd;
          fd.init(...);
          gui << Combo(fd.getIntermediateImageNames()).label("current image").handle("vis")
              << Draw3D().handle("image")
              << Show();

          ...

          gui["image"] = fd.getIntermediateImage( gui["vis"] );
          gui["image"].render();
          </pre>
          */
      std::string getIntermediateImageNames() const;

      /// returns a named intermeted image
      const core::ImgBase *getIntermediateImage(const std::string &name) const throw (utils::ICLException);

      /// extract the current camera from the given intermediate image name
      static int getCameraIDFromIntermediteImageName(const std::string &name);
    };

  } // namespace markers
}
