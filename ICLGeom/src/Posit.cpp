#include <ICLGeom/Posit.h>
#include <ICLUtils/StackTimer.h>
#include <ICLGeom/Camera.h>

namespace icl{
  
  struct Posit::Data{
    typedef DynMatrix<float> DMat;
    int N;             // number of model points
    int maxIterations; // termination criterion 1
    float minDelta;    // termination criterion 2
    std::vector<Vec> model; // model points
    DynMatrix<float> O;     // object matrix (pseudo inverse of model points)
    Posit::Result result;   // storage for pose result
    DMat I; // normalized image points
    DMat U; // normalized image coords (xs)
    DMat V; // normalized image coords (ys)
    DMat R1,R2,R3,R1normalized,R2normalized;
    DMat W;
    DMat M; // homogeneous model coordinates
  };
  
  Posit::Posit(int maxIterations, float minDelta):data(new Data){
    data->maxIterations = maxIterations;
    data->minDelta = minDelta;
    data->N = 0;
  }

  Posit::Posit(const std::vector<Vec> &modelPoints,int maxIterations, float minDelta):data(new Data){
    setModel(modelPoints);
    data->maxIterations = maxIterations;
    data->minDelta = minDelta;
  }
  
  Posit::~Posit(){
    delete data;
  }

  void Posit::setModel(const std::vector<Vec> &modelPoints){
    data->model = modelPoints;
    data->N = (int)modelPoints.size();
    const int N = data->N;
    
    data->M.setBounds(4,N);
   
    for(int i=0;i<N;++i){
      const Vec &v = modelPoints[i];
      data->M(0,i) = v[0];
      data->M(1,i) = v[1];
      data->M(2,i) = v[2];
      data->M(3,i) = 1;
    }
    data->O = data->M.pinv();

    data->I.setBounds(2,N);
    data->U.setBounds(1,N);
    data->V.setBounds(1,N);

    data->R1.setBounds(1,4);
    data->R2.setBounds(1,4);
    data->R3.setBounds(1,4);
  }

  const std::vector<Vec> &Posit::getModel() const{
    return data->model;
  }

  Posit::Posit(const Posit &other) : data(new Data){
    *this = other;
  }

  Posit &Posit::operator=(const Posit &other){
#define POSIT_COPY(X) data->X = other.data->X
    POSIT_COPY(N);
    POSIT_COPY(maxIterations);
    POSIT_COPY(minDelta);
    POSIT_COPY(model);
    POSIT_COPY(O);
    POSIT_COPY(result);
    POSIT_COPY(I);
    POSIT_COPY(U);
    POSIT_COPY(V);

    POSIT_COPY(R1);
    POSIT_COPY(R2);
    POSIT_COPY(R3);
    POSIT_COPY(R1normalized);
    POSIT_COPY(R2normalized);
    POSIT_COPY(W);
    POSIT_COPY(M);
    return *this;
  }

  void Posit::setMaxIterations(int maxIterations){
    data->maxIterations = maxIterations;
  }
  
  void Posit::setMinDelta(float minDelta){
    data->minDelta = minDelta;
  }
  
  int Posit::getMaxIterations() const{
    return data->maxIterations;
  }
    
  float Posit::getMinDelta() const{
    return data->minDelta;
  }

  FixedColVector<float,3> Posit::Result::getAngles() const{
    const Mat &M = *this;
    float angle_x(0), angle_y(0), angle_z(0); 
    angle_y = asin(M(0,2)); 
    /* http://en.wikipedia.org/wiki/Gimbal_lock ? */
    if ( fabs( cos( angle_y ) ) > 1.0e-6f ) {
      angle_x  = atan2(-M(1,2),M(2,2)); 
      angle_z  = atan2(-M(0,1),M(0,0)); 
    }else{
      angle_x  = 0;
      angle_z  = atan2(M(1,0),M(1,1)); 
    }
    
    return FixedColVector<float,3>(angle_x,angle_y,angle_z);
  }
  
