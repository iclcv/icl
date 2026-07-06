// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Sergius Gaulik

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/la/FixedMatrix.h>
#include <icl/utils/Point.h>
#include <icl/utils/config/Configurable.h>
#include <vector>

namespace icl::cv3d {
  /** \cond */
  class Camera;
  /** \endcond */


  /// Utility class that allows for 6D pose estimation from a set of at least 4 coplanar points
  /** \section GEN General
      This class computes the 6D-pose of an object from coplanar points.

      \section POSE Pose Detection Problem
      Given at least 4 model-points (which are 2D because of the fact, that they
      have to be coplanar), where no subset of 3 points is collinear, and a set of corresponding
      pixel-locations of these points, the pose-estimation problem can be redefined as
      homography problem.

      \section OBJCS The Object Coordinate Frame
      As all reference points of the object are coplanar, you can define the objects
      coordinate in that way that all reference points lie within the z=0 plane.
      As the algorithm uses this as prior knowledge, the PlanarPoseEstimator::getPose
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
      PlanarPoseEstimator instances can be set up to return the estimated pose
      w.r.t. the world frame or with respect to the camera frame. If you obtain the pose
      w.r.t. the camera frame, you have to multiply it with the inverted camera coordinate
      system transformation matrix.

      \section SCENE Integration with ICL's Scene class
      In order to use the PlanarPoseEstimator to estimate an objects pose matrix
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
  class ICLCv3d_API PlanarPoseEstimator : public utils::Configurable{

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
    enum PoseEstimationAlgorithm{
      HomographyBasedOnly, //!< closed-form homography decomposition only (\ref ALG) — the default
      Refined,             //!< HomographyBasedOnly seed, then refine over a local se(3) tangent
                           //!< (minimise reprojection error) via the framework Optimizer<V> (NelderMead)
    };

    /// Parameter struct that is used to specify optional RANSAC parameters for the internal pose estimation
    struct RANSACSpec{
      bool useRANSAC;         //!< enables/disables RANSAC (if disabled, the other parameters are obsolete)
      int numPointsForModel;  //!< number of points used for finding initial models
      int numRandomCycles;    //!< number of RANSAC cycles performed
      float maxPointProjectionDistance; //!< maximun distance of model points to be in the consensus set

      /// poseestimation algorithm that is used during the RANSAC sampling
      /** In the final step, where the model is finalized using all points of the consensus set,
          the PlanarPoseEstimator's PoseEstimationAlgorithm is used */
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
    /** The default algorithm is HomographyBasedOnly — a closed-form homography
        decomposition that recovers exact synthetic poses to ~1e-4 px. Refined
        additionally minimises the reprojection error over a local se(3) tangent
        of that seed (framework NelderMead); it improves noisy data and is exact
        on clean data. For the two-solution (flip) planar pose use getPoses()
        (IPPE).

        @param spec Optionally the PlanarPoseEstimator can be set up to use RANSAC to
                    automatically filter out invalid points by means for stochastic sampling.
        */
    PlanarPoseEstimator(ReferenceFrame returnedPosesReferenceFrame=worldFrame,
                               PoseEstimationAlgorithm a = HomographyBasedOnly,
                               const RANSACSpec &spec = RANSACSpec());

    /// Destructor
    ~PlanarPoseEstimator();

    /// Copy constructor
    PlanarPoseEstimator(const PlanarPoseEstimator &other);

    /// Assignment operator
    PlanarPoseEstimator &operator=(const PlanarPoseEstimator &other);

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

    /// One pose hypothesis with its mean reprojection error.
    struct PoseCandidate {
      math::FixedMatrix<float,4,4> pose;  //!< in the configured reference frame
      float error;                        //!< mean reprojection error [px]
    };

    /// Estimate the pose returning BOTH solutions of the planar (IPPE) ambiguity.
    /** A single planar marker's homography admits two poses that reproject the
        corners almost equally well — the "flip" (tilted toward vs away). At
        steep/oblique views these are nearly indistinguishable and a one-pose
        solver jitters between them. This returns up to two hypotheses, best-first
        (sorted by mean reprojection error).

        CLOSED-FORM (IPPE, Collins & Bartoli 2014) — no iterative search: fit the
        homography model→normalised-image, rotate into the canonical frame where
        the object centre projects to the optical axis (so the homography's third
        column is axis-aligned), and the metric rotation reduces to completing a
        2×2 block's third row/column — whose sign freedom IS the planar flip,
        giving both rotations directly. Each rotation's translation is then a small
        linear least-squares solve. Deterministic and accurate (it recovers exact
        synthetic poses to ~1e-5 px, where the iterative path left ~0.03 px).

        Disambiguate via the error ratio err[0]/err[1] — close to 1 means genuinely
        ambiguous (decide by temporal consistency, the interior pattern, or a marker
        grid). Returns a single entry when the second solution collapses onto the
        first (fronto-parallel / well-conditioned view). Needs n>=4 points. */
    std::vector<PoseCandidate> getPoses(int n, const utils::Point32f *modelPoints,
                                        const utils::Point32f *imagePoints, const Camera &cam);

    private:
    /// internal utility function
    math::FixedMatrix<float,4,4> getPoseInternal(PoseEstimationAlgorithm a, int n,
                                                 const utils::Point32f *modelPoints,
                                                 const utils::Point32f *imagePoints,
                                                 const utils::Point32f *normalizedImagePoints,
                                                 const Camera &cam);
  };
  } // namespace icl::cv3d