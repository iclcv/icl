#include <ICLGeom/CoplanarPointPoseEstimator.h>

#include <ICLGeom/Camera.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLUtils/Homography2D.h>

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

    // please note, the old implementation can be found in svn rev. 2753
    std::vector<Point32f> ips(n);//, pbs(n);
    for(int i=0;i<n;++i){
      ips[i] = Point32f(ifx*imagePoints[i].x+icx, ify * imagePoints[i].y+icy);
    }
  
    Homography2D H(ips.data(),modelPoints,n);
    
    // homography error, which is always 0
    //float err = 0;
    // for(int i=0;i<n;++i){
    //  err += H.apply(modelPoints[i]).distanceTo(ips[i]);
    //}
    //DEBUG_LOG("the error is " << err);

    H *= 1.0/sqrt( pow(H(0,0),2) + pow(H(0,1),2) + pow(H(0,2),2) ); 
    
    // if H solves Ax=0 then also -H solves it, therefore, we always
    // take the solution, where z is positive (object is in front of the camera)
    
    if(H(2,2) < 0){
      H *= -1;
    }

    FixedColVector<float,3> R1 = H.col(0);
    FixedColVector<float,3> R2 = H.col(1);
    R2 -= R1*(R1.transp()*R2)[0];  
    R2.normalize();
    FixedColVector<float,3> R3 = cross3(R1,R2);
    
    data->T.part<0,0,3,3>() = data->R = (R1,R2,R3);

    // -R * t -> provides translation part in 'clear-text'
    data->T.part<3,0,1,3>() = data->R.transp()*FixedColVector<float,3>( -H(2,0),-H(2,1),-H(2,2) ); 
    
    // this provides the original camera CS-Transformation Matrix
    data->T.part<3,0,1,3>() = FixedColVector<float,3>( H(2,0),H(2,1),H(2,2) );
    
    data->T(0,3) = data->T(1,3) = data->T(2,3) = 0;
    data->T(3,3) = 1;

#if 0
    Mat M = cam.getCSTransformationMatrix().inv()*data->T;
    float error = 0;
    for(int i=0;i<n;++i){
      Vec tmp = M * Vec(modelPoints[i].x,modelPoints[i].y,0,1);
      Point32f p = cam.project(tmp);
      error += p.distanceTo(imagePoints[i]);
    }

    Mat P = cam.getProjectionMatrix();    
    float error2 = 0;
    for(int i=0;i<n;++i){
      Vec tmp = homogenize( P * data->T * Vec(modelPoints[i].x,modelPoints[i].y,0,1) );
      error2 += Point32f(tmp[0],tmp[1]).distanceTo(imagePoints[i]);
    }
    std::cout << "error: " << error <<  "  error2:" << error2 << std::endl;

    /* explanation: 
                  ||                                     ||  where, I: imagePoints
       E(T) = sum || I[i] - project(C⁻1 * T * vec4(M[i]) ||         M: modelPoints
               i  ||                                     ||       C⁻1: inverse cam transform

        but since, project(x) p2(hom(P*C*x)), where P: projection matrix and C: camera matrix,
        and p2: extracts x and y from a 4D homogenious vector,
        
                  ||                                   ||  where,   P: cam projection matrix
       E(T) = sum || I[i] - p2(hom(P * T * vec4(M[i])) ||         hom: homogenization
               i  ||                                   ||          p2(x,y,z,w) = (x,y)
        
        The question, that remains is, how this function is minimized and, if we assume a 
        closed solution in least sqaure scene, if the initial T does already minimize E(T)
        if T is a homogeneous transform [R|t]

        * first approach: try to use stochastic search (perhaps coarse to fine) to find
          better solutions for T iteratively
        * 2nd approach: try to derivate E(T) wrt. T in order to implement gradient descent
          to optimize T
        * find out whether a closed form solution for E(T) is already found with the method
          above 
    */
#endif
    if(data->referenceFrame == cameraFrame){
      return data->T;
    }else{
      return cam.getCSTransformationMatrix().inv()*data->T;
    }
  }

}
