#ifndef ICL_EXTRINSIC_CAMERA_CALIBRATOR_H
#define ICL_EXTRINSIC_CAMERA_CALIBRATOR_H

#include <ICLGeom/Camera.h>
#include <ICLUtils/Exception.h>

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

   
    
    struct Result{
      std::string method; //!< calibration method 
      Camera camera;      //!< found camera parameters
      float error;        //!< resulting error
    };

    
    /// applys linear a calibration
    /** @param worldPoints 3D-homogeneous points in the world 
        @param imagePoints corresponding points located in the image (in image coordinates)
        @param imageSize corresponding image size
        @param focalLength focal length of the camera (currently not estimated internally)

    */
    static Result calibrateLinear(std::vector<Vec> worldPoints,
                                  std::vector<Point32f> imagePoints,
                                  const Size &imageSize,
                                  const float &focalLength) throw (InvalidWorldPositionException,
                                                                   NotEnoughDataPointsException);

    /// applys stochastic calibration originating in a coarse starting guess of camera parameters
    /** @param worldPoints 3D-homogeneous points in the world 
        @param imagePoints corresponding points located in the image (in image coordinates)
        @param startCamera coarse starting guess for camera parameters
        @param maxSteps maximum number of optimization trials 
        @param minErrorThreshold optimization is stopped when this error is met
        @param optimizeFocalLength if set to true, focal length is optimized internally too
        @param useAnnealing if set to true, internall, an annealing factor is used during 
                            optimization progress. Current formular:
                            AnnealingFactor is exp(-5*t) where t runs from 0 to 1 (1 at step maxStep)
    */
    static Result calibrateStochastic(std::vector<Vec> worldPoints,
                                      std::vector<Point32f> imagePoints,
                                      const Camera &startCamera,
                                      int maxSteps=10000, float minErrorThreshold=0.1,
                                      bool optimizeFocalLength=true, bool useAnnealing=true) throw (NotEnoughDataPointsException);


    /// Combination of linear and stochastic optimization (most common here)
    /** Here we use the result from a linear calibration step as origin for the stochastic one. */
    static Result calibrateLinearAndStochastic(std::vector<Vec> worldPoints,
                                               std::vector<Point32f> imagePoints,
                                               const Size &imageSize, 
                                               const float &focalLength,
                                               int steps=10000, float minErrorThreshold=0.1,
                                               bool optimizeFocalLength=true, bool useAnnealing=true) throw (InvalidWorldPositionException, 
                                                                                                             NotEnoughDataPointsException);
       
    /// calculates the root mean square error between projected world points and found marker points
    static float estimateRMSE(const std::vector<Vec> &worldPoints,                      
                              const std::vector<Point32f> imagePoints,
                              const Camera &cam);
    
    private:
    
    /// This class cannot be instantiated
    ExtrinsicCameraCalibrator(){}
    
    /// internally used utility function 
    static void checkAndFixPoints(std::vector<Vec> &worldPoints, std::vector<Point32f> &imagePoints) throw (NotEnoughDataPointsException);
    
  };


}

#endif
