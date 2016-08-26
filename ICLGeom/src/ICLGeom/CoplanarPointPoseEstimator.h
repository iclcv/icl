/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/CoplanarPointPoseEstimator.h       **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Sergius Gaulik                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLMath/FixedMatrix.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Configurable.h>

namespace icl{
  namespace geom{
    
    /** \cond */
    class Camera;
    /** \endcond */
    
  
    /// Utility class that allows for 6D pose estimation from a set of at least 4 coplanar points
    /** \section GEN General
        In contrast to the POSIT algorithms (see icl::Posit), this class is able to compute
        the 6D-pose of an object from coplanar points.
        
        \section POSE Pose Detection Problem
        Given at least 4 model-points (which are 2D because of the fact, that they
        have to be coplanar), where no subset of 3 points is collinear, and a set of corresponding
        pixel-locations of these points, the pose-estimation problem can be redefined as
        homography problem.
  
        \section OBJCS The Object Coordinate Frame
        As all reference points of the object are coplanar, you can define the objects
        coordinate in that way that all reference points lie within the z=0 plane. 
        As the algorithm uses this as prior knowledge, the CoplanarPointPoseEstimator::getPose
        interface does allow to pass 2D object reference points only.
        
        \section ALG Algorithm
        We implemented the algorithm from the paper 
        "Pose estimation based on four coplanar point correspondences" written by Yang et. al. 
        and published 2009. Internally, a 9 by 2N matrix A is created. The solution for the
        pose detection problem is basically achieved by minimizing |Ax| using SVD. 
        Finally, the resulting x can be used to obtain the transformation matrix. We also added
        one heuristic to the method described in the paper above. As the projection of a plane 
        is ambiguous (you can project from the front and from the back), we always use always 
        use the solution, where the resulting z-value is in front of the camera (Otherwise, we
        would not be able to see it). Mathematically, this means, that we do not always use
        x, but sometimes also -x, which obviously leads to the same error |Ax|.
        
        \section REF Reference Frames
        CoplanarPointPoseEstimator instances can be set up to return the estimated pose
        w.r.t. the world frame or with respect to the camera frame. If you obtain the pose
        w.r.t. the camera frame, you have to multiply it with the inverted camera coordinate
        system transformation matrix.
        
        \section SCENE Integration with ICL's Scene class
        In order to use the CoplanarPointPoseEstimator to estimate an objects pose matrix
        for visualization using an instance of the icl::Scene class, you can simply define an
        objects base vertices in the object coordinate frame:\n
        e.g. (1,1,1,1), (1, 1,-1,1), ... for a unity cube.
        For moving these points to their real location in the 3D scene, you simply have to 
        transform these vectors by the obtained 'w.r.t. world frame' transformation matrix.
        
        \section BENCH Benchmark Results
        Simple Pose estimation with 4 points needs about 80 ns on an Intel(R) Xeon(R) E5530
        (2.40GHz). If 9 points are used, it needs about 110 ns.


        \section RANSAC Enabling RANSAC 
        As a new feature, the pose estimation step can be optimized using RANSAC internally.
        @TODO perhaps, we need some more text here !
    */
    class ICLGeom_API CoplanarPointPoseEstimator : public utils::Configurable{
  
      /// Internal data structure
      struct Data;
      
      /// Internally used data pointer
      Data *data;
  
      /// Internally used to sync property settings with internal data
      void propertyChangedCallback(const Property &p);
      
#if !(defined ICL_MSC_VER && ICL_MSC_VER < 1800)
      /// Internally used to correct the first transformation matrix using robust pose estimation algorithm
      void robustPoseCorrection(int n, const utils::Point32f *modelPoints, 
                                const utils::Point32f *normalizedImagePoints);
#endif
      
      public:
  
      /// Reference frame enumeration
      enum ReferenceFrame{
        cameraFrame, //!< poses are returned w.r.t. the camera frame
        worldFrame   //!< poses are returned w.r.t. the world frame
      };
  
