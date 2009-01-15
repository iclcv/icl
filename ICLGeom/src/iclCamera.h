#ifndef ICL_CAMERA_H
#define ICL_CAMERA_H

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
      a view-port size.\n
      The camera coordinate system can be transformed (in particular rotated and translated). This changes
      will only affect the cameras parameter vectors norm, up and pos. By calling the getTransformationMatrix
      function, it is possible to get a combined homogeneous transformation matrix, which transforms and projects
      objects into the given view-port.
  */
  struct Camera{
    /// Create a new camera with given parameters
    /** @param pos position of the camera center
        @param norm view-vector of the camera
        @param up up-vector of the camera
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
  Camera(const Vec &pos=Vec(0,0,10,0),
         const Vec &norm=Vec(0,0,-1,0), 
         const Vec &up=Vec(1,0,0,0), 
         float f=-45, 
         float zNear=0.1,
         float zFar=100
         );

    
    /// Default copy constructor copies all but the internal matrix buffer
  Camera(const Camera &cam):
    m_oPos(cam.m_oPos),m_oNorm(cam.m_oNorm),m_oUp(cam.m_oUp),m_fF(cam.m_fF),
      m_fZNear(cam.m_fZNear),m_fZFar(cam.m_fZFar){}
    
    
    /// returns the camera transformation matrix
    const Mat &getTransformationMatrix() const;
    
    /// returns the matrix, that transforms vectors into the camera coordinate system
    /** (internally called by getTransformationMatrix) */
    Mat getCoordinateSystemTransformationMatrix() const;
    
    /// returns the matrix, that projects vector to the camera plane
    /** (internally called by getTransformationMatrix) */
    Mat getProjectionMatrix() const;
    
    /// returns the current pos-vector
    inline const Vec &getPos() const { return m_oPos; }

    /// returns the current norm-vector
    inline const Vec &getNorm() const { return m_oNorm; }

    /// returns the current up-vector
    inline const Vec &getUp() const{ return m_oUp; }

    /// returns the current focal length
    inline float getFocalLength() const{ return m_fF; }
    
    /// returns the current horizontal vector (norm x up)
    inline Vec getHorz()const{ return cross(m_oNorm,m_oUp); }

    /// show some camera information to std::out
    void show(const std::string &title) const;

    /// transforms norm and up by the given matrix
    void transform(const Mat &m){
      m_oNorm *= m;
      m_oUp *= m;
    }
    
    /// rotates norm and up by the given angles
    /** @param arcX angle around the x-axis
        @param arcY angle around the y-axis
        @param arcZ angle around the z-axis
    **/
    void rotate(float arcX, float arcY, float arcZ){
      transform(create_hom_4x4(arcX,arcY,arcZ));
    }
    /// translates the current pos-vector
    void translate(float dx, float dy, float dz){ 
      translate(Vec(dx,dy,dz,0)); 
    }
    /// translates the current pos-vector
    void translate(const Vec &d){
      m_oPos+=d;
    }
    
    /// sets the focal length
    void setFocalLength( float f){ this->m_fF = f; }
    

    private:
    Vec m_oPos;        ///!< center position vector
    Vec m_oNorm;       ///!< norm vector
    Vec m_oUp;         ///!< up vector
    float m_fF;        ///!< focal length
    float m_fZNear;     ///!< nearest clipping plane (must be > 0 and < zFar)
    float m_fZFar;      ///!< farest clipping plane (must be > 0 and < zFar)
    
    mutable Mat m_oMatBuf; //!< internal buffer for the current transformation matrix
    
  };
}

#endif
