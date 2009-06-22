#include <iclExtrinsicCameraCalibrator.h>
#include <iclFixedMatrixUtils.h>
#include <iclDynMatrix.h>

#include <iclStochasticOptimizer.h>
#include <iclRandom.h>

namespace icl{
  
#if 0
  // OLD style .. will be removed soon
  ExtrinsicCameraCalibrator::ExtrinsicCameraCalibrator(const std::string method, float *params):
    m_method(method){
    if(method != "linear" && method != "linear+stochastic"){
      ERROR_LOG("wrong method ...");
      m_method = "linear";
      return;
    }
    if(method == "linear+stochastic"){
      m_params.push_back(params ? params[0] : 10000);
      m_params.push_back(params ? params[1] : 0.001);
    }
  }
#endif

  static FixedMatrix<float,4,3> compute_Q_Matrix(const std::vector<Vec> &XWs, 
                                                 const std::vector<Point32f> &XIs){
    ICLASSERT_RETURN_VAL(XWs.size() == XIs.size(), (FixedMatrix<float,4,3>(0.0)) );
    int N = (int)XWs.size();
    
    DynMatrix<float> U(1,2*N);
    for(int i=0;i<N;++i){
      U[2*i] = XIs[i].x;
      U[2*i+1] = XIs[i].y;
    }
    
    DynMatrix<float> B(11,2*N);
    for(int i=0;i<N;++i){
      float x=XWs[i][0], y=XWs[i][1],z=XWs[i][2], u=-XIs[i].x,v=-XIs[i].y;
      float r1[11] = {x,y,z,1,0,0,0,0,u*x,u*y,u*z};
      float r2[11] = {0,0,0,0,x,y,z,1,v*x,v*y,v*z};
      
      std::copy(r1,r1+11,B.row_begin(2*i));
      std::copy(r2,r2+11,B.row_begin(2*i+1));
    }

    DynMatrix<float> Cv = B.pinv() * U;
    return FixedMatrix<float,4,3>(Cv[0],Cv[1],Cv[2],Cv[3],
                                  Cv[4],Cv[5],Cv[6],Cv[7],
                                  Cv[8],Cv[9],Cv[10],1);
  }
  
  static Vec vec3to4(const FixedMatrix<float,1,3> &v,float x=0){
    return Vec(v[0],v[1],v[2],x);
  }
  
  
  class StochasticCameraOptimizer : public StochasticOptimizer<float>{
    Camera cam;
    const std::vector<Vec> &XWs;
    const std::vector<Point32f> &XIs;
    float data[7];
    float noise[7];
    float noiseVar;
    bool optimizeFocalLength;
    bool useAnnealing;
  public:
    StochasticCameraOptimizer(const Camera &cam, 
                              const std::vector<Vec> &XWs,
                              const std::vector<Point32f> &XIs,
                              float noiseVar,
                              bool optimizeFocalLength,
                              bool useAnnealing):
      StochasticOptimizer<float>(optimizeFocalLength ? 7 : 6/*(f) and pos and rot*/),cam(cam),XWs(XWs),
      XIs(XIs),noiseVar(noiseVar),optimizeFocalLength(optimizeFocalLength),useAnnealing(useAnnealing)
    {
      randomSeed();
    }
    Camera camFromData(const float *data){
      Camera cam = this->cam;
      cam.translate(data[0]*10,data[1]*10,data[2]*10);
      cam.rotate(data[3],data[4],data[5]);
      if(optimizeFocalLength) cam.setFocalLength(this->cam.getFocalLength()+data[6]*10);
      /*
          cam.translate(data[0],data[1],data[2]);
          cam.rotate(data[3],data[4],data[5]);
          if(optimizeFocalLength) cam.setFocalLength(this->cam.getFocalLength()+data[6]);
      */

      return cam;
    }
    
  protected:
    virtual void reinitialize(){
      std::fill(data,data+7,0);
    }
    virtual float *getData(){ return data; }
    virtual float getError(const float *data){
      return ExtrinsicCameraCalibrator::estimateRMSE(XWs,XIs,camFromData(data));
      /*
          Camera cam = camFromData(data);
          float err = 0;
          const std::vector<Point32f> XIs2 = cam.project(XWs);
          for(unsigned int i=0;i<XIs.size();++i){
          err += (XIs[i]-XIs2[i]).norm();
          }
          return err/XIs.size();
       */
    }
    virtual const float *getNoise(int currentTime, int endTime){
      (void)currentTime;(void)endTime;
      if(useAnnealing){
        float relTime = float(currentTime)/float(endTime);
        float noiseFac = exp(-relTime*5);   // ???
        std::fill(noise,noise+7,GRand(0,noiseFac*noiseVar));
      }else{
        std::fill(noise,noise+7,GRand(0,noiseVar));
      }
      return noise;
    }
  };
  
