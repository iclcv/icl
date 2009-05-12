#include "iclCamera.h"
#include <math.h>

namespace icl{


  Camera::Camera(const Vec &pos, const Vec &rot, const Size &viewPortSize,
                 float f, float zNear, float zFar, bool rightHandedCS){

    FixedMatrix <double,2,4> nu( 0,0,
                                 0,1,
                                 1,0,
                                 1,1 );
    nu = create_hom_4x4<double>(rot[0],rot[1],rot[2])*nu;
    Vec norm = nu.col(0); norm[3] = 0;
    Vec up = nu.col(1); up[3] = 0;
    init(pos,norm,up,Rect(Point::null,viewPortSize),f,zNear,zFar,rightHandedCS);
  }
  
  
  void Camera::init(const Vec &pos,
                    const Vec &norm,
                    const Vec &up,
                    const Rect &viewPort,
                    float f,
                    float zNear,
                    float zFar,
                    bool rightHandedCS){
    m_pos = pos;
    m_norm = normalize3(norm,0);
    m_up = normalize3(up,0);
    m_zNear = zNear;
    m_zFar = zFar;
    m_F = f>0 ? f : cos((-f/2)*M_PI/180)/sin((-f/2)*M_PI/180);
    m_viewPort = viewPort;
    
    m_rightHandedCS = rightHandedCS;

    /*
    DEBUG_LOG("init Camera ...");
    #define X(Y) DEBUG_LOG(#Y << " : " << Y);
    X(m_pos);    X(m_norm); X(m_up); X(m_zNear); X(m_zFar); X(m_F); X(m_viewPort); X(m_rightHandedCS);
    */
  }
  
  Mat Camera::getCoordinateSystemTransformationMatrix() const{
    // Transformation matrix ** T **
    /* [ --- hh --- | -hh.p ]
       [ --- uu --- | -uu.p ]
       [ --- nn --- | -nn.p ]
       [--------------------]
       [ 0   0   0  |   1   ]
    */
    const Vec &nn = m_norm;
    const Vec &ut = m_up;

    //    Vec hh = cross(nn,ut);
    Vec hh = cross(ut,nn);
    Vec uu;

    if(m_rightHandedCS){
      // see http://en.wikipedia.org/wiki/Cross_product#Cross_product_and_handedness
      uu = cross(nn,hh); // now its right handed!
    }else{
      uu = cross(hh,nn); // this is for left handed coordinate systems:
    }


    Mat T;
    T.col(0) = hh;
    T.col(1) = uu;
    //T.col(2) = -nn; // WHY? -> because we used hh = cross(nn,ut) before ...
    T.col(2) = nn; 
    T.col(3) = Vec(0.0);
    T = T.transp();
    //    ------------------>   very old, i guess, T[3] = Vec(0,0,0,1);
    
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

    /* THIS IS CORRECT OLD VERSION ...
    return  Mat ( slope , 0     , 0 , dx,
                  0     , slope , 0 , dy,
                  0     , 0     , 0 , 0 ,
                  0     , 0     , 0 , 1 );
   */
    return  Mat ( slope , 0     , 0 , dx,
                  0     , slope , 0 , dy,
                  0     , 0     , 1 , 0 ,
                  0     , 0     , 0 , 1 );

  }

  float Camera::getViewPortAspectRatio() const{
    return float(m_viewPort.width)/m_viewPort.height;
  }

  Rect32f Camera::getNormalizedViewPort() const{
    float ar = getViewPortAspectRatio();
    if(ar > 1){
      return Rect32f(-ar,-1,2*ar,2);
    }else{
      return Rect32f(-1,-ar,2,2*ar);
    }
  }
  
  Vec Camera::screenToCameraFrame(const Point32f &pixel) const{
    // Todo: optimize this code by pre-calculate inverse matrices ...
    Mat V = getViewPortMatrix(); // V(2,2) = 1; this is no longer needed!
    Mat P = getProjectionMatrix();
    return homogenize(P.inv()*homogenize(V.inv() * Vec(pixel.x,pixel.y,m_F,1)));
  }


  Vec Camera::cameraToWorldFrame(const Vec &Xc) const{
    return getCoordinateSystemTransformationMatrix().inv()*Xc;
  }


  Vec Camera::screenToWorldFrame(const Point32f &pixel) const{
    return cameraToWorldFrame(screenToCameraFrame(pixel));
  }
  
  Camera::ViewRay Camera::getViewRay(const Point32f &pixel) const{
    return ViewRay(m_pos, screenToWorldFrame(pixel)-m_pos);
  }
  
  Camera::ViewRay Camera::getViewRay(const Vec &Xw) const{
    return ViewRay(m_pos, Xw-m_pos);
  }
    
    /// Projects a world point to the screen
  Point32f Camera::project(const Vec &Xw) const{
    Mat C = getCoordinateSystemTransformationMatrix();
    Mat P = getProjectionMatrix();
    Mat V = getViewPortMatrix();
    
    Vec vP = homogenize(V*P*C*Xw);

    return Point32f(vP[0],vP[1]);
  }

  Point32f Camera::projectToNormalizedViewPort(const Vec &v) const{
    Mat C = getCoordinateSystemTransformationMatrix();
    Mat P = getProjectionMatrix();
    
    Vec vP = homogenize(P*C*v);
    
    return Point32f(-vP[0],-vP[1]); // (-) ? 
  }


  FixedMatrix<float,4,2> Camera::get4Dto2DMatrix() const{
    Mat C = getCoordinateSystemTransformationMatrix();
    Mat P = getProjectionMatrix();
    Mat V = getViewPortMatrix();
    
    return FixedMatrix<float,4,2> ((V*P*C).begin());
  }
 
  /// Projects a set of points (just an optimization)
  void Camera::project(const std::vector<Vec> &Xws, std::vector<Point32f> &dst) const{
    dst.resize(Xws.size());
    Mat C = getCoordinateSystemTransformationMatrix();
    Mat P = getProjectionMatrix();
    Mat V = getViewPortMatrix();
    Mat M = V*P*C;
    for(unsigned int i=0;i<Xws.size();++i){
      Vec vP = homogenize(M*Xws[i]);
      dst[i].x = vP[0];
      dst[i].y = vP[1];
    }
  }

  /// Projects a set of points (results are x,y,z,1)
  void Camera::project(const std::vector<Vec> &Xws, std::vector<Vec> &dstXYZ) const{
    dstXYZ.resize(Xws.size());
    Mat C = getCoordinateSystemTransformationMatrix();
    Mat P = getProjectionMatrix();
    Mat V = getViewPortMatrix();
    Mat M = V*P*C;
    for(unsigned int i=0;i<Xws.size();++i){
      dstXYZ[i] = homogenize(M*Xws[i]);
    }
  }


  const std::vector<Point32f> Camera::project(const std::vector<Vec> &Xws) const{
    std::vector<Point32f> v;
    project(Xws,v);
    return v;
  }

  Vec Camera::getHorz() const {
    Vec nn = m_norm;
    Vec ut = m_up;
    if(m_rightHandedCS){
       return cross(ut,nn);
    }else{
      return cross(nn,ut);
    }
  }
  

}
