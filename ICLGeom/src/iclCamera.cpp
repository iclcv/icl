#include "iclCamera.h"
#include <math.h>

namespace icl{
  
  Camera::Camera(const Vec &pos,
                 const Vec &norm,
                 const Vec &up,
                 float f,
                 float zNear,
                 float zFar):
    m_oPos(pos),m_oNorm(norm),m_oUp(up),m_fZNear(zNear),m_fZFar(zFar){
    m_fF = f>0 ? f : cos((-f/2)*M_PI/180)/sin((-f/2)*M_PI/180);
  }

  Mat Camera::getCoordinateSystemTransformationMatrix() const{
    // Transformation matrix ** T **
    /* [ --- hh --- | -hh.p ]
       [ --- uu --- | -uu.p ]
       [ --- nn --- | -nn.p ]
       [--------------------]
       [ 0   0   0  |   1   ]
    */
    Vec nn = normalize(m_oNorm);
    Vec ut = normalize(m_oUp);
    Vec hh = cross(nn,ut);
    Vec uu = cross(hh,nn);

    Mat T;
    T.col(0) = hh;
    T.col(1) = uu;
    T.col(2) = -nn;
    T.col(3) = Vec(0.0);
    T = T.transp();
    //    T[3] = Vec(0,0,0,1);
    T.col(3) =-(T*m_oPos);
    T(3,3) = 1;

    return T;
  }
  
  Mat Camera::getProjectionMatrix() const{
    float A = (m_fZFar + m_fZNear)/(m_fZNear - m_fZFar);
    float B = (2*m_fZFar*m_fZNear)/(m_fZNear - m_fZFar);
    
    return  Mat ( m_fF , 0   ,   0,  0,
                  0    , m_fF,   0,  0,
                  0    , 0   ,   A,  B,
                  0    , 0   ,  -1,  0 );
  }


  const Mat &Camera::getTransformationMatrix() const {
    Mat T = getCoordinateSystemTransformationMatrix();
    Mat P = getProjectionMatrix();
    
    m_oMatBuf=P*T;    
    return m_oMatBuf;
  }

  void Camera::show(const std::string &title) const{
    printf("cam: %s \n",title.c_str());
    std::cout << "norm:\n" << m_oNorm << "\nup:\n" << m_oUp << "\npos:\n" << m_oPos << "\nf: " << m_fF << std::endl;
  }
  

}
