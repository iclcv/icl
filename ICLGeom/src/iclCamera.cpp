#include "iclCamera.h"

namespace icl{
  const Mat &Camera::getTransformationMatrix(){
    // Transformation matrix ** T **
    /* [ --- hh --- | -hh.p ]
       [ --- uu --- | -uu.p ]
       [ --- nn --- | -nn.p ]
       [--------------------]
       [ 0   0   0  |   1   ]
    */
    Vec nn = -m_oNorm.normalized();
    Vec ut = m_oUp.normalized();
    Vec hh = nn.cross(ut);
    Vec uu = hh.cross(nn);
    
    Mat T = Mat( hh, uu, nn, 0).transposed();
    T[3] = Vec(0,0,0,1);
    T[3] =-(T*m_oPos);
    T[3][3] = 1;


    // Projection matrix ** P **
    Mat P = Mat ( 1 , 0,     0,   0,
                  0 , 1,     0,   0,
                  0 , 0,     0,   0,
                  0 , 0,  -1/m_fF,   0 );
    // identical to 
    // P = Mat ( f , 0,   0,  0,
    //           0 , f,   0,  0,
    //           0 , 0,   0,  0,
    //           0 , 0,  -1,  0 );

                 
    
    m_oMatBuf=P*T;    
    return m_oMatBuf;
  }

  void Camera::show(const std::string &title) const{
    printf("cam: %s \n",title.c_str());
    Mat(m_oNorm,m_oUp,m_oPos,m_fF).show("norm,up,pos,f");
  }
  

}
