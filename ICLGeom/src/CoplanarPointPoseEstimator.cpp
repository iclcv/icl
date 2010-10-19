#include <ICLGeom/CoplanarPointPoseEstimator.h>

#include <ICLGeom/Camera.h>
#include <ICLGeom/GeomDefs.h>

namespace icl{
  typedef DynMatrix<float> DMat;
  
  static inline void assign_row(float* p, float a, float b, float c, float d, 
                                float e, float f, float g, float h, float i){
    p[0] = a; p[1] = b; p[2] = c; p[3] = d; p[4] = e; 
    p[5] = f; p[6] = g; p[7] = h; p[8] = i;
  }

  static inline FixedMatrix<float,1,3> cross3(const FixedMatrix<float,1,3> &v1,
                                              const FixedMatrix<float,1,3> &v2){
    return FixedMatrix<float,1,3>(v1[1]*v2[2]-v1[2]*v2[1],
                                  v1[2]*v2[0]-v1[0]*v2[2],
                                  v1[0]*v2[1]-v1[1]*v2[0]);
  }
  
  struct CoplanarPointPoseEstimator::Data{
    DMat A,U,s,V;

    FixedMatrix<float,3,3> H,R;
    FixedMatrix<float,1,3> C;
    FixedMatrix<float,4,4> T;
    
    CoplanarPointPoseEstimator::ReferenceFrame referenceFrame;
  };
  
  
  CoplanarPointPoseEstimator::CoplanarPointPoseEstimator(ReferenceFrame returnedPoseReferenceFrame):
    data(new Data){
    data->referenceFrame = returnedPoseReferenceFrame;
  }
  
  CoplanarPointPoseEstimator::~CoplanarPointPoseEstimator(){
    delete data;
  }

  CoplanarPointPoseEstimator::CoplanarPointPoseEstimator(const CoplanarPointPoseEstimator &other):
    data(new Data){
    *this = other;
  }



  CoplanarPointPoseEstimator &CoplanarPointPoseEstimator::operator=(const CoplanarPointPoseEstimator &other){
    *data = *other.data;
    return *this;
  }


  CoplanarPointPoseEstimator::ReferenceFrame CoplanarPointPoseEstimator::getReferenceFrame() const{
    return data->referenceFrame;;
  }
  
  void CoplanarPointPoseEstimator::setReferenceFrame(CoplanarPointPoseEstimator::ReferenceFrame f){
    data->referenceFrame = f;
  }

    
  Mat CoplanarPointPoseEstimator::getPose(int n, 
                                          const Point32f *modelPoints, 
                                          const Point32f *imagePoints, 
                                          const Camera &cam){
    float ifx = 1.0f/(cam.getFocalLength()*cam.getSamplingResolutionX());
    float ify = 1.0f/(cam.getFocalLength()*cam.getSamplingResolutionY());
    float icx = -ifx * cam.getPrincipalPointOffset().x;
    float icy = -ify * cam.getPrincipalPointOffset().y;

    DMat &A = data->A;
    A.setBounds(9,2*n);
  
    for(int i=0;i<n;++i){
      float px = ifx*imagePoints[i].x+icx, py = ify * imagePoints[i].y+icy;
      float qx = modelPoints[i].x, qy = modelPoints[i].y;
      assign_row(A.row_begin(2*i+0),0,0,0,-qx,-qy,-1,py*qx,py*qy,py);
      assign_row(A.row_begin(2*i+1),qx,qy,1,0,0,0,-px*qx,-px*qy,-px);
    }
  
    A.svd(data->U,data->s,data->V);

    FixedMatrix<float,3,3> &H = data->H;
    std::copy(data->V.col_begin(8),data->V.col_end(8),H.begin());

    //SHOW(data->s[8]);
    DMat h(1,9,H.begin(),true);
    //SHOW(A*h);

    H *= 1.0/sqrt( pow(H(0,0),2) + pow(H(0,1),2) + pow(H(0,2),2) ); 
    
    if(H(2,2) < 0){
      H *= -1;
    }

    FixedColVector<float,3> R1 = H.col(0);
    FixedColVector<float,3> R2 = H.col(1);
    R2 -= R1*(R1.transp()*R2)[0];  
    R2.normalize();
    FixedColVector<float,3> R3 = cross3(R1,R2);
    //R2 = cross3(R1,R3);


    /*
        SHOW(R1.length())
        SHOW(R2.length())
        SHOW(R3.length())
        
        SHOW(R1.transp() * R2);
        SHOW(R2.transp() * R3);
        SHOW(R1.transp() * R3);
    */
    
    data->T.part<0,0,3,3>() = data->R = (R1,R2,R3);

    // -R * t -> provides translation part in 'clear-text'
    data->T.part<3,0,1,3>() = data->R.transp()*FixedColVector<float,3>( -H(2,0),-H(2,1),-H(2,2) ); 
    
    // this provides the original camera CS-Transformation Matrix
    data->T.part<3,0,1,3>() = FixedColVector<float,3>( H(2,0),H(2,1),H(2,2) );

    // if h solves Ax=0 then also -h solves it, therefore, we always
    // take the solution, where z is positive (object is in front of the camera)
    
    data->T(0,3) = data->T(1,3) = data->T(2,3) = 0;
    data->T(3,3) = 1;
    
    
    if(data->referenceFrame == cameraFrame){
      return data->T;
    }else{
      return cam.getCSTransformationMatrix().inv()*data->T;
    }
  }

}
