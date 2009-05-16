#include <iclExtrinsicCameraCalibrator.h>
#include <iclFixedMatrixUtils.h>
#include <iclDynMatrix.h>

namespace icl{
  

  ExtrinsicCameraCalibrator::ExtrinsicCameraCalibrator(const std::string method)
        throw (ExtrinsicCameraCalibrator::UnknownCalibrationMethodException):
    m_method(method){
    if(method != "linear"){
      throw UnknownCalibrationMethodException();
    }
  }

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
    
  Camera ExtrinsicCameraCalibrator::calibrate(std::vector<Vec> XWs, 
                                              std::vector<Point32f> XIs,
                                              const Size &imageSize, 
                                              const float focalLength) const throw (ExtrinsicCameraCalibrator::InvalidWorldPositionException,
                                                                                    ExtrinsicCameraCalibrator::NotEnoughDataPointsException){
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
       
    if(m_method == "linear"){
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
      
      return Camera(pos,norm, up, Rect(Point::null,imageSize),focalLength);
    }
    
    return Camera();
  }
}
