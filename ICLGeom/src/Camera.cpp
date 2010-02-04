#include <ICLUtils/ConfigFile.h>
#include <ICLUtils/XMLDocument.h>
#include <ICLUtils/DynMatrixUtils.h>
#include <ICLUtils/FixedMatrixUtils.h>
#include <ICLGeom/Camera.h>
#include <fstream>

namespace icl {

  void Camera::setRotation(const Vec &rot) {
    // see http://en.wikipedia.org/wiki/Rotation_matrix#Dimension_three
    float sin_a = sin(rot[0]); float cos_a = cos(rot[0]);
    Mat3x3 R_x(1, 0, 0,
            0, cos_a, -sin_a,
            0, sin_a, cos_a);
    float sin_b = sin(rot[1]); float cos_b = cos(rot[1]);
    Mat3x3 R_y(cos_b, 0, sin_b,
            0, 1, 0,
            -sin_b, 0, cos_b);
    float sin_c = sin(rot[2]); float cos_c = cos(rot[2]);
    Mat3x3 R_z(cos_c, -sin_c, 0,
            sin_c,  cos_c, 0,
            0, 0, 1);
    setRotation(R_x*R_y*R_z);
  }
  
  Mat Camera::createTransformationMatrix(const Vec &norm, const Vec &up, const Vec &pos) {
    // see http://en.wikipedia.org/wiki/Cross_product#Cross_product_and_handedness
    /* [ --- hh --- | -hh.p ]
       [ --- up --- | -up.p ]
       [ -- norm -- | -no.p ]
       [ 0   0   0  |   1   ]
    */
    // double cross product to ensure that they are all orthogonal
    Vec hh = cross(up,norm); 
    Vec uu = cross(norm,hh);

    Mat T;
    T.row(0) = hh;
    T.row(1) = uu;
    T.row(2) = norm;
    T.row(3) = Vec(0.0);
    T.col(3) = Vec(0.0);
    
    T.col(3) = -(T*pos);
    T(3,3) = 1;
    
    return T;
  }
    
  Mat Camera::getCSTransformationMatrix() const {
    return createTransformationMatrix(m_norm, m_up, m_pos);
  }
  
  Mat Camera::getCSTransformationMatrixGL() const {
    // for OpenGL, the camera looks in negative z direction
    return createTransformationMatrix(-m_norm, m_up, m_pos);
  }

  Mat Camera::getProjectionMatrix() const {
    return Mat(m_f * m_mx,   m_skew, m_px, 0,
                        0, m_f*m_my, m_py, 0,
                        0,        0, 0,    0,
                        0,        0, 1,    0);
  }

  Mat Camera::getProjectionMatrixGL() const {
    float clip_near = m_f;
    float A = (m_renderParams.clipZFar + clip_near)/(clip_near - m_renderParams.clipZFar);
    float B = (2*m_renderParams.clipZFar*clip_near)/(clip_near - m_renderParams.clipZFar);
    float w2 = m_renderParams.chipSize.width/2;
    float h2 = m_renderParams.chipSize.height/2;
    // Because OpenGL will automatically flip the y-coordinates in the end,
    // we need switch the sign of the skew and py components of the matrix.
    // Also we need switch the sign back when doing the projection by hand.
    return Mat( m_f*m_mx/w2,     -m_skew/w2,   (m_px-w2)/w2,  0,
                          0,     m_f*m_my/h2, -(m_py-h2)/h2,  0,
                          0,               0,             A,  B,
                          0,               0,            -1,  0);
  }

  Mat Camera::getViewportMatrixGL() const {
    float w2 = m_renderParams.viewport.width/2;
    float h2 = m_renderParams.viewport.height/2;
    const float &zFar = m_renderParams.viewportZMax;
    const float &zNear = m_renderParams.viewportZMin;
    return Mat(w2,  0, 0, w2 + m_renderParams.viewport.x,
                0, h2, 0, h2 + m_renderParams.viewport.y,
                0,  0, (zFar-zNear)/2, (zFar+zNear)/2,
                0,  0, 0,    1);
  }
  
  // Projects a world point to the screen
  Point32f Camera::project(const Vec &Xw) const {
    Mat T = getCSTransformationMatrix();
    Mat P = getProjectionMatrix();
    
    Vec xi = homogenize(P*T*Xw);

    return Point32f(xi[0],xi[1]);
  }
  
