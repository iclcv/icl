#include "iclCamera.h"
#include <math.h>

namespace icl{
  
  void Camera::init(const Vec &pos,
                    const Vec &norm,
                    const Vec &up,
                    const Rect &viewPort,
                    float f,
                    float zNear,
                    float zFar){
    m_pos = pos;
    m_norm = norm;
    m_up = up;
    m_zNear = zNear;
    m_zFar = zFar;
    m_F = f>0 ? f : cos((-f/2)*M_PI/180)/sin((-f/2)*M_PI/180);
    m_viewPort = viewPort;
  }
  
  Mat Camera::getCoordinateSystemTransformationMatrix() const{
    // Transformation matrix ** T **
    /* [ --- hh --- | -hh.p ]
       [ --- uu --- | -uu.p ]
       [ --- nn --- | -nn.p ]
       [--------------------]
       [ 0   0   0  |   1   ]
    */
    Vec nn = normalize(m_norm);
    Vec ut = normalize(m_up);
    Vec hh = cross(nn,ut);
    Vec uu = cross(hh,nn);

    Mat T;
    T.col(0) = hh;
    T.col(1) = uu;
    T.col(2) = -nn;
    T.col(3) = Vec(0.0);
    T = T.transp();
    //    T[3] = Vec(0,0,0,1);
    T.col(3) =-(T*m_pos);
    T(3,3) = 1;
    
    return T;
  }
  
  Mat Camera::getProjectionMatrix() const{
    float A = (m_zFar + m_zNear)/(m_zNear - m_zFar);
    float B = (2*m_zFar*m_zNear)/(m_zNear - m_zFar);
    
    return  Mat ( m_F , 0   ,   0,  0,
                  0    , m_F,   0,  0,
                  0    , 0   ,   A,  B,
                  0    , 0   ,  -1,  0 );
  }


  Mat Camera::getTransformationMatrix() const {
    return getProjectionMatrix()*getCoordinateSystemTransformationMatrix();
  }
  
  void Camera::show(const std::string &title) const{
    printf("cam: %s \n",title.c_str());
    std::cout << "norm:\n" << m_norm << "\nup:\n" << m_up << "\npos:\n" << m_pos << "\nf: " << m_F << std::endl;
  }

  Mat Camera::getViewPortMatrix() const{
    //float dx = m_oViewPort.width/2;
    //float dy = m_oViewPort.height/2;
    
    float dx = (m_viewPort.left()+m_viewPort.right())/2;
    float dy = (m_viewPort.top()+m_viewPort.bottom())/2;
    float slope = iclMin(m_viewPort.width/2,m_viewPort.height/2);
    return  Mat ( slope , 0     , 0 , dx,
                  0     , slope , 0 , dy,
                  0     , 0     , 0 , 0 ,
                  0     , 0     , 0 , 1 );
  }
  
  Vec Camera::screenToCameraFrame(const Point32f &pixel) const{
    // optimization:
    float dx = m_viewPort.x+m_viewPort.width/2;
    float dy = m_viewPort.y+m_viewPort.height/2;
    float slope = iclMin(m_viewPort.width,m_viewPort.height)/2;
    return Vec(pixel.x/slope-dx,pixel.y/slope-dy,m_F,1);
    
    /*
        Mat V = getViewPortMatrix(); V(2,2) = 1;
        return V.inv() * Vec(pixel.x,pixel.y,m_F,1); 
    */
  }


  Vec Camera::cameraToWorldFrame(const Vec &Xc) const{
    return getCoordinateSystemTransformationMatrix().inv()*Xc;
  }

}
