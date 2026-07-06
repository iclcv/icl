// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Andre Ueckermann, Christof Elbrechter

#pragma once

#include <icl/cv3d/Types.h>
#include <icl/core/DataSegment.h>
#include <icl/math/la/DynMatrix.h>
#include <icl/math/fit/ModelFitter.h>
#include <cmath>

namespace icl::cv3d {
  /// class for planar RANSAC estimation on poincloud data (xyzh).
  /** The RansacPlaneFitter class does not minimize the error but simply counts the points with a distance smaller a given threshold.
      The smaller the threshold the preciser the model. Additionally, it is possible to assign all points on the plane to the initial surface.*/

  class RansacPlaneFitter{

	  public:
	    enum Mode {BEST, GPU, CPU};

	    /// Constructor
      /** Constructs an object of this class.
          @param mode the selected mode: CPU, GPU or BEST (uses GPU if available)*/
      RansacPlaneFitter(Mode mode=BEST);


      /// Destructor
      ~RansacPlaneFitter();


      struct Result{
        int numPoints;//number of destination points
        int countOn;//number of points on the model
        int countAbove;//number of points above the model
        int countBelow;//number of points below the model
        float euclideanThreshold;//selected threshold
        Vec n0;//best model (normal)
        float dist;//best model (distance)
        int tolerance;//tolerance for ON_ONE_SIDE
        int acc;//number of accepted passes for ON_ONE_SIDE (result smaller tolerance)
        int nacc;//number of rejected passes for ON_ONE_SIDE (result bigger tolerance)
        //int maxID;//for assignment of points (all with this id + on plane)
      };


      enum OptimizationCriterion {
        MAX_ON=1,
        ON_ONE_SIDE=2
      };


      /// Applies the planar RANSAC estimation on a destination region with a model from the src region (e.g. for best fitting plane src=dst).
      /** @param xyzh the input xyzh from the pointcloud
          @param srcIDs vector of IDs (pointcloud position -- for structured x+y*w) of the source surface (for model determination)
          @param dstIDs vector of IDs of the destination surface (for matching)
          @param threshold the maximal euclidean distance in mm
          @param passes number of ransac passes
          @param subset the subset of points for matching (2 means every second point)
          @param tolerance number of points allowed not to be on one single side of the object for ON_ONE_SIDE
          @param optimization the optimization criterion (ON_ONE_SIDE e.g. for cutfree adjacency test or MAX_ON e.g. for best fitting plane)
          @return the Result struct.
      */
      Result apply(core::DataSegment<float,4> &xyzh, std::vector<int> &srcIDs, std::vector<int> &dstIDs,
                  float threshold, int passes, int subset, int tolerance, int optimization);


      /// Applies the planar RANSAC estimation on a destination region with a model from the src region (e.g. for best fitting plane src=dst).
      /** @param xyzh the input xyzh from the pointcloud
          @param srcPoints vector of pointcloud points of the source surface (for model determination)
          @param dstIDs vector of pointcloud points of the destination surface (for matching)
          @param threshold the maximal euclidean distance in mm
          @param passes number of ransac passes
          @param subset the subset of points for matching (2 means every second point)
          @param tolerance number of points allowed not to be on one single side of the object for ON_ONE_SIDE
          @param optimization the optimization criterion (ON_ONE_SIDE e.g. for cutfree adjacency test or MAX_ON e.g. for best fitting plane)
          @return the Result struct.
      */
      Result apply(std::vector<Vec> &srcPoints, std::vector<Vec> &dstPoints, float threshold,
                  int passes, int subset, int tolerance, int optimization);


      /// Applies the planar RANSAC estimation on multiple pairs of surfaces (given by a boolean test matrix).
      /** @param xyzh the input xyzh from the pointcloud
          @param pointsIDs vector (surfaces) of vector IDs (pointcloud position -- for structured x+y*w) of the surfaces
          @param testMatrix boolean matrix for surface pair testing (1 = test, 0 = dont test)
          @param threshold the maximal euclidean distance in mm
          @param passes number of ransac passes
          @param tolerance number of points allowed not to be on one single side of the object for ON_ONE_SIDE
          @param optimization the optimization criterion (ON_ONE_SIDE e.g. for cutfree adjacency test or MAX_ON e.g. for best fitting plane)
          @param labelImage the labelImage with the point label
          @return matrix of Result structs.
      */
      math::DynMatrix<Result> apply(core::DataSegment<float,4> &xyzh, std::vector<std::vector<int> > &pointIDs,
                  math::DynMatrixBase<bool> &testMatrix, float threshold, int passes, int tolerance, int optimization, core::Img32s labelImage);


      /// Create a label image of all points on the planar model (incl. the original surface)
      /** @param xyzh the input xyzh from the pointcloud
          @param newMask the mask (e.g. ROI) for the relabeling
          @param oldLabel the original label image
          @param newLabel the resulting label image with all points on the model
          @param desiredID the id in the resulting label image
          @param srcID the ID of the label image from the fitted surface
          @param threshold the maximum euclidean distance for relabeling
          @param result the result struct returned by apply (contains the model and the original surface ID)
      */
      void relabel(core::DataSegment<float,4> &xyzh, core::Img8u &newMask, core::Img32s &oldLabel, core::Img32s &newLabel,
                   int desiredID, int srcID, float threshold, Result &result);


      /// Creates random models (n and distance) for RANSAC
      /** @param srcPoints the input points
          @param n0 the empty input n0 vector
          @param dist the empty input distance vector
          @param passes the number of passes
      */
      static void calculateRandomModels(std::vector<Vec> &srcPoints, std::vector<Vec> &n0, std::vector<float> &dist, int passes);