  void ExtrinsicCameraCalibrator::checkAndFixPoints(std::vector<Vec> &XWs, std::vector<Point32f> &XIs) throw (NotEnoughDataPointsException){
    if(XWs.size() > XIs.size()){
      ERROR_LOG("got more world points than image points (erasing additional world points)");
      XWs.resize(XIs.size());
    } else if(XWs.size() < XIs.size()){
      ERROR_LOG("got less world points than image points (erasing additional image points)");
      XIs.resize(XWs.size());
    }
    if(XWs.size() < 6){
      throw NotEnoughDataPointsException();
    }
    for(unsigned int i=0;i<XWs.size();++i){
      XWs[i][3]=1;
    }
  }
  
#if 0

  // OLD style .. will be removed soon
  Camera ExtrinsicCameraCalibrator::calibrate(std::vector<Vec> XWs, 
                                              std::vector<Point32f> XIs,
                                              const Size &imageSize, 
                                              const float focalLength,
                                              float *rmse, 
                                              bool optimizeFocalLength) const throw (ExtrinsicCameraCalibrator::InvalidWorldPositionException,
                                                                                     ExtrinsicCameraCalibrator::NotEnoughDataPointsException){
    
    checkAndFixPoints(XWs,XIs);
    
    if(m_method == "linear" || m_method == "linear+stochastic"){
      std::vector<Point32f> XIs_save = XIs;
      float fInv = 1.0/focalLength;
      Camera cam;
      cam.setViewPort(Rect(Point::null,imageSize));
      cam.setFocalLength(focalLength);
      
      for(unsigned int i=0;i<XIs.size();++i){
        XIs[i] = cam.removeViewPortTransformation(XIs[i])*fInv;
      }
      
      FixedMatrix<float,4,3> Q;
      try{
        Q = compute_Q_Matrix(XWs, XIs);
      }catch(...){
        throw InvalidWorldPositionException();
      }
      FixedMatrix<float,3,3> M = Q.part<0,0,3,3>();
      FixedMatrix<float,1,3> c4 = Q.col(3);
      
      FixedMatrix<float,3,3> K; // intrinsic parameters
      FixedMatrix<float,3,3> R; // extrinsic (rotation matrix)
      FixedMatrix<float,1,3> T; // extrinsic (tranlation vector)
  
      decompose_QR(M,R,K);
      T = -M.inv() * c4;
      
      /*
          std::cout <<"Computed Pos: " << T.transp() << std::endl;
          std::cout <<"Orig Cam Pos: "<< scene.getCamera(0).getPos().transp() << std::endl;
          std::cout << "----------" << std::endl;
      */
      
      Vec pos(T[0],T[1],T[2]);
      Vec norm = vec3to4(R.transp()*FixedMatrix<float,1,3>(0,0,1));
      Vec up = vec3to4(R.transp()*FixedMatrix<float,1,3>(0,1,0));
      
      if(m_method == "linear+stochastic"){
        Camera cam(pos,norm, up, Rect(Point::null,imageSize),focalLength);
        StochasticCameraOptimizer stochOpt(cam,XWs,XIs_save,m_params[1], optimizeFocalLength,false); 
        StochasticOptimizer::Result result = stochOpt.optimize(m_params[0]);
        cam = stochOpt.camFromData(result.data);
        if(rmse) *rmse = estimateRMSE(XWs,XIs_save,cam);
        return cam;
      }else{
        Camera cam(pos,norm, up, Rect(Point::null,imageSize),focalLength);
        if(rmse) *rmse = estimateRMSE(XWs,XIs_save,cam);
        return cam;
      }
    }
    return Camera();
  }
#endif

