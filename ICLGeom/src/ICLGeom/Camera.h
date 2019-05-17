/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/Camera.h                           **
** Module : ICLGeom                                                **
** Authors: Erik Weitnauer, Christof Elbrechter                    **
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
#include <ICLUtils/Size.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Rect.h>
#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Array2D.h>
#include <ICLGeom/PlaneEquation.h>
#include <ICLGeom/ViewRay.h>


// the icl namespace
namespace icl {
  namespace geom{
    /// Camera class
    /** This camera class implements a model of a central projection camera with
        finite focal length. It is very general and can be applied to most cameras,
        e.g. CCD cameras. Because it assumes a linear projection, any distortion in
        the camera image should be corrected before using it in this class.

        <h2> The Camera Model</h2>

        The camera model was explicitly designed close to the camera model that is used in
        OpenGL. It is described by a set of extrinsic and intrinsic parameters:

        - Extrinsic camera parameters (position and orientation of the camera in the world)
          - <b>p</b> the camera position vector.
          - <b>n</b> the image plane's normal vector (sometimes called the view-vector)
            the norm vector is directed from the camera center to the scene.
          - <b>u</b> which defines the "roll"-angle of the camera. It points into
            the positive y-direction of the image-plane (which means that it will
            normally, despite its name, point from the center of the camera towards
            its bottom side) and is perpendicular to the norm vector.
          - <b>h</b> the horizontal vector pointing to the positive x-direction of the
            image plane is computed based on norm and up vector. It forms a right-handed
            coordinate system together with them.
        - Intrinsic Parameters
          - <b>f</b> The focal length is the distance between the lense and the image plane
            of the camera
          - <b>m<sub>x</sub>, m<sub>y</sub></b> sampling resolution on the camera image. In case of CCD
            cameras this is the resolution of the sensor chip in [pixel/mm]
          - <b>p<sub>x</sub>, p<sub>y</sub></b> the offset between the center of the image plane and the
            principal point of the camera, in [pixel]
          - <b>s</b> The skew parameter is zero, when the x- and y-axis of the
            image plane are perpendicular to each other. This should normally be
            the case.

        The Intrinsic camera parameters are used to create the camera's projection matrix <b>P</b>

        \f[
        P = \left(\begin{array}{cccc}
           fm_x & s     & p_x & 0 \\
           0    & f m_y & p_y & 0 \\
           0    & 0     & 0 &   0 \\
           0    & 0     & 1 &   0
        \end{array}\right)
        \f]

        Please note, that OpenGL's definition of the projection matrix looks different.
        OpenGL uses a flipped y-axis, and it's definition of projection also contains
        entries in the 3rd row. In our case, z values are not needed after the projection.

        The cameras coordinate system transformation matrix <b>C</b> is defined by:

        \f[
        C = \left(\begin{array}{cc}
           h^T & -h^T p \\
           u^T & -u^T p \\
           n^T & -n^T p \\
           (0,0,0) & 1  \\
        \end{array}\right)
        \f]

        Together, <b>P</b> and <b>C</b> are used to describe the projection model of the camera.
        A 3D utils::Point <b>p<sub>w</sub></b> in the world is projected to the camera screen <b>p<sub>s</sub></b> by

        \f[
        p_s' = homogenize(P C p_w)
        \f]

        <b>p<sub>s</sub></b> contains just the first two components of <b>p'<sub>s</sub></b>.
        The <em>homogenized</em> operation devided a homogeneous 3D vector by it's 4th component.

        In literature, sometimes are simgle 3x4 camera matrix is used. We call this matrix the
        camera's <em><b>Q</b>-matrix</em>. The matrix contains all information that is neccessary for
        creating a camera. Usually, it can be decomposed into <b>P</b> and <b>C</b> using
        QR-decomposition. The icl::Camera class provides function to directly obtain a camera's
        <b>Q</b>-matrix. <b>Q</b> is defined by the first two and the last row of the matrix product <b>P C</b>.


        <h2>Camera Calibration</h2>

        Valid camera instances can either be set up manually, or they can be created
        by camera calibration. ICL uses a quite simple yet very powerful calibration technique,
        which needs a set of at least 8 non-coplanar world points and their corresponding image
        coordinates. ICL features an easy to use camera calibration tool, which is discribed on
        ICL's <a href="www.iclcv.org">website</a>.
    */