  // Projects a set of points
  void Camera::project(const std::vector<Vec> &Xws, std::vector<Point32f> &dst) const{
    dst.resize(Xws.size());
    Mat T = getCSTransformationMatrix();
    Mat P = getProjectionMatrix();
    Mat M = P*T;
    for(unsigned int i=0;i<Xws.size();++i){
      Vec xi = homogenize(M*Xws[i]);
      dst[i].x = xi[0];
      dst[i].y = xi[1];
    }
  }

  // Projects a set of points
  const std::vector<Point32f> Camera::project(const std::vector<Vec> &Xws) const{
    std::vector<Point32f> xis;
    project(Xws,xis);
    return xis;
  }

  /// Project a world point onto the image plane.
  Vec Camera::projectGL(const Vec &Xw) const {
    Mat T = getCSTransformationMatrix();
    Mat P = getProjectionMatrixGL();
    // correct the sign of skew and y-offset component
    P(1,0) *= -1; P(2,1) *= -1;
    Mat V = getViewportMatrixGL();
    return homogenize(V*P*T*Xw);
  }

  /// Project a vector of world points onto the image plane.
  void Camera::projectGL(const std::vector<Vec> &Xws, std::vector<Vec> &dst) const {
    dst.resize(Xws.size());
    Mat T = getCSTransformationMatrix();
    Mat P = getProjectionMatrixGL();
    // correct the sign of skew and y-offset component
    P(1,0) *= -1; P(2,1) *= -1;
    Mat V = getViewportMatrixGL();
    Mat M = V*P*T;
    for(unsigned int i=0;i<Xws.size();++i) {
      dst[i] = homogenize(M*Xws[i]);
    }
  }

  /// Project a vector of world points onto the image plane.
  const std::vector<Vec> Camera::projectGL(const std::vector<Vec> &Xws) const {
    std::vector<Vec> dst;
    projectGL(Xws, dst);
    return dst;
  }
  
  void Camera::setRotation(const Mat3x3 &rot) {
    m_norm = Vec(rot(0,2), rot(1,2), rot(2,2), 0).normalized();
    m_norm[3] = 1;
    m_up = Vec(rot(0,1), rot(1,1), rot(2,1), 0).normalized();
    m_up[3] = 1;
    // we need to check the handedness
    Vec v = cross(m_up,m_norm);
    // get the component with biggest abs. value of v
    int idx = 0;
    if (abs(v[1]) > abs(v[idx])) idx = 1;
    if (abs(v[2]) > abs(v[idx])) idx = 2;
    // check if the signs are same
    if (v[idx] * rot(idx,0) < 0) {
      // not the same sign -- switch directions to make right handed cs
      m_norm *= -1; m_norm[3] = 1;
      m_up *= -1; m_up[3] = 1;
    }
  }
  
  Camera Camera::createFromProjectionMatrix(const FixedMatrix<icl32f,4,3> &Q,
                                                float focalLength) {
    FixedMatrix<float,3,3> M = Q.part<0,0,3,3>();
    FixedMatrix<float,1,3> c4 = Q.col(3);
    
    FixedMatrix<float,3,3> K; // intrinsic parameters
    FixedMatrix<float,3,3> R; // extrinsic (rotation matrix)
    FixedMatrix<float,1,3> T; // extrinsic (tranlation vector)
    
    decompose_RQ(M,K,R);
    K = K/K(2,2); // normalize K
    T = -M.inv() * c4;
    Camera cam;
    cam.setPosition(Vec(T[0],T[1],T[2],1));
    cam.setRotation(R);
    cam.setFocalLength(focalLength);
    cam.setSamplingResolution(K(0,0)/focalLength,K(1,1)/focalLength);
    cam.setPrincipalPointOffset(Point32f(K(2,0),K(2,1)));
    cam.setSkew(K(1,0));
    return cam;
  }
  
  Camera Camera::calibrate_pinv(std::vector<Vec> Xws,
                                    std::vector<Point32f> xis,
                                    float focalLength)
         throw (NotEnoughDataPointsException) {
    // TODO: normalize points
    checkAndFixPoints(Xws,xis);

    int N = (int)Xws.size();
    
    DynMatrix<float> U(1,2*N);
    for(int i=0;i<N;++i){
      U[2*i] = xis[i].x;
      U[2*i+1] = xis[i].y;
    }
    
    DynMatrix<float> B(11,2*N);
    for(int i=0;i<N;++i){
      float x=Xws[i][0], y=Xws[i][1],z=Xws[i][2], u=-xis[i].x,v=-xis[i].y;
      float r1[11] = {x,y,z,1,0,0,0,0,u*x,u*y,u*z};
      float r2[11] = {0,0,0,0,x,y,z,1,v*x,v*y,v*z};
      
      std::copy(r1,r1+11,B.row_begin(2*i));
      std::copy(r2,r2+11,B.row_begin(2*i+1));
    }

    DynMatrix<float> Cv = B.pinv() * U;
    FixedMatrix<float,4,3> Q(Cv[0],Cv[1],Cv[2],Cv[3],
                             Cv[4],Cv[5],Cv[6],Cv[7],
                             Cv[8],Cv[9],Cv[10],1);

    return Camera::createFromProjectionMatrix(Q, focalLength);
  }
  
