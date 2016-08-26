/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/apps/camera-calibration/CameraCalibration   **
**          Utils.h                                                **
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

#include <ICLGeom/GeomDefs.h>
#include <string>
#include <ICLMarkers/FiducialDetector.h>
#include <ICLQt/DrawHandle3D.h>
#include <ICLGeom/SceneObject.h>
#include <ICLUtils/Lockable.h>
#include <ICLCore/Line32f.h>
#include <QtCore/QObject>
#include <ICLQt/GUI.h>

namespace icl{
  namespace markers{

    /// special utility class that provices outsource functions and types for the camera-calibration app
    struct CameraCalibrationUtils{

      /// A PossibleMarker is a marker prototype that <em>could</em> be detected
      struct PossibleMarker{
        PossibleMarker():loaded(false){}
      PossibleMarker(int cfgFileIndex,const geom::Vec &v):loaded(true),center(v),hasCorners(false),cfgFileIndex(cfgFileIndex),gridIdx(-1){}
      PossibleMarker(int cfgFileIndex,const geom::Vec &v, const geom::Vec &a, const geom::Vec &b, const geom::Vec &c, const geom::Vec &d):
        loaded(true),center(v),hasCorners(true),cfgFileIndex(cfgFileIndex),gridIdx(-1){
            corners[0] = a;
            corners[1] = b;   
            corners[2] = c;
            corners[3] = d;
        }
        bool loaded;
        geom::Vec center;
        bool hasCorners;
        geom::Vec corners[4];
        int cfgFileIndex;
        int gridIdx;
      };

      /// special utility class that implements saving the best of a number of calibration results
      struct BestOfNSaver : public QObject, public utils::Lockable{
        std::vector<geom::Camera> cams;
        std::vector<float> errors;
        int n;
        int num_end;
        bool inited;
        std::string filename;
        float lastBestError;
        std::string lastFileName;
        float runningBestError;
        utils::Function<int> nFramesSource;
        
        BestOfNSaver(utils::Function<int> nFramesSource);
        virtual bool event(QEvent *event);
        void init();
        void stop();
        
        std::pair<int,float> next_hook(const geom::Camera &cam, float error);
      };

      /// supported marker types (actually only BCH really makes sense)
      enum MarkerType {
        BCH,
        AMOEBA
      };

      /// A named transform can be provided in the calibration object definition file
      /** Named transforms are used to rotate and translate the calibration object origin frame */
      struct NamedTransform{
        NamedTransform(){}
        NamedTransform(const std::string &name,const geom::Mat &t):
        name(name),transform(t){}
        std::string name;
        geom::Mat transform;
      };

      /// MarkerGrids are used to more simply define a 2D-aligned grid of markers attached to a calibration object
      struct MarkerGrid{
        math::Vec3 offset;
        math::Vec3 dx;
        math::Vec3 dy;
        utils::Size dim;
        utils::Size32f markerSize;
        std::vector<int> markerIDs;
        MarkerType type;
        int getDim() const { return dim.width * dim.height; }
        utils::Point getCell(int id) const;
        utils::Point getCellFromCellIdx(int cellIdx) const;
        int getCellIdx(int id) const;
      };

      /// represents the parsed content of an XML-calibration object description file
      struct CalibFile{
        std::string filename;
        std::vector<NamedTransform> transforms;
        geom::SceneObject* obj;
        std::vector<MarkerGrid> grids;
      };

      /// represents a marker that was actaully found in the calibration process
      struct FoundMarker{
        FoundMarker(){}
      FoundMarker(int markerID, const PossibleMarker *possible, MarkerType t, markers::Fiducial fid, const utils::Point32f &imagePos,
                  const geom::Vec &worldPos, int cfgFileIndex):
        id(markerID),possible(possible),type(t),fid(fid),imagePos(imagePos),worldPos(worldPos),hasCorners(false),
          cfgFileIndex(cfgFileIndex){}
      FoundMarker(int markerID, const PossibleMarker *possible, MarkerType t,
                  markers::Fiducial fid, const utils::Point32f &imagePos, const geom::Vec &worldPos,
                  const utils::Point32f imageCornerPositions[4],
                  const geom::Vec worldCornerPositions[4],
                  int cfgFileIndex):
        id(markerID), possible(possible), type(t), fid(fid),imagePos(imagePos),worldPos(worldPos),hasCorners(true),cfgFileIndex(cfgFileIndex){
          std::copy(imageCornerPositions,imageCornerPositions+4,this->imageCornerPositions);
          std::copy(worldCornerPositions,worldCornerPositions+4,this->worldCornerPositions);
        }
        int id;
        const PossibleMarker *possible;
        MarkerType type;
        markers::Fiducial fid;
        utils::Point32f imagePos;
        geom::Vec worldPos;
        bool hasCorners;
        utils::Point32f imageCornerPositions[4];  
        geom::Vec worldCornerPositions[4];
        int cfgFileIndex;
      };