    class ICLGeom_API Camera {
      public:

      struct ICLGeom_API RenderParams {
        utils::Size chipSize;    //!< chip size in [pixels] for transformation to chip coordinates
        float clipZNear;  //!< position of the near clipping plane in [mm]
        float clipZFar;   //!< position of the far clipping plane in [mm]
        utils::Rect viewport;    //!< in [pixel]
        float viewportZMin, viewportZMax; //!< interval for the z coordinates in viewport

        RenderParams(const utils::Size &chipSize=utils::Size::VGA, float clipZNear=1.0, float clipZFar=10000.,
          const utils::Rect &viewport=utils::Rect(0,0,640,480), float viewportZMin=0.,
          float viewportZMax=1.):
          chipSize(chipSize), clipZNear(clipZNear), clipZFar(clipZFar), viewport(viewport),
          viewportZMin(viewportZMin), viewportZMax(viewportZMax) {}
      };

      /// We need at least 6 Data points in general positions
      struct NotEnoughDataPointsException  : public utils::ICLException{
        NotEnoughDataPointsException():utils::ICLException(__FUNCTION__){}
      };

      /// internal typedef
      typedef math::FixedMatrix<icl32f,3,3> Mat3x3;

      /** @{ @name constructors */

      /// basic constructor that gets all possible parameters
      Camera(const Vec &pos=Vec(0,0,10,1),
               const Vec &norm=Vec(0,0,-1,1),
               const Vec &up=Vec(1,0,0,1),
               float f=3,
               const utils::Point32f &principalPointOffset=utils::Point32f(320,240),
               float sampling_res_x = 200,
               float sampling_res_y = 200,
               float skew = 0,
               const RenderParams &renderParams = RenderParams()): m_pos(pos), m_norm(norm), m_up(up), m_f(f),
             m_px(principalPointOffset.x), m_py(principalPointOffset.y),
             m_mx(sampling_res_x), m_my(sampling_res_y), m_skew(skew), m_renderParams(renderParams) {}

      /// loads a camera from given file
      /** @param filename file name of valid configuration file (in ICL's ConfigFile core::format)
          @param prefix valid prefix that determines wheret to find the camera within the
                 given config file (note, that this prefix must end with '.')
          */
      Camera(const std::string &filename, const std::string &prefix="config.")  ;
      /// loads a camera from given input stream
      /** @param configDataStream stream object to read and interpret input file name of valid configuration file (in ICL's ConfigFile core::format)
          @param prefix valid prefix that determines where to find the camera within the
                 given config file (note, that this prefix must end with '.')
          */
      Camera(std::istream &configDataStream, const std::string &prefix="config.")  ;


      /** @} @{ @name static creation functions */
      /// Compute all camera parameters from the 4x3 projection matrix.
      static Camera createFromProjectionMatrix(const math::FixedMatrix<icl32f,4,3> &Q, float focalLength=1);

      /// Uses the passed world point -- image point references to estimate the projection parameters.
      /** At least 6 data points references are needed. It is not possible to estimate the
          focal length f directly, but only the products f*m_x and f*m_y (which is sufficient for
          defining the projection). Therefore an arbitrary value for f != 0 may be passed to the
          function.
          The method minimizes the algebraic error with the direct linear transform algorithm in
          which an SVD is used.
      */
      static Camera calibrate(std::vector<Vec> Xws, std::vector<utils::Point32f> xis, float focalLength=1,
                              bool performLMAOptimization=true)
        ;