  Camera Camera::calibrate(std::vector<Vec> Xws,
                               std::vector<Point32f> xis,
                               float focalLength)
         throw (NotEnoughDataPointsException) {
         
#ifndef HAVE_MKL
	return calibrate_pinv(Xws,xis,focalLength);
#else
    // TODO: normalize points
    // TODO: check whether we have svd (IPP) available
    checkAndFixPoints(Xws,xis);
    
    unsigned int n = Xws.size();
    DynMatrix<icl32f> A(12,2*n);
    
    for (unsigned int k=0; k<n; ++k) {
      int i = 2*k;
      float x = xis[k].x; float y = xis[k].y;
      float X = Xws[k][0]; float Y = Xws[k][1]; float Z = Xws[k][2]; float W = Xws[k][3];
      A(0,i) = 0; A(1,i) = 0; A(2,i) = 0; A(3,i) = 0;
      A(4,i) = -X; A(5,i) = -Y; A(6,i) = -Z; A(7,i) = -W;
      A(8,i) = y*X; A(9,i) = y*Y; A(10,i) = y*Z; A(11,i) = y*W;
      i = 2*k+1;
      A(0,i) = X; A(1,i) = Y; A(2,i) = Z; A(3,i) = W;
      A(4,i) = 0; A(5,i) = 0; A(6,i) = 0; A(7,i) = 0;
      A(8,i) = -x*X; A(9,i) = -x*Y; A(10,i) = -x*Z; A(11,i) = -x*W;
    }
    
    DynMatrix<icl32f> U,s,V;
    svd_dyn(A,U,s,V);
   
    FixedMatrix<float,4,3> Q;
    for (int i=0; i<4; i++) for (int j=0; j<3; j++) {
      Q(i,j) = V(11,j*4+i);
    }
    return Camera::createFromProjectionMatrix(Q, focalLength);       
#endif
  }
  
  void Camera::checkAndFixPoints(std::vector<Vec> &Xws, std::vector<Point32f> &xis) throw (NotEnoughDataPointsException) {
    if(Xws.size() > xis.size()){
      ERROR_LOG("got more world points than image points (erasing additional world points)");
      Xws.resize(xis.size());
    } else if(Xws.size() < xis.size()){
      ERROR_LOG("got less world points than image points (erasing additional image points)");
      xis.resize(Xws.size());
    }
    if(Xws.size() < 6){
      throw NotEnoughDataPointsException();
    }
    for(unsigned int i=0;i<Xws.size();++i){
      Xws[i][3]=1;
    }
  }
  
