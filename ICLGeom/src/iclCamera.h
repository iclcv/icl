#ifndef ICL_CAMERA_H
#define ICL_CAMERA_H

#include <iclLockable.h>
#include <iclObject.h>
#include <iclSize.h>

// the icl namespace
namespace icl{

   
  /// Camera class 
  /** The camera class implements 3 different homogeneous transformations:
      -# World-Coordinate-System (CS) to Cam-CS transformation
      -# Projection into the image plane
      -# A view-port transformation 
      
      Each Szene has its own camera instance, which is used internally to 
      transform the objects coordinates into the current virtual image plane
      and its view-port.\n
      The camera is characterized by 3 unity vectors:
      - <b>norm</b> which is the image planes normal vector (sometimes called the view-vector)
      - <b>up</b> which defines the "roll"-angle of the camera. It points into the positive y-direction
        of the image-plane and is perpendicular to the norm vector
      - <b>pos</b> the camera position vector

      Additionally each camera has a fixed focal length (parameter f) and
      a view-port size (and offset, which is Point::null, most of the times).\n
      The camera coordinate system can be transformed (in particular rotated and translated). This changes
      will only affect the cameras parameter vectors norm, up and pos. By calling the getTransformationMatrix
      function, it is possible to get a combined homogeneous transformation matrix, which transforms and projects
      objects into the given view-port.
  */
  struct Camera : public Lockable{

    /// Creates a camera from given position and rotation vector
    /** If the camera rotation is 0,0,0, the cameras normal vector is (0,0,1) and 
        it's up vector is (0,1,0) */
    Camera(const Vec &pos, const Vec &rot, const Size &viewPortSize,
           float f, float zNear=0.01, float zFar=1000, bool rightHandedCS=true);
    
    /// common constructor with given view port size
    /** Just as the constructor below, but without viewport offset*/
    inline Camera(const Vec &pos=Vec(0,0,10,0),
                  const Vec &norm=Vec(0,0,-1,0), 
                  const Vec &up=Vec(1,0,0,0),
                  const Size &viewPortSize=Size::VGA,
                  float f=-45, 
                  float zNear=0.01,
                  float zFar=1000,
                  bool rightHandedCS=true){
      init(pos,norm,up,Rect(Point::null,viewPortSize),f,zNear,zFar,rightHandedCS);
    }

    /// Creates a new camera with given parameters
    /** @param pos position of the camera center
        @param norm view-vector of the camera
        @param up up-vector of the camera
        @param viewPort 
              OLD: ??view-port size, which must be equal
                  to the view-port (image-size) of the
                  given ICLDrawWidget if the corresponding
                  render-function is called, resp. the 
                  size of the Img32f for the other render-
                  function.
        @param f focal length in mm or Field of view in degree
                  if f is negative, it is interpretet in OpenGL's
                  gluPerspective manner as opening angle of the camera
                  view field. Otherwise, f is interpretet as focal
                  length in mm.
        @param zNear nearest clipping plane (clipping is not yet implemented, 
                     but zNear is used to estimation the camera projection internally.)
        @param zFar farest clipping plane (clipping is not yet implemented, 
                    but zNear is used to estimation the camera projection internally.)
    */
    inline Camera(const Vec &pos,
                  const Vec &norm,
                  const Vec &up,
                  const Rect &viewPort,
                  float f=-45, 
                  float zNear=0.01,
                  float zFar=1000,
                  bool rightHandedCS=true){
      init(pos,norm,up,viewPort,f,zNear,zFar,rightHandedCS);
    }
    
    
    /// Default copy constructor copies all but the internal matrix buffer
    inline Camera(const Camera &cam){
      init(cam.m_pos,cam.m_norm,cam.m_up,cam.m_viewPort,cam.m_F,cam.m_zNear,cam.m_zFar);
    }    
   
    /// re-initializes the camera with given data
    void init(const Vec &pos,const Vec &norm, const Vec &up, 
              const Rect &viewPort, float f, float zNear, float zFar,
              bool rightHandedCS=true);
    
    /// returns the camera transformation matrix
    Mat getTransformationMatrix() const;
    
    /// returns the matrix, that transforms vectors into the camera coordinate system
    /** (internally called by getTransformationMatrix) */
    Mat getCoordinateSystemTransformationMatrix() const;
    