      /// Uses the passed world point -- image point references to estimate the projection parameters.
      /** Same as the method calibrate, but using a pseudoinvers instead of the SVD for the estimation.
          This method is less stable and less exact. */
      static Camera calibrate_pinv(std::vector<Vec> Xws, std::vector<utils::Point32f> xis, float focalLength=1,
                                   bool performLMAOptimization=true)
        ;

      /// performs extrinsic camera calibration using a given set of 2D-3D correspondences and the given intrinsic camera calibration data
      /** @see Camera::calibrate_extrinsic((std::vector<Vec>,std::vector<utils::Point32f>,float,float,float,float,float) */
      static Camera calibrate_extrinsic(const std::vector<Vec> &Xws, const std::vector<utils::Point32f> &xis,
                                        const Camera &intrinsicCamValue, const RenderParams &renderParams=RenderParams(),
                                        bool performLMAOptimization=true)
      ;

      /// performs extrinsic camera calibration using a given set of 2D-3D correspondences and the given intrinsic camera calibration data
      /** @see Camera::calibrate_extrinsic((std::vector<Vec>,std::vector<utils::Point32f>,float,float,float,float,float) */
      static Camera calibrate_extrinsic(const std::vector<Vec> &Xws, const std::vector<utils::Point32f> &xis,
                                        const Mat &camIntrinsicProjectionMatrix, const RenderParams &renderParams=RenderParams(),
                                        bool performLMAOptimization=true)
      ;

