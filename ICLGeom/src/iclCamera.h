#ifndef ICL_CAMERA_H
#define ICL_CAMERA_H

#include <iclObject.h>
#include <iclSize.h>

/// the icl namespace
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
        @param f focal length
        @param imageSize view-port size*/
    Camera(const Vec &pos=0,const Vec &norm=0, const Vec &up=0, float f=0, const Size &imageSize = Size::null):
      pos(pos),norm(norm),up(up),f(f),imageSize(imageSize){}
    
    /// returns the camera transformation matrix
    const Mat &getTransformationMatrix();
    
    /// returns the current pos-vector
    inline const Vec &getPos() const { return pos; }

    /// returns the current norm-vector
    inline const Vec &getNorm() const { return norm; }

    /// returns the current up-vector
    inline const Vec &getUp() const{ return up; }

    /// returns the current view-port size
    inline const Size &getImageSize()const{ return imageSize; }

    /// returns the current focal length
    inline float getFocalLength() const{ return f; }
    
    /// returns the current horizontal vector (norm x up)
    inline Vec getHorz()const{ return norm.cross(up); }

    /// show some camera information to std::out
    void show(const std::string &title) const;

    /// transforms norm and up by the given matrix
    void transform(const Mat &m){
      //m.show("camera transformation \n");
      norm *= m;
      up*=m;
    }
    
    /// rotates norm and up by the given angles
    /** @param arcX angle around the x-axis
        @param arcY angle around the y-axis
        @param arcZ angle around the z-axis
    **/
    void rotate(float arcX, float arcY, float arcZ){
      transform(Mat::rot(arcX,arcY,arcZ));
    }
    /// translates the current pos-vector
    void translate(float dx, float dy, float dz){ 
      translate(Vec(dx,dy,dz,0)); 
    }
    /// translates the current pos-vector
    void translate(const Vec &d){
      pos+=d;
    }
    
    /// sets the focal length
    void setFocalLength( float f){ this->f = f; }
    

    private:
    Vec pos;        ///!< center position vector
    Vec norm;       ///!< norm vector
    Vec up;         ///!< up vector
    float f;        ///!< focal length
    Size imageSize; ///!< image size
    
    Mat M; //!< internal buffer for the current transformation matrix
    
  };
}

#endif
