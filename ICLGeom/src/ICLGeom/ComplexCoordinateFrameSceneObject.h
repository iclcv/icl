// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#ifndef ICL_HAVE_OPENGL
#if WIN32
#pragma WARNING("this header must not be included if ICL_HAVE_OPENGL is not defined")
#else
#warning "this header must not be included if ICL_HAVE_OPENGL is not defined"
#endif
#else

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/SceneObject.h>
#include <mutex>

namespace icl::geom {
  /// Special SceneObject implementation that define a visible coordinate frame
  /** In constrast to the <em>normal</em> CoordinateFrameSceneObject class, the
      ComplexCoordinateFrameSceneObject is build of cones and cylinders and
      it uses billboard text as axix label.
      The ComplexCoordinateFrameSceneObject is already integrated with the Scene
      class. Simply set scene.setDrawCoordinateFrameEnabled(true,l,t) to
      visualize a Scene's coordintate frame. If you need a coordinate frame
      that is not alligned with the scene's origin, you can use this class. */
  class ComplexCoordinateFrameSceneObject : public SceneObject{
    /// internally used mutex
    mutable std::recursive_mutex mutex;

    /// length of each axis
    float axisLength;

    /// thickness of each axis
    float axisThickness;

    /// axis labels
    std::string xLabel, yLabel, zLabel;

    public:

    /// Default constructor with useful default size
    ICLGeom_API ComplexCoordinateFrameSceneObject(float axisLength=100,float axisThickness=5,
                                                  bool withXYZLabels=true,
                                                  const std::string &xLabel="x",
                                                  const std::string &yLabel="y",
                                                  const std::string &zLabel="z");

    /// forwards initialization to setParams
    ICLGeom_API ComplexCoordinateFrameSceneObject(float axisLengths[3],float axisThickness=5,
                                                  bool withXYZLabels=true,
                                                  const std::string &xLabel="x",
                                                  const std::string &yLabel="y",
                                                  const std::string &zLabel="z",
                                                  const GeomColor &xAxisColor=GeomColor(255,0,0,255),
                                                  const GeomColor &yAxisColor=GeomColor(0,255,0,255),
                                                  const GeomColor &zAxisColor=GeomColor(0,0,255,255),
                                                  const GeomColor &textLabelColor=GeomColor(255,255,255,255),
                                                  float textScaling=1.0f);


    /// Dynamic adaption
    /** The default text label size is 3 x axisThickness. If you use very thin axes, textScaling
        can be used to compensate a resulting too small text size */
    ICLGeom_API void setParams(float axisLength[3], float axisThickness, bool withXYZLabels = true,
                               const GeomColor &xAxisColor=GeomColor(255,0,0,255),
                               const GeomColor &yAxisColor=GeomColor(0,255,0,255),
                               const GeomColor &zAxisColor=GeomColor(0,0,255,255),
                               const GeomColor &textLabelColor=GeomColor(255,255,255,255),
                               float textScaling = 1.0f);

    /// returns current length of the axis'
    inline float getAxisLength() const { return axisLength; }

    /// returns current thickness of the axis'
    inline float getAxisThickness() const { return axisThickness; }

    /// locks the internal mutex
    virtual void lock() const override { mutex.lock(); }

    /// unlocks the internal mutex
    virtual void unlock() const override { mutex.unlock(); }
  };
  } // namespace icl::geom
#endif