      /// performs extrinsic camera calibration using a given set of 2D-3D correspondences and the given intrinsic camera calibration data
      /** In many cases, when camera calibration is performed in a realy scene, it is quite difficult to place the calibration
          object well alligned and still in such a way that it covers a major fraction of the camera image. Instead, the calibration
          object usually is rather small, which leads to a poor calibration performace.

          Tests showed (here, we used rendered images of a calibration object to get real ground-truth data) that in case
          of the common calibration performed by Camera::calibrate or Camera::calibrate_pinv sometimes lead to extreme camera
          positioning errors when the calibration object is too far away. The reason for this is that a far-awys calibration object
          looks more and more isometric in the camera image which makes it more and more difficult for the method to distinguish
          between a closer camera with a shot focal length or a futher-away camera with a higher focal length. Unfortunately,
          this effect often seems to be optimized by favoring one or another of these quantities, so even z-positioning errors of
          more than 10 cm could be observed. This effect is less important, when using such a camera in a multi-camera setup,
          as here, the error that is introduced is similarly smally as the missing-vanishing-point-effect that caused it
          in the first place. However, in other applications or even when calibrating a Kinect-Device, the camera position
          defines the basis for point cloud creation, so here a better positioning is mandatory.

          To avoid these issues, it is recommended to perform the camera calibration in two steps. In the first step, only the
          intrinsic camera parameters are optimized. During this step, camera and calibration object can be positioned arbitrarily
          so that the calibration object perfectly covers the whole image space. In the 2nd step, the already obtained intrinsic
          parameters are fixed so that only the camera's extrinsic parameters (position and orientation) has to be optimized

          \section ALG Algorithm

          Outgoing from the a the camera's projection law: (u',v', 0, h) = P C x, which results in homogenized real screen coordinates
          u = u'/h and v = v'/h the method internally creates a system of linear equation to get a least square algebraic optimum.

          Let \f[
           P = \left(\begin{array}{cccc}
           fm_x & s     & p_x & 0 \\
           0    & f m_y & p_y & 0 \\
           0    & 0     & 0 &   0 \\
           0    & 0     & 1 &   0
         \end{array}\right)
          \f]

          be the projection matrix and C the camera's coordinate frame transformation matrix (which transforms points from
          the world frame into the local camera frame. Further, we denote the lines of C by C<sub>1</sub>, C<sub>2</sub>,
          C<sub>3</sub> and C<sub>4</sub>.

          The projection law leads to the two formulas for u and v:

          \f[
          u = \frac{fm_x C_1 x + s C_2 x + p_x C_3 x}{C_3 x}
          \f]
          and
          \f[
          v = \frac{fm_y C_2 x + p_y C_3 x}{C_3 x}
          \f]

          which can be made linear wrt. the coefficients of C by deviding by C<sub>3</sub>x  and then bringing the
          left part to the right, yielding:

          \f[
          fm_x C_1 x + s C_2 x + (p_x - u) C_3 x = 0
          \f]
          and
          \f[
          fm_y C_2 x + (p_y - v) C_3 x = 0
          \f]

          This allows us to create a linear system of equations of the shape Ax=0, which is solved
          by finding the eigenvector to the smallest eigenvalue of A (which is internally done using SVD).

          For each input point x=(x,y,z,1) and corresponding image point (u,v),
          we define d<sub>u</sub> = p<sub>x</sub> - u and  d<sub>v</sub> = p<sub>y</sub> - v
          in order to get two lines of the equation

          \f[
           P = \left(\begin{array}{cccccccccccc}
           fm_x x & fm_x y & fm_z x & fm_x & s x & s y & s z & s & d_u x & d_u y & d_u z & d_u \\
           0 & 0 & 0 & 0 & fm_y x & fm_y y & fm_y x & fm_y & d_v x & d_v y & d_v z & d_v \\
           ...
         \end{array}\right) ( C_1 C_2 C_3 )^T
          \f].

          Solving Px=0, yields a 12-D vectors whose elements written row-by-row into the first 3 lines
          of a 4x4 identity matrix is almost our desired relative camera transform matrix C. Actually,
          we receive only a scaled version of the solution C' = kC, which has to be normalized to make the
          rotation part become unitary. The normalization is performed using RQ-decomposition on the
          rotation part R' (upper left 3x3 sub-matrix of C'), which decomposes R' into a product R'=R*Q.
          Here, per definition, the resulting Q is unitary and therefore it is identical to the actual
          desired rotation part of C. As the RQ-decomposition does two things at once, it normalizes
          and it orthogonalizes the rows and colums of R', the factor k that is needed to also scale the
          translation part of C' cannot simply be extracted using e.g. R'(0,0) / Q(0,0) or a mean
          fraction between corresponding elements. Instead, we use the trace(R)/3, which could be shown to
          provide better results.

          \section LMA Further non-linear optimization

          Optionally, the resulting camera parameters, which result from an algebraic error minimization,
          can be optimized wrt. the pixel projection error. Experiments showed, that this can reduce the actual
          error by a factor of 10. The LMA-based optimization takes slightly longer then the normal
          linear optimization, but it is still real-time applicable and it should not increase the error, which
          is why, it is recommended to be used!

          \section PARAMS The function parameters

          fx, fy are the known camera x- and y-focal lengths, s is the skew, and px and py is the principal point offset */
      static Camera calibrate_extrinsic(std::vector<Vec> Xws, std::vector<utils::Point32f> xis,
                                        float fx, float fy, float s, float px ,float py,
                                        const RenderParams &renderParams=RenderParams(),
                                        bool performLMAOptimization=true)
      ;

      /// performs a non-linear LMA-based optimization to improve camera calibration results
      static Camera optimize_camera_calibration_lma(const std::vector<Vec> &Xws,
                                                    const std::vector<utils::Point32f> xis,
                                                    const Camera &init);

      /** @} @{ @name putils::rojection functions */

      // projections normal
      /// Project a world point onto the image plane. Caution: Set last component of world points to 1.
      utils::Point32f project(const Vec &Xw) const;
      /// Project a vector of world points onto the image plane.  Caution: Set last component of world points to 1.
      void project(const std::vector<Vec> &Xws, std::vector<utils::Point32f> &dst) const;
      /// Project a vector of world points onto the image plane. Caution: Set last component of world points to 1.
      const std::vector<utils::Point32f> project(const std::vector<Vec> &Xws) const;



      // projections OpenGL
      /// Project a world point onto the image plane.
      Vec projectGL(const Vec &Xw) const;
      /// Project a vector of world points onto the image plane.
      void projectGL(const std::vector<Vec> &Xws, std::vector<Vec> &dst) const;
      /// Project a vector of world points onto the image plane.
      const std::vector<Vec> projectGL(const std::vector<Vec> &Xws) const;