      /// Creates random models (n and distance) for RANSAC
      /** @param xyzh the input xyz pointcloud data
          @param srcPoints the sourcePoint Ids
          @param n0 the empty input n0 vector
          @param dist the empty input distance vector
          @param passes the number of passes
      */
      static void calculateRandomModels(core::DataSegment<float,4> &xyzh, std::vector<int> &srcPoints, std::vector<Vec> &n0, std::vector<float> &dist, int passes);

    private:

      struct Data;  //!< internal data type
      Data *m_data; //!< internal data pointer

      void calculateMultiCL(core::DataSegment<float,4> &xyzh, core::Img32s labelImage, math::DynMatrixBase<bool> &testMatrix, float threshold, int passes,
                  std::vector<Vec> &n0, std::vector<float> &dist, std::vector<int> &cAbove, std::vector<int> &cBelow, std::vector<int> &cOn,
                  std::vector<int> &adjs, std::vector<int> &start, std::vector<int> &end);

      void calculateMultiCPU(core::DataSegment<float,4> &xyzh, std::vector<std::vector<int> > &pointIDs, math::DynMatrixBase<bool> &testMatrix,
                  float threshold, int passes, std::vector<std::vector<Vec> > &n0Pre, std::vector<std::vector<float> > &distPre, std::vector<int> &cAbove,
                  std::vector<int> &cBelow, std::vector<int> &cOn, std::vector<int> &adjs, std::vector<int> &start, std::vector<int> &end);

      void calculateSingleCL(std::vector<Vec> &dstPoints, float threshold, int passes, int subset,
                  std::vector<Vec> &n0, std::vector<float> &dist, std::vector<int> &cAbove, std::vector<int> &cBelow, std::vector<int> &cOn);

      void calculateSingleCPU(std::vector<Vec> &dstPoints, float threshold, int passes, int subset,
                  std::vector<Vec> &n0, std::vector<float> &dist, std::vector<int> &cAbove, std::vector<int> &cBelow, std::vector<int> &cOn);

      void initOpenCL();

      Result createResult(std::vector<Vec> &n0, std::vector<float> &dist, std::vector<int> &cAbove, std::vector<int> &cBelow, std::vector<int> &cOn,
              float threshold, int passes, int tolerance, int optimization, int numPoints);

      math::DynMatrix<Result> createResultMatrix(math::DynMatrixBase<bool> &testMatrix, std::vector<int> &start, std::vector<int> &end, std::vector<int> &adjs,
                 std::vector<int> &cAbove, std::vector<int> &cBelow, std::vector<int> &cOn, std::vector<std::vector<int> > &pointIDs,
                 std::vector<std::vector<Vec> > &n0Pre, std::vector<std::vector<float> > &distPre, float threshold, int passes, int tolerance, int optimization);

      static void calculateModel(Vec &fa, Vec &fb, Vec &rPoint, Vec &n0, float &dist);

      void relabelCL(core::DataSegment<float,4> &xyzh, core::Img8u &newMask, core::Img32s &oldLabel, core::Img32s &newLabel,
                   int desiredID, int srcID, float threshold, Result &result, int w, int h);

      void relabelCPU(core::DataSegment<float,4> &xyzh, core::Img8u &newMask, core::Img32s &oldLabel, core::Img32s &newLabel,
                   int desiredID, int srcID, float threshold, Result &result, int w, int h);

  };

  /// A plane model: a point p satisfies p·n0 == dist on the plane.
  /** Same (normal, offset) representation as RansacPlaneFitter::Result (n0, dist)
      and its OpenCL kernels, so the generic PlaneFitter and the tuned
      point-cloud pipeline speak the same model. */
  struct PlaneModel {
    Vec n0 = Vec(0,0,1,0);   //!< unit normal (homogeneous w unused)
    float dist = 0.f;        //!< plane offset: on-plane points satisfy p·n0 == dist
    PlaneModel() = default;
    PlaneModel(const Vec &n0, float dist) : n0(n0), dist(dist) {}
    /// signed point-plane distance (p·n0 - dist); sign is the side of the plane
    float signedDistance(const Vec &p) const {
      return p[0]*n0[0] + p[1]*n0[1] + p[2]*n0[2] - dist;
    }
  };

  /// ModelFitter face: total-least-squares plane fit to a set of 3D points.
  /** Fits the plane (n0, dist) that minimises the sum of squared point-plane
      distances (PCA — the normal is the eigenvector of the smallest covariance
      eigenvalue). Being a math::ModelFitter it composes with the generic
      robustifiers: wrap it in math::RobustFitter<Vec,PlaneModel> for a robust
      plane fit that rejects outliers. This is the generic, unstructured-point
      counterpart to RansacPlaneFitter's tuned OpenCL multi-surface pipeline
      (which stays the specialized engine for structured point-cloud data). */
  class ICLCv3d_API PlaneFitter : public math::ModelFitter<Vec, PlaneModel> {
    public:
    /// least-squares plane through all given points (needs >= 3 non-collinear)
    PlaneModel fit(const std::vector<Vec> &pts) override;
    /// non-negative point-plane distance |p·n0 - dist|
    double residual(const PlaneModel &m, const Vec &p) const override {
      return std::fabs(m.signedDistance(p));
    }
    /// a plane needs 3 points
    int minSamples() const override { return 3; }
  };

  } // namespace icl::cv3d