#ifndef ICL_EXTRINSIC_CAMERA_CALIBRATOR_H
#define ICL_EXTRINSIC_CAMERA_CALIBRATOR_H

#include <iclCamera.h>
#include <iclException.h>

namespace icl{
  
  class ExtrinsicCameraCalibrator{
    public:

    /// World positions must be in general positions
    struct InvalidWorldPositionException : public ICLException{
      InvalidWorldPositionException():ICLException(__FUNCTION__){}
    };

    /// We need at least 6 Data points in general positions
    struct NotEnoughDataPointsException  : public ICLException{
      NotEnoughDataPointsException():ICLException(__FUNCTION__){}
    };

    /// If a given method is not supported
    struct UnknownCalibrationMethodException  : public ICLException{
      UnknownCalibrationMethodException():ICLException(__FUNCTION__){}
    };

    
    /// Creates a new Instance with given calibration method
    /** @param method currently the following methods are supported:
        - "linear" linear (least-square base optimization)
        - "linear+stochastic" linear approach followed by a stochastic search
          approach (in our test sceneario, this reduced the remaining least square
          error to about 20%
        @param params method dependend parameters can be passed here
                      if params != 0, and method is "linear+stochastic", then
                      params can be a pointer to two floats,
                      - params[0] is the count of steps, that should be used for 
                        stochastic optimization (10000 at default)
                      - params[1] is the variation variance (0.001 at default)
    */
    ExtrinsicCameraCalibrator(const std::string method="linear", float *params = 0)
      throw (UnknownCalibrationMethodException);

    /// applys a calibration step
    /** @param worldPoint 3D-homogeneous points in the world 
        @param imagePoints corresponding points located in the image (in image coordinates)
        @param imageSize corresponding image size
        @param focalLength focal length of the camera (currently not estimated internally)
        @param rmse if no NULL, resulting root-mean-square error is passed to the
                    content of this pointer */
    Camera calibrate(std::vector<Vec> worldPoints, 
                     std::vector<Point32f> imagePoints,
                     const Size &imageSize, 
                     const float focalLength,
                     float *rmse=0) const throw (InvalidWorldPositionException,
                                                 NotEnoughDataPointsException);
    
    private:
    static void estimateRMSE(const std::vector<Vec> &worldPoints,                      
                             const std::vector<Point32f> imagePoints,
                             const Camera &cam, float *rmse);
    
    std::string m_method;
    std::vector<float> m_params;
  };


}

#endif