      // projection magic
      /// Returns a view-ray equation of given pixel location
      ViewRay getViewRay(const utils::Point32f &pixel) const;

      /// returns a list of viewrays corresponding to a given set of pixels
      /** This method is much faster than using getViewRay several times since the
          projection matrix inversion that is necessary must only be done once */
      std::vector<ViewRay> getViewRays(const std::vector<utils::Point32f> &pixels) const;

      /// returns a 2D array of all viewrays
      /** This method is much faster than using getViewRay several times since the
          projection matrix inversion that is necessary must only be done once */
      utils::Array2D<ViewRay> getAllViewRays() const;

      /// Returns a view-ray equation of given point in the world
      ViewRay getViewRay(const Vec &Xw) const;

      /// returns estimated 3D point for given pixel and plane equation
      Vec estimate3DPosition(const utils::Point32f &pixel, const PlaneEquation &plane) const ;
      /// calculates the intersection point between this view ray and a given plane
      /** Throws an utils::ICLException in case of parallel plane and line
          A ViewRay is defined by  \f$V: \mbox{offset} + \lambda \cdot \mbox{direction} \f$
          A Plane is given by \f$ P: < (X - \mbox{planeOffset}), \mbox{planeNormal}> = 0 \f$
          Intersection is described by
          \f$<(\mbox{offset} + \lambda \cdot \mbox{direction}) - \mbox{planeOffset},planeNormal> = 0\f$
          which yields:
          \f[ \lambda = - \frac{<\mbox{offset}-\mbox{planeOffset},\mbox{planeNormal}>}{<\mbox{direction},\mbox{planeNormal}>} \f]
          and .. obviously, we get no intersection if direction is parallel to planeNormal
      */
      static Vec getIntersection(const ViewRay &v,
                                 const PlaneEquation &plane) ;

      /** @} @{ @name complex setter and getter functions */

      // setters and getters
      /// set the norm and up vectors according to the passed rotation matrix
      void setRotation(const Mat3x3 &rot);
      /// set norm and up vectors according to the passed yaw, pitch and roll
      void setRotation(const Vec &rot);

      /// sets the camera's rotation and position from given 4x4 homogeneous matrix
      /** Note, that this function is designed to not change the camera
          when calling cam.setTransfromation(cam.getCSTrannsformationMatrix()).
          In other words, this means, that it will use the given rotation part R,
          as rotation matrix, but set the camera position to -R*t */
      void setTransformation(const Mat &m);

      /// this sets the camera to the given world tranformation
      /** If m is [R|t], the function sets the camera rotation to R
          and the position to t. Note that getRotation() will return R^-1 which is R^t */
      void setWorldTransformation(const Mat &m);


      /// adapts the camera transformation, so that the given frame becomes the world frame
      /** To this end, the camera's world transform is set to
          m.inv() * cam.getInvCSTransformationMatrix() */
      void setWorldFrame(const Mat &m);

      /// get world to image coordinate system transformation matrix
      Mat getCSTransformationMatrix() const;

      /// returns the transform of the camera wrt. world frame
      Mat getInvCSTransformationMatrix() const;

      /// get world to image coordinate system transformation matrix
      Mat getCSTransformationMatrixGL() const;
      /// get projection matrix
      Mat getProjectionMatrix() const;
      /// get the projection matrix as expected by OpenGL
      Mat getProjectionMatrixGL() const;
      Mat getViewportMatrixGL() const;

      /// returns the common 4x3 camera matrix
      math::FixedMatrix<icl32f,4,3> getQMatrix() const;

      /// returns the inverse QMatrix
      math::FixedMatrix<icl32f,3,4> getInvQMatrix() const;

      /// translates the current position vector
      inline void translate(const Vec &d) { m_pos += d; }

      std::string toString() const;