  FixedColVector<float,3> Posit::Result::getTranslation() const{
    const Mat &m = *this;
    return FixedColVector<float,3>(m(3,0),m(3,1),m(3,2),1);
  }


  template<class T>
  static inline T norm3dyn(const DynMatrix<T> &m){
    return sqrt(m[0]*m[0] + m[1]*m[1] + m[2]*m[2]);
  }

  const Posit::Result &Posit::findPose(const std::vector<Point32f> &imagePoints, const Camera &cam) throw (ICLException){
    Point32f pp = cam.getPrincipalPointOffset();
    float fx = cam.getFocalLength()*cam.getSamplingResolutionX();
    float fy = cam.getFocalLength()*cam.getSamplingResolutionY();
    return findPose(imagePoints,pp,fx,fy);
  }

  const Posit::Result &Posit::findPose(const std::vector<Point32f> &imagePoints, const Point &pp,
                                       float fx, float fy) throw (ICLException){
    BENCHMARK_THIS_FUNCTION;
    typedef DynMatrix<float> DMat;
    typedef FixedColVector<float,3> Vec3;
    
    const int N = data->N;
    float ifx = 1.0/fx;
    float ify = 1.0/fy;
    
    for(int i=0;i<N;++i){
      data->U[i] = data->I(0,i) = (imagePoints[i].x - pp.x)*ifx;
      data->V[i] = data->I(1,i) = (imagePoints[i].y - pp.y)*ify;
    }
  
    float T[3]={0,0,0};
    Vec R[3]={Vec(0.0),Vec(0.0),Vec(0.0)};
    float a(0),b(0),s(0),u(0),v(0),du(0),dv(0);
    for(int i=0;i<data->maxIterations;++i){
      data->O.mult(data->U,data->R1);
      data->O.mult(data->V,data->R2);
    
      a = 1.0/norm3dyn(data->R1);
      b = 1.0/norm3dyn(data->R2);
    
      T[2] = ::sqrt(a*b); // alternatively: T[2] = 0.5*(a+b);

      data->R1.mult(T[2],data->R1normalized);
      data->R2.mult(T[2],data->R2normalized);

      R[0][0] = data->R1normalized[0];
      R[0][1] = data->R1normalized[1];
      R[0][2] = data->R1normalized[2];
      R[0][3] = data->R1normalized[3];

      R[1][0] = data->R2normalized[0];
      R[1][1] = data->R2normalized[1];
      R[1][2] = data->R2normalized[2];
      R[1][3] = data->R2normalized[3];
      
      R[2] = cross(R[0],R[1]);

      data->R3[0] = R[2][0];
      data->R3[1] = R[2][1];
      data->R3[2] = R[2][2];
      data->R3[3] = T[2];

      T[0] = data->R1normalized[3];
      T[1] = data->R2normalized[3];
      
      data->M.mult(data->R3,data->W);
      data->W *= 1./T[2];
      
      s=0;
      
      for(int j=0;j<N;++j){
        u = data->U[j];
        v = data->V[j];
        
        data->U[j] = data->W[j] * data->I(0,j);
        data->V[j] = data->W[j] * data->I(1,j);
        
        du = u-data->U[j];
        dv = v-data->V[j];
        
        s += du*du + dv*dv;
      }
    
      if(fx*fy*s < data->minDelta){
        //  std::cout << "reached error of "  << delta << " after " << (i+1) << " iterations (stop!)" << std::endl; 
        break;
      }
    }
  
    data->result(0,0) = R[0][0];
    data->result(1,0) = R[0][1];
    data->result(2,0) = R[0][2];
    data->result(3,0) = T[0];
    
    data->result(0,1) = R[1][0];
    data->result(1,1) = R[1][1];
    data->result(2,1) = R[1][2];
    data->result(3,1) = T[1];
    
    data->result(0,2) = R[2][0];
    data->result(1,2) = R[2][1];
    data->result(2,2) = R[2][2];
    data->result(3,2) = T[2];

    data->result(0,3) = data->result(1,3) = data->result(2,3) = 0;
    data->result(3,3) = 1;

    return data->result;
  }
}

