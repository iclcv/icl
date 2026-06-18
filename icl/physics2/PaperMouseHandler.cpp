// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/PaperMouseHandler.h>
#include <icl/physics2/PaperDriver.h>
#include <icl/physics2/FoldDriver.h>
#include <icl/physics2/PaperMoverDriver.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Node.h>
#include <icl/geom/Camera.h>
#include <icl/geom/ViewRay.h>
#include <icl/geom/PlaneEquation.h>
#include <icl/qt/MouseHandler.h>

namespace icl::physics2 {

  using utils::Point32f;

  namespace { enum Mode { None, FoldMode, GrabMode }; }

  struct PaperMouseHandler::Data {
    geom2::Scene2 *scene;
    int camIndex;
    PaperDriver *paper;
    FoldDriver *fold = nullptr;
    PaperMoverDriver *mover = nullptr;
    Mode mode = None;
    Point32f foldStart{-1, -1};
    geom::Vec foldStart3D{0, 0, 0, 1};
    geom::PlaneEquation dragPlane;
  };

  PaperMouseHandler::PaperMouseHandler(int cameraIndex, geom2::Scene2 *scene, PaperDriver *paper)
    : geom2::Scene2MouseHandler(cameraIndex, scene), m_data(std::make_unique<Data>()) {
    m_data->scene = scene;
    m_data->camIndex = cameraIndex;
    m_data->paper = paper;
    // Resolve the sibling behaviour drivers once (the dispatch targets).
    if (paper && paper->node()) {
      m_data->fold = paper->node()->getDriver<FoldDriver>();
      m_data->mover = paper->node()->getDriver<PaperMoverDriver>();
    }
  }

  PaperMouseHandler::~PaperMouseHandler() = default;

  void PaperMouseHandler::process(const qt::MouseEvent &e) {
    Data &d = *m_data;
    const geom::Camera &cam = d.scene->getCamera(d.camIndex);
    // CAMERA-resolution pixels (relative pos * resolution) — the space
    // getViewRay/estimate3DPosition expect, not raw widget pixels (e.getPos()).
    const utils::Point32f camPix(e.getRelPos().x * cam.getResolution().width,
                                 e.getRelPos().y * cam.getResolution().height);
    geom::ViewRay ray = cam.getViewRay(camPix);

    // The wheel and the right button always drive the camera (zoom / orbit).
    if (e.isWheelEvent() || e.isRight()) {
      geom2::Scene2MouseHandler::process(e);
      return;
    }

    if (e.isPressEvent() && e.isLeft()) {
      // Qt remaps physical Ctrl -> Meta on macOS, so accept either as "Ctrl".
      const bool ctrl  = e.isModifierActive(qt::ControlModifier) ||
                         e.isModifierActive(qt::MetaModifier);
      const bool shift = e.isModifierActive(qt::ShiftModifier);
      if (!ctrl && !shift) {               // plain left-drag -> camera orbit
        d.mode = None;
        geom2::Scene2MouseHandler::process(e);
        return;
      }
      Point32f p = d.paper->hit(ray);
      if (p.x < 0.f) {                     // modified press off the paper -> camera
        d.mode = None;
        geom2::Scene2MouseHandler::process(e);
        return;
      }
      if (ctrl && !shift) {                // Ctrl -> Fold
        d.mode = FoldMode; d.foldStart = p;
        d.foldStart3D = d.paper->interpolatePosition(p);
        d.dragPlane = geom::PlaneEquation(d.foldStart3D, cam.getNorm());
        if (d.fold) d.fold->setPreview(d.foldStart3D, d.foldStart3D);
      } else if (d.mover) {                // Shift -> Grab, Shift+Ctrl -> Sheet
        d.mode = GrabMode;
        d.dragPlane = geom::PlaneEquation(d.paper->interpolatePosition(p), cam.getNorm());
        if (ctrl) d.mover->beginSheetGrab(p);   // Shift+Ctrl
        else      d.mover->beginGrab(p);         // Shift only
      }
      return;
    }
    if (e.isReleaseEvent() && d.mode != None) {
      if (d.mode == FoldMode && d.fold) {
        d.fold->clearPreview();
        Point32f p = d.paper->hit(ray);
        if (p.x >= 0.f && p.distanceTo(d.foldStart) > 0.02f)
          d.fold->foldAlongLine(d.foldStart, p);
      } else if (d.mode == GrabMode && d.mover) {
        d.mover->endGrab();
      }
      d.mode = None;
      return;
    }
    if (e.isDragEvent() && d.mode != None) {
      if (d.mode == GrabMode && d.mover)
        d.mover->updateGrab(cam.estimate3DPosition(camPix, d.dragPlane));
      else if (d.mode == FoldMode && d.fold)   // grow the live preview line
        d.fold->setPreview(d.foldStart3D, cam.estimate3DPosition(camPix, d.dragPlane));
      return;
    }

    geom2::Scene2MouseHandler::process(e);   // moves etc. -> camera
  }

} // namespace icl::physics2