  void Camera::load_camera_from_stream(std::istream &is, const std::string &prefix,
                                         Camera &cam){
    cam = Camera(); // load default values
    ConfigFile f(new XMLDocument(is));
    f.setPrefix(prefix);

    #define LOAD_FROM_STREAM(KEY,FNAME) \
    if (f.contains("camera." #KEY)) cam.set##FNAME(f["camera." #KEY]); \
    else WARNING_LOG("No " #KEY " found in configuration (using default: '" << cam.get##FNAME() << "')");
    LOAD_FROM_STREAM(name,Name);
    LOAD_FROM_STREAM(position,Position);
    LOAD_FROM_STREAM(norm,Norm);
    LOAD_FROM_STREAM(up,Up);
    LOAD_FROM_STREAM(f,FocalLength);
    LOAD_FROM_STREAM(principal-point-offset,PrincipalPointOffset);
    LOAD_FROM_STREAM(sampling-resolution-x,SamplingResolutionX);
    LOAD_FROM_STREAM(sampling-resolution-y,SamplingResolutionY);
    LOAD_FROM_STREAM(skew,Skew);
    #undef LOAD_FROM_STREAM
    
    f.setPrefix(prefix+"camera.render-params.");
    #define LOAD_FROM_STREAM(KEY,ATTR) \
    if (f.contains(#KEY)) cam.getRenderParams().ATTR = f[#KEY]; \
    else WARNING_LOG("No " #KEY " found in configuration (using default: '" << cam.getRenderParams().ATTR << "')");
    LOAD_FROM_STREAM(chip-size, chipSize);
    LOAD_FROM_STREAM(clip-z-far, clipZFar);
    LOAD_FROM_STREAM(viewport, viewport);
    LOAD_FROM_STREAM(viewport-z-min, viewportZMin);
    LOAD_FROM_STREAM(viewport-z-max, viewportZMax);
    #undef LOAD_FROM_STREAM
  }
 
  std::string Camera::toString() const {
    std::ostringstream os;
    os << "Position: " << getPosition().transp() << std::endl;
    os << "Norm: " << getNorm().transp() << std::endl;
    os << "Up: " << getUp().transp() << std::endl;
    os << "Focal Length: " << getFocalLength() << std::endl;
    os << "Principal Point Offset: " << getPrincipalPointOffset() << std::endl;
    os << "Sampling Resolution: " << getSamplingResolutionX() << ", "
                                  << getSamplingResolutionY() << std::endl;
    os << "Skew: " << getSkew() << std::endl;
    return os.str();
  }
  
  /// ostream operator
  std::ostream &operator<<(std::ostream &os, const Camera &cam){
    ConfigFile f;
    f.setPrefix("config.camera.");

    #define WRITE_TO_STREAM(KEY,FNAME) \
    f[#KEY] = cam.get##FNAME();
    WRITE_TO_STREAM(name,Name);
    WRITE_TO_STREAM(position,Position);
    WRITE_TO_STREAM(norm,Norm);
    WRITE_TO_STREAM(up,Up);
    WRITE_TO_STREAM(f,FocalLength);
    WRITE_TO_STREAM(principal-point-offset,PrincipalPointOffset);
    WRITE_TO_STREAM(sampling-resolution-x,SamplingResolutionX);
    WRITE_TO_STREAM(sampling-resolution-y,SamplingResolutionY);
    WRITE_TO_STREAM(skew,Skew);
    #undef WRITE_TO_STREAM

    f["render-params.chip-size"] = cam.getRenderParams().chipSize;
    f["render-params.clip-z-far"] = cam.getRenderParams().clipZFar;
    f["render-params.viewport"] = cam.getRenderParams().viewport;
    f["render-params.viewport-z-min"] = cam.getRenderParams().viewportZMin;
    f["render-params.viewport-z-max"] = cam.getRenderParams().viewportZMax;
    
    return os << f;
  }

  /// istream operator parses a camera from an XML-string
  std::istream &operator>>(std::istream &is, Camera &cam) throw (ParseException) {
    cam = Camera(is,"config.");
    return is; 
  }
  
  Camera::Camera(const std::string &filename, const std::string &prefix) throw (ParseException){
    std::ifstream is(filename.c_str());
    load_camera_from_stream(is,prefix,*this);
  }

  Camera::Camera(std::istream &is, const std::string &prefix) throw (ParseException){
    load_camera_from_stream(is,prefix,*this);
  }
  
  ViewRay Camera::getViewRay(const Point32f &pixel) const {
    Mat T = getCSTransformationMatrix();
    Mat P = getProjectionMatrix();
    Mat M = P*T;
    FixedMatrix<icl32f,4,3> Q;
    Q.row(0) = M.row(0); Q.row(1) = M.row(1); Q.row(2) = M.row(3);
    Vec dir = m_pos-homogenize(pinv(Q)*FixedColVector<icl32f,3>(pixel.x, pixel.y, 1));
    dir[3] = 0; dir.normalize(); dir[3] = 1;
    return ViewRay(m_pos,dir);
  }
  
  ViewRay Camera::getViewRay(const Vec &Xw) const{
    return ViewRay(m_pos, Xw-m_pos);
  }
  
  static inline float sprod_3(const Vec &a, const Vec &b){
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
  }
  
  Vec Camera::getIntersection(const ViewRay &v, const PlaneEquation &plane) throw (ICLException) {
    float denom = sprod_3(v.direction, plane.normal);
    if(!denom) throw ICLException("no intersection -> plane normal is perdendicular to view-ray direction");
    float lambda = - sprod_3(v.offset-plane.offset,plane.normal) / denom;
    return v(lambda);
  }
  
  Vec Camera::estimate3DPosition(const Point32f &pixel, const PlaneEquation &plane) const throw (ICLException) {
    return getIntersection(getViewRay(pixel),plane);
  }
}