      /// accumulates a combination of data needed for the calibration application
      struct CalibFileData{
        qt::GUI objGUI;
        utils::SmartPtr<markers::FiducialDetector> fds[2];
        std::vector<PossibleMarker> possible[2];
        std::vector<std::string> configurables;
        std::string iin; // comma-sep. string list
        markers::FiducialDetector *lastFD; // used for visualization
        geom::SceneObject *planeObj;
        std::vector<CalibFile> loadedFiles;

      CalibFileData():lastFD(0),planeObj(0){
          possible[0] = possible[1] = std::vector<PossibleMarker>(4096);
        }
      };

      /// describes a MarkerGrid for which at least a single marker was found
      struct DetectedGrid{
        DetectedGrid();
        operator bool() const;
        const MarkerGrid *realGrid;
        std::vector<const FoundMarker*> foundMarkers;
        void setup(const MarkerGrid *realGrid);
        int numFound() const;

        geom::Mat estimatePose(const geom::Camera &cam) const;
        
        void getGridCornersAndTexture(const geom::Camera &cam,
                                      std::vector<utils::Point> &points,
                                      std::vector<core::Line32f> &lines,
                                      utils::Size32f &size,
                                      const utils::Rect &bounds) const;
      };
      

      /// accumulates all data that is provided by a single calibration call
      /** Some data, such as camera calibration parameters are
          returned though the given arguments */
      struct CalibrationResult{
        float error;
        std::string status;
        int saveRemainingFrames;
        float saveBestError;
      };

      // this was move to the camera class
      //      static geom::Camera optimize_extrinsic_lma(const geom::Camera &init, const std::vector<geom::Vec> &Xws, 
      //                                           const std::vector<utils::Point32f> &xis);

      /// actually performs the camera calibration
      static CalibrationResult perform_calibration(const std::vector<FoundMarker> &markers,
                                                   const std::vector<bool> &enabledCfgFiles,
                                                   const std::vector<geom::Mat> &Ts,
                                                   const geom::Mat &Trel, const utils::Size &imageSize,
                                                   bool &deactivatedCenters, bool useCorners,
                                                   bool normalizeError, BestOfNSaver *saver,
                                                   bool &haveAnyCalibration, geom::Scene &scene,
                                                   const geom::Camera *givenIntrinsicParams=0,
                                                   bool performLMAbasedOptimiziation=false);

      /// parses a camera calibration file
      static CalibFile parse_calib_file(const std::string &filename, int calibrationFileIndex, CalibFileData &data);

      /// adapts the camera calibration help-indicator plane
      static void change_plane(const std::string &handle, qt::GUI &planeOptionGUI, geom::Scene &scene,
                               CalibFileData &calibFileData);

      /// creates a simple template for a calibration object description file
      static const std::string &create_sample_calibration_file_content();


      /// creates a new FiducialDetector (used for the camera calibration app)
      static FiducialDetector *create_new_fd(MarkerType t,
                                             std::vector<std::string> &configurables,
                                             std::string &iin,
                                             const utils::Size *imageSize=0);

      /// returns the output-filename (either from a given program-argument or by spawning a dialog)
      static std::string get_save_filename(const std::string &progArgName);


      /// saves the calibration result
      static void save_cam_filename(geom::Camera cam, 
                                    const std::string &outputSizeProgArg,
                                    const std::string &filename);

      /// saves the calibration result (using program argument for the definition of the output filename)
      static void save_cam_pa(const geom::Camera &cam, 
                           const std::string &outputSizeProgArg,
                           const std::string &outputFileNameProgArg);

      /// utility method that visualizes markers that were found in the calibration loop
      static void visualize_found_markers(qt::DrawHandle3D &draw,
                                          const std::vector<FoundMarker> &markers,
                                          const std::vector<bool> &enabled,
                                          bool deactivatedCenters, bool useCorners);

      /// detects markers in the source image
      static std::vector<FoundMarker> detect_markers(const core::ImgBase *image,
                                                     CalibFileData &calibFileData);

      /// visualizes the help-indicator plane (if required)
      static void visualize_plane(qt::DrawHandle3D &draw,
                                  const std::string &planeDim,
                                  float planeOffset,
                                  const utils::Point32f &currentMousePos,
                                  geom::Scene &scene);

      /// performs the image preprocessing (based on program args)
      static const core::ImgBase *preprocess(const core::ImgBase *image);
    };
  }
}