      /** @} @{ @name simple getter functions */

      const std::string &getName() const { return m_name; }
      const Vec &getPosition() const { return m_pos; }
      const Vec &getNorm() const { return m_norm; }
      const Vec &getUp() const { return m_up; }
      Vec getHoriz() const { return cross(m_up, m_norm); }
      float getFocalLength() const { return m_f; }
      utils::Point32f getPrincipalPointOffset() const { return utils::Point32f(m_px, m_py); }
      float getPrincipalPointOffsetX() const { return m_px; }
      float getPrincipalPointOffsetY() const { return m_py; }
      float getSamplingResolutionX() const { return m_mx; }
      float getSamplingResolutionY() const { return m_my; }
      float getSkew() const { return m_skew; }
      const RenderParams &getRenderParams() const { return m_renderParams; }
      RenderParams &getRenderParams() { return m_renderParams; }

      /** @} @{ @name simple setter functions */

      void setName(const std::string &name) { m_name = name; }
      void setPosition(const Vec &pos) { m_pos = pos; }
      void setNorm(const Vec &norm, bool autoOrthogonalizeRotationMatrix=false) {
        m_norm = norm; m_norm[3] = 0; m_norm.normalize(); m_norm[3] = 1;
        if(autoOrthogonalizeRotationMatrix) orthogonalizeRotationMatrix();
      }
      /// gets automatically normalized
      void setUp(const Vec &up, bool autoOrthogonalizeRotationMatrix=false) {
        m_up = up; m_up[3] = 0; m_up.normalize(); m_up[3] = 1;
        if(autoOrthogonalizeRotationMatrix) orthogonalizeRotationMatrix();
      }
      /// extracts the current rotation matrix and uses gramSchmidth orthogonalization
      void orthogonalizeRotationMatrix();

      void setFocalLength(float value) { m_f = value; }
      void setPrincipalPointOffset(float px, float py) { m_px = px; m_py = py; }
      void setPrincipalPointOffset(const utils::Point32f &p) { m_px = p.x; m_py = p.y; }
      void setSamplingResolutionX(float value) { m_mx = value; }
      void setSamplingResolutionY(float value) { m_my = value; }
      void setSamplingResolution(float x, float y) { m_mx = x; m_my = y; }
      void setSkew(float value) { m_skew = value; }
      void setRenderParams(const RenderParams &rp) { m_renderParams = rp; }


      /** @} @{ @name more complex manipulation functions */

      /// Changes the camera resolution and adapts dependent values
      /** Internally, this function also adapts the render parameters chipSize and viewport
          Furthermore, the prinizipal-point-offset is automatically set to the center of
          the new screen
      */
      void setResolution(const utils::Size &newScreenSize);

      /// Changes the camera resolution and adapts dependent values
      /** Internally, this function also adapts the render parameters chipSize and viewport
          Furthermore, the prinizipal-point-offset is set to the new given value
      */
      void setResolution(const utils::Size &newScreenSize, const utils::Point &newPrincipalPointOffset);

      /// returns the current chipSize (camera resolution in pixels)
      inline const utils::Size &getResolution() const { return m_renderParams.chipSize; }
      /** @} @{ @name 3D-position estimation */