      /// Algorithm, that is used for pose-estimation
      /** TODO: describe the algorithms*/
      enum PoseEstimationAlgorithm{
        HomographyBasedOnly, //!< uses the above described algorithm (\ref ALG) only
        SimplexSampling,     //!< performs simplex sampling for optimization (very fast and very accurate!
        SamplingCoarse,      //!< use some predefined sampling parameters for brute force coase sampling (fast)
        SamplingMedium,      //!< use some predefined sampling parameters for brute force medium sampling (average speed)
        SamplingFine,        //!< use some predefined sampling parameters for brute force fine sampling (slow)
        SamplingCustom,      //!< uses custom properties to define sampling parameters for brute force sampling
      };
      
      /// Parameter struct that is used to specify optional RANSAC parameters for the internal pose estimation
      struct RANSACSpec{
        bool useRANSAC;         //!< enables/disables RANSAC (if disabled, the other parameters are obsolete)
        int numPointsForModel;  //!< number of points used for finding initial models
        int numRandomCycles;    //!< number of RANSAC cycles performed
        float maxPointProjectionDistance; //!< maximun distance of model points to be in the consensus set

        /// poseestimation algorithm that is used during the RANSAC sampling
        /** In the final step, where the model is finalized using all points of the consensus set,
            the CoplanarPointPoseEstimator's PoseEstimationAlgorithm is used */
        PoseEstimationAlgorithm poseEstimationDuringSampling; 

        /// Constructor with given parameters and defaults
        /** By default, RANSAC is disabled */
        RANSACSpec(bool useRANSAC = false, int numPointsForModel = 4, int numRandomCycles=20, 
                    float maxPointProjectionDistance = 10.0f,
                    PoseEstimationAlgorithm poseEstimationDuringSampling = HomographyBasedOnly):
          useRANSAC(useRANSAC), numPointsForModel(numPointsForModel),numRandomCycles(numRandomCycles),
          maxPointProjectionDistance(maxPointProjectionDistance),
          poseEstimationDuringSampling(poseEstimationDuringSampling){}
      };
      
      /// Default constructor with given reference-frame for the returned poses
      /** Please note that the Downhill Simplex based pose optimization is very accurate and very fast.
          Using other modes does usually slowdown the pose estimation process <b>and</b> also decrease
          the result quality. Hovever the brute force search is still provided due to 'historic' reasons.
          
          @param spec Optionally the CoplanarPointPoseEstimator can be set up to use RANSAC to 
                      automatically filter out invalid points by means for stochastic sampling. 
          */
      CoplanarPointPoseEstimator(ReferenceFrame returnedPosesReferenceFrame=worldFrame, 
                                 PoseEstimationAlgorithm a = SimplexSampling,
                                 const RANSACSpec &spec = RANSACSpec());
  
      /// Destructor
      ~CoplanarPointPoseEstimator();
      
      /// Copy constructor
      CoplanarPointPoseEstimator(const CoplanarPointPoseEstimator &other);
      
      /// Assignment operator
      CoplanarPointPoseEstimator &operator=(const CoplanarPointPoseEstimator &other);
      
      /// returns the current reference frame value
      ReferenceFrame getReferenceFrame() const;
      
      /// sets the refernce-frame property
      void setReferenceFrame(ReferenceFrame f);
      
      /// main working function that estimates the pose from given model- and image points
      /** @see \ref POSE 
          @see \ref OBJCS
          @see \ref ALG
          
          note: the camera class is only forward-declared for this file
      */
      math::FixedMatrix<float,4,4> getPose(int n, const utils::Point32f *modelPoints, 
                                           const utils::Point32f *imagePoints, const Camera &cam);

      private:
      /// internal utility function
      math::FixedMatrix<float,4,4> getPoseInternal(PoseEstimationAlgorithm a, int n, 
                                                   const utils::Point32f *modelPoints, 
                                                   const utils::Point32f *imagePoints, 
                                                   const utils::Point32f *normalizedImagePoints, 
                                                   const Camera &cam);
    };
  } // namespace geom
}

