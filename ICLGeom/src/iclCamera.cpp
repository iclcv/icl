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
    Vec nn = -norm.normalized();
    Vec ut = up.normalized();
    Vec hh = nn.cross(ut);
    Vec uu = hh.cross(nn);
    
    Mat T = Mat( hh, uu, nn, 0).transposed();
    T[3] = Vec(0,0,0,1);
    T[3] =-(T*pos);
    M[3][3] = 1;


    // Projection matrix ** P **
    Mat P = Mat ( 1 , 0,     0,   0,
                  0 , 1,     0,   0,
                  0 , 0,     0,   0,
                  0 , 0,  -1/f,   0 );
    // identical to 
    // P = Mat ( f , 0,   0,  0,
    //           0 , f,   0,  0,
    //           0 , 0,   0,  0,
    //           0 , 0,  -1,  0 );

    // Viewport matrix ** V **
    float dx = imageSize.width/2;
    float dy = imageSize.height/2;
    float slope = std::min(dx,dy);
    Mat V = Mat ( slope , 0     , 0 , dx,
                  0     , slope , 0 , dy,
                  0     , 0     , 0 , 0 ,
                  0     , 0     , 0 , 1 );
                 
    
    M=V*P*T;    
    return M;
  }

  void Camera::show(const std::string &title) const{
    printf("cam: %s \n",title.c_str());
    Mat(norm,up,pos,f).show("norm,up,pos,f");
  }
  

}
