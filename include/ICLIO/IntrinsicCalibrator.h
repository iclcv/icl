/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLIO/IntrinsicCalibrator.h                    **
 ** Module : ICLIO                                                  **
 ** Authors: Christian Groszewski                                   **
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

#include <ICLCore/Img.h>
#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/DynMatrix.h>
#include <ICLUtils/DynMatrixUtils.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLUtils/StackTimer.h>
#include <ICLUtils/Array2D.h>
#include <ICLIO/ImageUndistortion.h>

namespace icl{

  /**
      This class implements the image undistortion procedure.
      It is a reimplementation from the matlab toolbox
      Pass in data where world and image points corresponds to each other in layout
      or use the optimze call where world points are generated automatically and
      layout of image points must be rowwise.
      */
  class IntrinsicCalibrator : public Uncopyable{

    public:
    ///Simple struct for handle result of computation of intrinsics
    struct Result : public ImageUndistortion{
      
      /// create a null result
      Result(){}
      
      /// create a result with given parameters
      Result(const std::vector<double> &params, const Size &size):
        ImageUndistortion("MatlabModel5Params",params,size){}

      ///returns focal length for x direction
      /**
          * return x component of focal length
          */
      double getFocalLengthX() const { return getParams()[0]; }
      ///returns focal length for y direction
      /**
          * return y component of focal length
          */
      double getFocalLengthY() const { return getParams()[1]; }
      ///returns new computed principal point x component
      /**
          * return x component of principal point
          */
      double getPrincipalX() const { return getParams()[2]; }
      ///returns new computed principal point y component
      /**
          * return y component of principal point
          */
      double getPrincipalY() const { return getParams()[3]; }
      ///returns the skew
      /**
          * @return skew
          */
      double getSkew() const { return getParams()[4]; }

      ///returns first radial distortion param
      /**
          * @return first radial distortion param
          */
      double getK1() const { return getParams()[5]; }
      ///returns second radial distortion param
      /**
          * @return second radial distortion param
          */
      double getK2() const { return getParams()[6]; }
      ///returns third radial distortion param
      /**
          * @return third radial distortion param
          */
      double getK3() const { return getParams()[9]; }
      ///returns first tangential distortion param
      /**
          * @return first tangential distortion param
          */
      double getP1() const { return getParams()[7]; }
      ///returns second tangential distortion param
      /**
          * @return second tangential distortion param
          */
      double getP2() const { return getParams()[8]; }
    };
    private:
    ///internal datastorage
    class Data;
    Data *m_data;
    ///struct for handling result of computation
    Result m_calres;

    ///Computes the rigid motion transformation Y = R*X+T
    /**
        *@param X  3D structure in the world coordinates
        *@param om  rotation vector
        *@param T  translation vector
        *@param Y 3D coordinates of the structure points in the camera frame
        *@param dYdom Derivative of Y with respect to om
        *@param dYdT Derivative of Y with respect to T
        */
    void rigid_motion(const DynMatrix<icl64f> &X, const DynMatrix<icl64f> &om, const DynMatrix<icl64f> &T,
                      DynMatrix<icl64f> &Y, DynMatrix<icl64f> &dYdom, DynMatrix<icl64f> &dYdT);

    ///Projects a 3D structure onto the image plane.
    /**
        * @param X 3D world coordinates
        * @param om rotation vector
        * @param T translation vector
        * @param f focal length
        * @param c principal point
        * @param k distortion coefficients
        * @param alpha skew
        * @param xp Projected pixel coordinates (2xN matrix for N points)
        * @param dxpdom Derivative of xp with respect to om
        * @param dxpdT Derivative of xp with respect to T
        * @param dxpdf Derivative of xp with respect to f
        * @param dxpdc Derivative of xp with respect to c
        * @param dxpdk Derivative of xp with respect to k
        */
    void project_points2(const DynMatrix<icl64f> &X, const DynMatrix<icl64f> &om, const DynMatrix<icl64f> &T,
                         const DynMatrix<icl64f> &f, const DynMatrix<icl64f> &c, const DynMatrix<icl64f> &k, const double alpha,
                         DynMatrix<icl64f> &xp, DynMatrix<icl64f> &dxpdom, DynMatrix<icl64f> &dxpdT, DynMatrix<icl64f> &dxpdf,
                         DynMatrix<icl64f> &dxpdc, DynMatrix<icl64f> &dxpdk, DynMatrix<icl64f> &dxpdalpha);