      /// computes the 3D position of a n view from n cameras
      /** @param cams list of cameras
          @param UVs list of image points (in image coordinates)
          @param removeInvalidPoints if this flag is set to true,
                 all given points are checked to be within the cameras
                 viewport. If not, these points are removed internally.

          <h3> Method: </h3>

          This function uses a standard linear approach using a pseudo-inverse to
          solve the problem in a "least-square"-manner:

          The camera is essentially represented by the 3x4-Q-Matrix, which can be
          obtained using icl::Camera::getQMatrix() const. Q is defined as follows:

          <pre>
                      | -- x --  tx |
          Q = [R|t] = | -- y --  ty |
                      | -- z --  tz |
          </pre>

          The camera projection is trivial now. For a given projected point [u,v]'
          (in image coordinates) and a position in the world Pw (homogeneous):

          <pre>
          [u,v,1*]' = hom( Q Pw )
          </pre>

          Where '1*' becomes a real 1.0 just because of the homogenization step
          using hom(). Component-wise, this can be re-written as:

          <pre>
          u  = x Pw + tx
          v  = y Pw + ty
          1* = z Pw + tz
          </pre>

          In order to ensure, '1*' becomes a real 1.0, the upper two equations
          have to be devided by (z Pw + tz), which provides us the following two
          equations:

          <pre>
          u = (x Pw + tx) / (z Pw + tz)
          v = (y Pw + ty) / (z Pw + tz)
          </pre>

          These equations have to be reorganized so that Pw can be factored out:

          <pre>
          (u z - x) Pw = tx - u tz
          (v z - y) Pw = ty - v tz
          </pre>

          Now we can write this in matrix notation again:

          <pre>
          A Pw = B      ,where


          A = | u z - x |
              | v z - y |

          B = | tx - u tz |
              | ty - v tz |

          </pre>

          The obviously under-determined equation-system above uses only a single camera. If
          we put the results from several views together into a single equation system, it
          becomes unambigoulsly solvable using a pseudo-inverse approach:

          <pre>
          | A1 |      | B1 |             | A1 |+ | B1 |
          | A2 | Pw = | B2 |    => Pw =  | A2 |  | B2 |
          |....|      |....|             |....|  |....|
          </pre>

          where 'A+' means the pseudo-inverse of A
      */
      static Vec estimate_3D(const std::vector<Camera*> cams,
                             const std::vector<utils::Point32f> &UVs,
                             bool removeInvalidPoints=false) ;

      /// estimate world frame pose of object specified by given object points
      Mat estimatePose(const std::vector<Vec> &objectCoords,
                       const std::vector<utils::Point32f> &imageCoords,
                       bool performLMAOptimization=true);

      /// multiview 3D point estimation using svd-based linear optimization (should not be used)
      /** This functions seems to provide false results for more than 2 views:
          use estimate_3D instead*/
      static Vec estimate_3D_svd(const std::vector<Camera*> cams,
                                 const std::vector<utils::Point32f> &UVs);

      /** @}*/

      /// parses a given image-undistortion file and creates a camera
      /** Please note that only the camera's intrinsic parameters are in the file,
          so extrinsic parameters for position and orientation will be identical to a
          defautl created Camera instance. The undistortion file must use the model
          "MatlabModel5Params". The given resolution is not used if the given
          file is a standard camera-calibration file */
      static Camera create_camera_from_calibration_or_udist_file(const std::string &filename) ;

      protected:
      static Mat createTransformationMatrix(const Vec &norm, const Vec &up, const Vec &pos);

      private:
      // General Parameters
      std::string m_name;   //!< name of the camera (visualized in the scene if set)

      // External Parameters
      Vec m_pos;        //!< center position vector
      Vec m_norm;       //!< normal vector of image plane
      Vec m_up;         //!< vector pointing to pos. y axis on image plane

      // Internal Parameters
      float m_f;        //!< focal length
      float m_px, m_py; //!< principal point offset
      float m_mx, m_my; //!< sampling resolution
      float m_skew;     //!< skew parameter in the camera projection, should be zero

      // Rendering Parameters
      RenderParams m_renderParams;

      ////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////

      /// internally used utility function
      static void checkAndFixPoints(std::vector<Vec> &worldPoints, std::vector<utils::Point32f> &imagePoints)
        ;
      /// intenal helper function
      static void load_camera_from_stream(std::istream &is,
                                          const std::string &prefix,
                                          Camera &cam);
    };

    /// ostream operator (writes camera in XML core::format)
    ICLGeom_API std::ostream &operator<<(std::ostream &os, const Camera &cam);

    /// istream operator parses a camera from an XML-string
    ICLGeom_API std::istream &operator>>(std::istream &is, Camera &cam) ;

  } // namespace geom
} // namespace icl

