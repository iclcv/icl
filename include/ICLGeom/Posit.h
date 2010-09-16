#ifndef ICL_POSIT_H
#define ICL_POSIT_H

#include <ICLGeom/GeomDefs.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Point32f.h>

namespace icl{

  /** \cond */
  class Camera;
  /** \endcond */
  
  /// Implementation of the posit algorithm for 6D pose detection from a single camera
  /** \section GEN General Information
      The POSIT algorithm allows for 6D object pose detection using a single camera. 
      POSIT needs an indexed set of model-points (in the model coordinate frame). In the 
      detection step, also a set of corresponding image coordinates are needed. POSIT
      runs iteratively to find an optimal object pose.
      
      We implement the 'modern' POSIT algorithm, which internally uses another norm
      as the 'classic' POSIT algorithm, both can easily be found in literature.

      \section RESTR Restrictions
      Please note, that <b>POSIT needs non-coplanar model points</b>. If the model points
      are complanar (or even almost coplanar), pose estimation results are undefined.
      TODO find heuristik for this or add another algorithm ...
      
      \section ALGO Algorithm Description
      The algorithm is well described in the paper
      "Model-based object pose in 25 lines of code" written by Dementhon and Davis in 1995
  */
  class Posit{
    struct Data; //!< internal data storage class
    Data *data;  //!< internal data storage
    
    public:
    /// creates new posit instance with optionally given termination criterions
    Posit(int maxIterations=100, float minDelta=0.001);
    
    /// creates new posit with given model points
    Posit(const std::vector<Vec> &modelPoints, int maxIterations=100, float minDelta=0.001);

    /// Destructor
    ~Posit();
    
    /// Copy constructor (all internal data is simply copied deeply)
    Posit(const Posit &other);

    /// assignment operator (all internal data is simply copied deeply)
    Posit &operator=(const Posit &other);
    
    /// sets new model points
    void setModel(const std::vector<Vec> &modelPoints);
    
    /// sets the max-iterations termination criterion
    void setMaxIterations(int maxIterations);
    
    /// sets the min-delta termination criterion
    void setMinDelta(float minDelta);
    
    /// returns the current max-iterations termination criterion
    int getMaxIterations() const;
    
    /// returns the current min-delta termination criterion
    float getMinDelta() const;
    
    /// returns the current model points
    const std::vector<Vec> &getModel() const;
    
    /// result type (basically a 4x4 homogeneous transformation matrix)
    struct Result : public Mat{
      friend class Posit::Data;
      private:
      /// private Konstructor
      inline Result():Mat(0.0){}
      /// private Konstructor
      inline Result(float r00, float r10, float r20, float tx,
                    float r01, float r11, float r21, float ty,
                    float r02, float r12, float r22, float tz):
      Mat(r00,r10,r20,tx,r01,r11,r21,ty,r02,r12,r22,tz,0,0,0,1){}
      
      public:
      /// extracts the objects euler angles from the internal matrix 
      FixedColVector<float,3> getAngles() const;
      
      /// extracts the objects translation vector (last column) form the internal matrix
      FixedColVector<float,3> getTranslation() const;
    };
    
    /// main function to obtain an objects pose from given image points and camera
    const Result &findPose(const std::vector<Point32f> &imagePoints, const Camera &cam) throw (ICLException);

    /// utility wrapper if no whole camera is available
    /** Please note, that the focal lenghts have to be combined with a cameras
        x- and y-sampling-resolution in order to get valid focal length values. In other words
        given focal length have to be relative to an x- and y-sampling-resolution of 1.0.
        Therefore, the given focal length might sometimes be in range [500,...] event 
        though the used camera has a default focal lenght of e.g. [10-55] mm.

        For ICL's icl::Camera type, the focal length values are computed as follows:
        \code
        Point32f principlePointOffset = cam.getPrincipalPointOffset();
        float focalLengthX = cam.getFocalLength()*cam.getSamplingResolutionX();
        float focalLEngthY = cam.getFocalLength()*cam.getSamplingResolutionY();
        \endcode
        
        However, if you already have an instance of icl::Camera in your code, you can simply
        use Posit::findPose(const std::vector<Point32f> &, const icl::Camera&), which
        automatically extracts the correct focal length and principal point offset values
        from the given icl::Camera instance.
    */
    const Result &findPose(const std::vector<Point32f> &imagePoints, 
                           const Point &principlePointOffset,
                           float focalLengthX, float focalLengthY) throw (ICLException);
  };
  
}

#endif