    /// returns the matrix, that projects vector to the camera plane
    /** (internally called by getTransformationMatrix) */
    Mat getProjectionMatrix() const;
    
    /// returns the complete camera transformation
    FixedMatrix<float,4,2> get4Dto2DMatrix() const;
    
    /// returns the current pos-vector
    inline const Vec &getPos() const { return m_pos; }

    /// returns the current norm-vector
    inline const Vec &getNorm() const { return m_norm; }

    /// returns the current up-vector
    inline const Vec &getUp() const{ return m_up; }

    /// returns the current focal length
    inline float getFocalLength() const{ return m_F; }

    /// sets new up  vector
    inline void setUp(const Vec &newUp){ 
      m_up = normalize3(newUp,0); 
    }

    /// sets new norm  vector
    inline void setNorm(const Vec &newNorm){ 
      m_norm = normalize3(newNorm,0); 
    }
    
    /// returns the current horizontal vector (norm x up)
    Vec getHorz()const;

    /// show some camera information to std::out
    void show(const std::string &title="") const;
    
    /// transforms norm and up by the given matrix
    inline void transform(const Mat &m){
      m_norm *= m;
      m_up *= m;
    }

    /// rotates norm and up by the given angles
    /** @param arcX angle around the x-axis
        @param arcY angle around the y-axis
        @param arcZ angle around the z-axis
    **/
    inline void rotate(float arcX, float arcY, float arcZ){
      transform(create_hom_4x4(arcX,arcY,arcZ));
    }
    /// translates the current pos-vector
    inline void translate(float dx, float dy, float dz){ 
      translate(Vec(dx,dy,dz,0)); 
    }
    /// translates the current pos-vector
    inline void translate(const Vec &d){
      m_pos+=d;
    }
    
    /// sets the focal length
    void setFocalLength( float f){ m_F = f; }
    
    /// returns the current viewport matrix
    Mat getViewPortMatrix() const;
    
    /// sets the current view port
    inline void setViewPort(const Rect &viewPort){
      m_viewPort = viewPort;
    }
    /// returns the current view port
    inline const Rect &getViewPort() const{
      return m_viewPort;
    }
    
    /// Transforms a point at given camera pixel location into the camera frame
    Vec screenToCameraFrame(const Point32f &pixel) const;
    
    /// Transforms a point from the camera coordinate System into the world coordinate system
    Vec cameraToWorldFrame(const Vec &Xc) const;
    
    /// Returns where a given pixel (on the chip is currently in the world)
    /** i.e. The world location of the camera's CCD-Element for a given pixel */
    Vec screenToWorldFrame(const Point32f &pixel) const;
    
    /// This is a view-ray's line equation in parameter form
    struct ViewRay{
      ViewRay(){}
      ViewRay(const Vec &o, const Vec &d):offset(o),direction(d){offset[3]=direction[3]=1;}
      Vec offset;
      Vec direction;
    };
    
    /// Returns a view-ray equation of given pixel location
    ViewRay getViewRay(const Point32f &pixel) const;

    /// Returns a view-ray equation of given point in the world
    ViewRay getViewRay(const Vec &Xw) const;
    
    /// Projects a world point to the screen
    Point32f project(const Vec &Xw) const;
    
    /// Projects a set of points (just an optimization)
    const std::vector<Point32f> project(const std::vector<Vec> &Xws) const;

    /// Projects a set of points (just an optimization)
    void project(const std::vector<Vec> &Xws, std::vector<Point32f> &dst) const;

    /// Projects a set of points (results are x,y,z,1)
    void project(const std::vector<Vec> &Xws, std::vector<Vec> &dstXYZ) const;
    
    private:
    Vec m_pos;        //!< center position vector
    Vec m_norm;       //!< norm vector
    Vec m_up;         //!< up vector
    float m_F;        //!< focal length
    float m_zNear;    //!< nearest clipping plane (must be > 0 and < zFar)
    float m_zFar;     //!< farest clipping plane (must be > 0 and < zFar)
    
    Rect m_viewPort;  //!< current viewport e.g. (0,0,640,480) for a default VGA camera
    
    bool m_rightHandedCS; //!< is camera coordinate system right handed or not (default: true)
  };
}

#endif
