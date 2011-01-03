/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/CalibrationGrid.h                      **
** Module : ICLGeom                                                **
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

#ifndef ICL_CALIBRATION_GRID_H
#define ICL_CALIBRATION_GRID_H

#include <ICLUtils/Rect.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Camera.h>
#include <ICLQt/DrawWidget.h>
#include <ICLCore/ImgBase.h>

namespace icl{
  
  
  /// Utility class that is used in combination with the CalibrationObject class
  class CalibrationGrid {

    /// utility method for the default implementation
    inline int a(int x, int y){ return 4+x+nx*y; }

    /// utility method for the default implementation
    inline int b(int x, int y){ return 4+x+nx*y + nx*ny; }

    public:
    /// internal typedef for 3D vectors
    typedef FixedColVector<float,3> Vec3;

    /// The class represents one of two parts of which the default CalibrationGrid consists
    struct Half{
      std::vector<Point32f> img;  //!< row-major order sorted image points
      std::vector<Vec3> world;    //!< row-major order sorted world points
      Point32f p1,p2,p3;          //!< ???
    };
    protected:
    Half A; //!< left part of the default calibration grid
    Half B; //!< right part of the default calibration grid
    
    int nx; //!< number of nodes in x-direction of each grid half
    int ny; //!< number of nodes in x-direction of each grid half
    bool inputDataReady; //!< flag, that is used in the default implementation
    std::vector<Rect> lastBoundingBoxes; //!< these are just used for visualization in the default implementation
    
    public:
    /// Empty constructor creating an empty useless CalibrationGrid
    CalibrationGrid();
    
    /// common constructor loads the CalibrationGrid from given filename
    /** Optionally, a given SceneObject can be initialized with the calibration objects data */
    CalibrationGrid(const std::string &configFileName);
    
    /// loads the default CalibrationGrid from an xml-configuration file
    void loadFromFile(const std::string &configFileName);
    
    /// initializes a given SceneObject
    /** This method adds vertices and lines to the given SceneObject instance
        to represent this CalibrationGrid in an icl::Scene */
    void initializeSceneObject(SceneObject &so);

    /// 2D visualization method
    /** The default implementation uses Half A and B's img-points */
    virtual void visualize2D(ICLDrawWidget &w);

    /// adaptable method to find two marked points 
    /** The two marked points (equipped with an extra inner white marker) are
        detected by vision per default. This method can be adpated if there are
        other -- maybe simpler or more robust -- ways to find these two points 
        @param cogs input centers of gravity
        @param bbs input bounding boxes (these are used in the default implementation)
        @param maskedImage (this is the image that is returned by the parent
                           CalibrationObject::findPoints-method. If an adapted
                           CalibrationObject implementation does not return a
                           thresholded image, the default implementation for
                           findMarkedPoints will fail and has to be adapted.
    */
    virtual std::pair<int,int> findMarkedPoints(const std::vector<Point32f> &cogs, 
                                                const std::vector<Rect> &bbs, 
                                                const Img8u *hintImage);

    /// updates the calibration objects Half's A and B from given centers of gravity
    /** maskedImage is only passed to findMarkedPoints, so it can be null, if it is not used
        by an adapted implementation of findMarkedPoints */
    virtual void update(const std::vector<Point32f> &cogs, const std::vector<Rect> &bbs, const Img8u *hintImage);
    
    /// finally applies calibration return error and initializing given Camera
    virtual float applyCalib(const Size &imageSize, Camera &cam);

    /// computes the root mean squared error from given world-points, image points and calibrated Camera
    static float get_RMSE_on_image(const std::vector<Vec> &ws, const std::vector<Point32f> &is, const Camera &cam);
    
    /// creates an empty configuration file
    static void create_empty_configuration_file(const std::string &filename="/dev/stdout");

    /// return the dimension of one Half (e.g. 4x3 blobs)
    Size getDimension() const;
  };

}

#endif
