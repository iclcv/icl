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
                      - "linear" TODO description here 
    */
    ExtrinsicCameraCalibrator(const std::string method="linear")
      throw (UnknownCalibrationMethodException);

    /// applies a calibration step
    Camera calibrate(std::vector<Vec> worldPoints, 
                     std::vector<Point32f> imagePoints,
                     const Size &imageSize, 
                     const float focalLength) const throw (InvalidWorldPositionException,
                                                           NotEnoughDataPointsException);

    private:
    std::string m_method;
  };


}

#endif