    ///Compensates for radial and tangential distortion (Model From Oulu university)
    /**
        * @param xd distorted, normalized image coordinates
        * @param k distortion coefficients
        * @param x undistorted, normalized image coordinates
        */
    void comp_distortion_oulu(const DynMatrix<icl64f> xd,const DynMatrix<icl64f> k, DynMatrix<icl64f> &x);

    ///Computes the planar homography between the point coordinates on the plane (M) and the image point coordinates (m).
    /**
        * @param m homogeneous image coordinates
        * @param M homogeneous world coordinates
        * @param H Homography matrix
        */
    void compute_homography(const DynMatrix<icl64f> &m, const DynMatrix<icl64f> &M, DynMatrix<icl64f> &H);

    ///Levenberg-Marquardt sparse algorithm optimizes all parameters
    /**
        * @param impoints image coordinates
        * @param worldpoints world coordinates
        * @param params parameters to be optimized
        */
    void optimize(const DynMatrix<icl64f> &impoints, const DynMatrix<icl64f> &worldPoints, double *params);

    ///Initialization of the intrinsic parameters.
    /**
        * @param x images coordinate
        * @param X world coordinates
        * @param fc focal length
        * @param cc principal point coordinates
        * @param kc distortion coefficients
        * @param alpha_c: skew coefficient
        */
    void init_intrinsic_param(const DynMatrix<icl64f> &x, const DynMatrix<icl64f> &X,
                              DynMatrix<icl64f> &fc, DynMatrix<icl64f> &cc, DynMatrix<icl64f> &kc, double &alpha_c);

    /// Computes the extrinsic parameters
    /**
        * @param x_kk image coordinates
        * @param X_kk world coordinates
        * @param fc focal length
        * @param cc principal point
        * @param kc distortion coefficients
        * @param alpha_c the skew
        * @param thresh_cond number of iterations for optimization
        * @param omcck rotation vector
        * @param Tckk translation vector
        * @param Rckk rotation matrix
        */
    void comp_ext_calib(const DynMatrix<icl64f> &x_kk, const DynMatrix<icl64f> &X_kk, const DynMatrix<icl64f> &fc,
			const DynMatrix<icl64f> &cc, const DynMatrix<icl64f> &kc, const double alpha_c, const double thresh_cond,
			DynMatrix<icl64f> &omckk, DynMatrix<icl64f> &Tckk, DynMatrix<icl64f> &Rckk);

    ///Computes the normalized coordinates xn given the pixel coordinates x_kk and the intrinsic camera parameters fc, cc and kc.
    /**
        * @param x_kk image coordinates
        * @param fc focal length
        * @param cc principal point
        * @param kc distortion coefficients
        * @param alpha_c skew
        * @param xn normalized image coordinates
        */
    void normalize_pixel(const DynMatrix<icl64f> x_kk, const DynMatrix<icl64f> fc, const DynMatrix<icl64f> cc,
                         const DynMatrix<icl64f> kc,const double alpha_c, DynMatrix<icl64f> &xn);

    ///Computes the mean of each col of x_k
    /**
        * If x_k is is rowvector the mean of the rowvector is computed.
        * @param x_k Inputmatrix
        * @param res resultmatrix
        */
    void mean(const DynMatrix<icl64f> &x_k, DynMatrix<icl64f> &res);

    ///Computes the rodrigues transformation and derivative
    /*
        *@param in rotation matrix or vector
        *@param out rotation matrix or vector
        *@param dout derivative of rotation
        */

    void rodrigues(const DynMatrix<icl64f> &in,DynMatrix<icl64f> &out, DynMatrix<icl64f> &dout);

