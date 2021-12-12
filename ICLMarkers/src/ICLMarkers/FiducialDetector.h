/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/FiducialDetector.h           **
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
#include <ICLUtils/Configurable.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Any.h>
#include <ICLUtils/ParamList.h>

#include <ICLGeom/Camera.h>

#include <ICLMarkers/Fiducial.h>



namespace icl{
  namespace markers{
    /// Main Fiducial Detector class
    class ICLMarkers_API FiducialDetector : public utils::Uncopyable, public utils::Configurable{

      /// hidden data class
      struct Data;

      /// hidden data pointer
      Data *data;

      public:

      /// create a FiducialDetector instance with speical plugin type
      /** The given plugin type can be:

          - "art"
            for ARToolkit markers (visit the
            <a href="http://www.hitl.washington.edu/artoolkit/">ARToolkit Webpage</a>
            for more details)
            ART Fiducials can provide the full set of information including the 3D pose
            from a single marker
          - "bch"
            for ARToolkit+/Studierstube-Tracker like bch-code based binary markers
            (visit the <a href="http://studierstube.icg.tugraz.at/handheld_ar/stbtracker.php">
            Studierstube Tracker Homepage</a> for more details)
            BCH Fiducials can provide the full set of information including the 3D pose
            from a single marker. Usually, the detection rate is faster and more accurate
            in comparison to artoolkit markers.
          - "amoeba"
            for The amoeba style hierarchical markers provided by the
            <a href="http://reactivision.sourceforge.net/">reactivision software</a>
            Amoeba fiducials cannot be detected in 3D. They do only provide 2D center and
            rotation and the 2D boundary.
          - "icl1"
            for ICL's own markers, that were also used for the former version of
            deformable paper tracking. Here, 32 hierarchical markers are provided. These markers
            do also provide key-point association and 3D pose estimation. all allowed marker IDs
            can be used twice: positive IDs indicate 'normal' markers (with a black root region)
            while negative marker IDs indicate 'inverted' markers (with white root regions)

          <b>other parameters</b>\n
          For extraction of 3D marker information, usually a camera
          is needed. The camera can also set later using setCamera
          Even though the camera is passed per pointer, it is
          compied deeply into the FiducialDetector instance. The
          camera can be updated using setCamera() lateron\n
          The also optionally given markersToLoad and params
          are directly passed to the loadMarkers method

          <b>benchmarks</b>
          TODO
      */
      FiducialDetector(const std::string &plugin="bch",
                       const utils::Any &markersToLoad=utils::Any(),
                       const utils::ParamList &params=utils::ParamList(),
                       const geom::Camera *camera=0);

      /// Destructor
      virtual ~FiducialDetector();

      /// returns the current plugin type
      const std::string &getPluginType() const;

      /// sets the current camera
      /** The camera is usually needed for extraction of 3D marker
          information. After setting a camera, some already existent
          Fiducials might become out of date. Therefore, detect
          must be called again when the camera was changed. */
      void setCamera(const geom::Camera &camera);

      /// returns the current camera (or throws an exception if no camera is available)
      const geom::Camera &getCamera() const;

      /// loads markers according to the current plugin type
      /** - "bch":\n
            params is a string that is either a range "[a,b]" of markers
            that are used or a list "{a,b,c,...}" of allowed marker IDs or a single marker ID.
            The used bch codes are able to encode marker IDs in range [0,4095].
            Please note that angular braces define a range, while curly
            braces define a list here.\n
            Artoolkit markers need extra param in the param list which is the real marker size (in mm)
            An example param list is <tt>ParamList("size",utils::Size(50,50));</tt>
          - "art":\n
            here, an image file-name or file-pattern is required. This image is automatically
            converted into an appropriate matching representation
          - "amoeba"\n
            Each amoeba marker is represented by a special two-level hierarchical
            code representation that is wrapped by the TwoLevelRegionStructure class.
            You can pass either a file, that contains a list of these codes (one code per line)
            or a single code or a comma- or space separated list of codes. Please note
            that the "amoeba" markers are only free to use in combination with the
            <a href="http://reactivision.sourceforge.net">reacTIVision</a> software.
          - "icl1"\n
            These are the 32 allowed icl1 marker IDs: 1294  1252  1287  1035  1245  993  1292
            1250  1280  1028  1208  776  1238  986  1166  734  1285  1033  1243  991  1273
            1021  1201  769  949  517  1231  979  1159  727  907  475. Other ID's will not be detected,
            but can be added in a more genereal list. E.g. by adding which ="[450-1300]", you can
            add all markers. Actually if you load a marker ID x, also the inverted marker
            (that has a white root-region) with ID -x is added.
      */
      void loadMarkers(const utils::Any &which, const utils::ParamList &params=utils::ParamList());


      /// unloads all already defined markers
      /** usually, markers are unloaded with the same pattern that was
          also used for loading the markers before */
      void unloadMarkers(const utils::Any &which);

      /// detects markers in the given and returns all found markers
      /** Please note, the preferred input core::format is core::Img8u. Other formats
          are converted to depth8u internally. Usually, your
          plugin will use gray images as input. You can query this information
          by calling getPropertyValue("preferred image type") */
      const std::vector<Fiducial> &detect(const core::ImgBase *image);

      /// returns the list of supported features
      Fiducial::FeatureSet getFeatures() const;

      /// returns a list (comma separated) of all available intermediate image results
      /** The returned images can be used for debugging or for further processing.
          the Fiducial Detector does always add the two tokes "input" and "pp", which
          refers to the last input image and the last preprocessing result image
      */
      std::string getIntermediateImageNames() const;

      /// returns the intermediate result image associated with given name
      /** If no image is associated with this name, an exception is thrown. Note,
          the image might be associated, but still be null, in particular if detect
          was not called one before */
      const core::ImgBase *getIntermediateImage(const std::string &name) const;


      /// creates an image of a given markers
      /** sice this function call is directly passed to the underlying FiducialDetectorPlugin
          instance, its implemention depends on this implementation.
          @param whichOne specifies the marker to create in case of plugin type 'icl1' or 'bch',
                          this is simply the marker ID. If the plugin type is art, whichOne needs to
                          be an image filename. Amoeba markers cannot be created automatically
                          because the marker layout is not free.
          @param size resulting image size (please note, that the recommended image size aspect ratio
                      for icl1-typed markers is 13:17 (e.g. 230:300)
          @param params this list contains additional parameters that are neccessary for marker
                        creation.

          \section PARAMS params for different plugin types
          * <b>bch</b> needs an integer valued "border-width" parameter, which is used as border width.
            Please note that the bch-code center always consists of 6x6 pixels. The resulting image of size
            2*border-width + 6 is finally scaled up to 'size' before it is returned.
          * <b>art</b> needs a float valued 'border ratio' parameter. This is used to compute the border size
            for a given marker image. If the marker image (without border) has size A^2, then
            whole marker image (whith borders) has size (1+'border ratio')*A. Only finally, the image is scaled
            to it's destination size.
          * <b>icl1</b> does not need extra parameters
          * <b>amoeba</b> is not supported
      */
      core::Img8u createMarker(const utils::Any &whichOne,const utils::Size &size, const utils::ParamList &params);

      /// returns the internal plugin
      FiducialDetectorPlugin* getPlugin();
    };

  } // namespace markers
}