  ExtrinsicCameraCalibrator::Result 
  ExtrinsicCameraCalibrator::calibrateLinear(std::vector<Vec> XWs,
                                             std::vector<Point32f> XIs,
                                             const Size &imageSize,
                                             const float &focalLength) throw (InvalidWorldPositionException,
                                                                              NotEnoughDataPointsException){
    checkAndFixPoints(XWs,XIs);
    std::vector<Point32f> XIs_save = XIs;
    
    float fInv = 1.0/focalLength;
    Camera cam;
    cam.setViewPort(Rect(Point::null,imageSize));
    cam.setFocalLength(focalLength);
    
    for(unsigned int i=0;i<XIs.size();++i){
      XIs[i] = cam.removeViewPortTransformation(XIs[i])*fInv;
    }
    
    FixedMatrix<float,4,3> Q;
    try{
      Q = compute_Q_Matrix(XWs, XIs);
    }catch(...){
      throw InvalidWorldPositionException();
    }
    FixedMatrix<float,3,3> M = Q.part<0,0,3,3>();
    FixedMatrix<float,1,3> c4 = Q.col(3);
    
    FixedMatrix<float,3,3> K; // intrinsic parameters
    FixedMatrix<float,3,3> R; // extrinsic (rotation matrix)
    FixedMatrix<float,1,3> T; // extrinsic (tranlation vector)
    
    decompose_QR(M,R,K);
    T = -M.inv() * c4;
    
    /*
        std::cout <<"Computed Pos: " << T.transp() << std::endl;
        std::cout <<"Orig Cam Pos: "<< scene.getCamera(0).getPos().transp() << std::endl;
        std::cout << "----------" << std::endl;
     */
    
    Vec pos(T[0],T[1],T[2]);
    Vec norm = vec3to4(R.transp()*FixedMatrix<float,1,3>(0,0,1));
    Vec up = vec3to4(R.transp()*FixedMatrix<float,1,3>(0,1,0));
    
    cam = Camera(pos,norm, up, Rect(Point::null,imageSize),focalLength);
    Result result = { "linear", cam, estimateRMSE(XWs,XIs_save,cam) };
    return result;
  }
  


  ExtrinsicCameraCalibrator::Result 
  ExtrinsicCameraCalibrator::calibrateStochastic(std::vector<Vec> XWs,
                                                 std::vector<Point32f> XIs,
                                                 const Camera &cam,
                                                 int steps, float minErrorThreshold,
                                                 bool optimizeFocalLength, bool useAnnealing) 
    throw (ExtrinsicCameraCalibrator::NotEnoughDataPointsException){
    
    checkAndFixPoints(XWs,XIs);
    
    DEBUG_LOG("stoch. opt steps: " << steps 
              << "   err-thresh:" << minErrorThreshold 
              << " optF: " << (optimizeFocalLength?"yes":"no" )              
              << " ann: " << (useAnnealing?"yes":"no" )
              );
    StochasticCameraOptimizer stochOpt(cam,XWs,XIs,useAnnealing ? 0.01 : 0.001, optimizeFocalLength, useAnnealing); 
    StochasticOptimizer<float>::Result resStOpt = stochOpt.optimize(minErrorThreshold,steps);
    Result result = { 
      "stochastic" ,
      stochOpt.camFromData(resStOpt.data),
      resStOpt.error//estimateRMSE(XWs,XIs,restStOpt.cam)
    };
    return result;
  }
  
  ExtrinsicCameraCalibrator::Result 
  ExtrinsicCameraCalibrator::calibrateLinearAndStochastic(std::vector<Vec> XIs,
                                                          std::vector<Point32f> XWs,
                                                          const Size &imageSize, 
                                                          const float &focalLength,
                                                          int steps, float minErrorThreshold,
                                                          bool optimizeFocalLength, bool useAnnealing) 
    throw (ExtrinsicCameraCalibrator::InvalidWorldPositionException,
           ExtrinsicCameraCalibrator::NotEnoughDataPointsException){
    Result lin = calibrateLinear(XIs,XWs,imageSize,focalLength);
    Result linAndStochRes = calibrateStochastic(XIs,XWs,lin.camera,steps,minErrorThreshold,optimizeFocalLength,useAnnealing);
    linAndStochRes.method+="+stochastic";
    return linAndStochRes;
  }
  




  float ExtrinsicCameraCalibrator::estimateRMSE(const std::vector<Vec> &worldPoints,                      
                                                const std::vector<Point32f> XIs,
                                                const Camera &cam){
    float err = 0;
    const std::vector<Point32f> XIs2 = cam.project(worldPoints);
    for(unsigned int i=0;i<XIs.size();++i){
      //err += XIs[i].distanceTo(XIs2[i]);
      err += (XIs[i]-XIs2[i]).norm();
    }
    return err/XIs.size();
  }

}