    ///Computes extrinsic initial parameters
    /**
        * @param x_kk image coordinates
        * @param X_kk world coordinates
        * @param fc focal length
        * @param cc  principal point
        * @param kc distortion coefficients
        * @param alpha_c skew
        * @param omckk rotation vector
        * @param Tckk: translation vector
        * @param Rckk: rotation matrix
        */
    void compute_extrinsic_init(const DynMatrix<icl64f> &x_kk, const DynMatrix<icl64f> &X_kk, const DynMatrix<icl64f> &fc,
                                const DynMatrix<icl64f> &cc, const DynMatrix<icl64f> &kc, const double &alpha_c,
                                DynMatrix<icl64f> &omckk, DynMatrix<icl64f> &Tckk, DynMatrix<icl64f> &Rckk);

    ///optimizes precomputed extrinsic parameters
    /*
        * @param x_kk image coordinates
        * @param X_kk world coordinates
        * @param fc focal length
        * @param cc principal point
        * @param kc distortion coefficients
        * @param alpha_c skew
        * @param MaxIter maximum number of iterations for optimazation
        * @param omckk rotation vector
        * @param Tckk translation vector
        * @param Rckk rotation matricx
        */
    void compute_extrinsic_refine(const DynMatrix<icl64f> &omc_init, const DynMatrix<icl64f> &Tc_init,
                                  const DynMatrix<icl64f> &x_kk, const DynMatrix<icl64f> &X_kk, const DynMatrix<icl64f> &fc,const DynMatrix<icl64f> &cc,
                                  const DynMatrix<icl64f> &kc,const double alpha_c, const int MaxIter, double thresh_cond,
                                  DynMatrix<icl64f> &omckk, DynMatrix<icl64f> &Tckk, DynMatrix<icl64f> &Rckk, DynMatrix<icl64f> &JJ);

    public:

    ///Constructs a new object of this class for computation of intrinc parameters
    /**
        * @param boardWidth the width of the chessboard
        * @param boardHeight the height of the chessboard
        * @param boardCount the number of board for computations
        * @param imageWidth the imagewidth
        * @param imageHeight the imageheight
        */
    IntrinsicCalibrator(unsigned int boardWidth=6, unsigned int boardHeight=9, unsigned int boardCount=8,
			unsigned int imageWidth=640, unsigned int imageHeight=480);

    ///Destructor
    ~IntrinsicCalibrator();

    ///calibrates the camera
    /**Since for each image the worldcoordinates are/look exactly the same,
        * just pass the worldcoordinates for the first image. This will be used for all computations.
        * @param all image coordinates
        * @param world coordinates for the first image, sorting depends on image coordinates
        */
    Result calibrate(const DynMatrix<icl64f> &impoints, const DynMatrix<icl64f> &worldpoints);

    ///saves computed/current intrinsics to xml-file
    /**
        * @param filename name of xml-file for saving
        * @param size picturesize
        */
    void saveIntrinsics(const std::string &filename);

    ///loads intrincs from xml-file
    /**
        * @param filename name of xml-file to load
        */
    void loadIntrinsics(const std::string &filename);

    ///resets data to compute new intrinsics with
    /**
        * @param boardWidth the new width of the chessboard
        * @param boardHeight the new height of the chessboard
        * @param boardCount the new number of board for computations
        * @param imageWidth the new imagewidth
        * @param imageHeight the new imageheight
        */
    void resetData(unsigned int boardWidth, unsigned int boardHeight,
                   unsigned int boardCount,unsigned int imageWidth ,unsigned int imageHeight);

    ///return the result of computation
    Result getResult() const {
      return m_calres;
    }

    //2d coordinates in picture
    typedef FixedMatrix<double,1,2> Pos2D;
    //grid of 2d coords in picture
    typedef Array2D<Pos2D> DetectedGrid;

    struct CalibrationData{
      // list of detected images (row-major order of points)
      std::vector<DetectedGrid> data;
      //imagesize of data
      Size imageSize;
    };

    ///computes the calibration for given CalibrationData
    /**
        * @param data contains detected grids for calibration
        */
    static Result optimize(const CalibrationData &data);

  };
}

