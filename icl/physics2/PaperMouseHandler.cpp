// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/PaperMouseHandler.h>
#include <icl/physics2/PaperDriver.h>
#include <icl/physics2/FoldDriver.h>
#include <icl/physics2/PaperMoverDriver.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Node.h>
#include <icl/cv3d/Camera.h>
#include <icl/cv3d/ViewRay.h>
#include <icl/cv3d/PlaneEquation.h>
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
    geom::ViewRay foldRay;        // press ray; with the release ray it spans the cut plane
    geom::PlaneEquation dragPlane;
  };

  PaperMouseHandler::PaperMouseHandler(int cameraIndex, geom2::Scene2 *scene, PaperDriver *paper)
    : qt::MouseHandler(), m_data(std::make_unique<Data>()) {
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

  qt::MouseResult PaperMouseHandler::process(const qt::MouseEvent &e) {
    using qt::MouseResult;
    Data &d = *m_data;
    const geom::Camera &cam = d.scene->getCamera(d.camIndex);
    // CAMERA-resolution pixels (relative pos * resolution) — the space
    // getViewRay/estimate3DPosition expect, not raw widget pixels (e.getPos()).
    const utils::Point32f camPix(e.getRelPos().x * cam.getResolution().width,
                                 e.getRelPos().y * cam.getResolution().height);
    geom::ViewRay ray = cam.getViewRay(camPix);

    // The wheel and the right button are camera gestures: forward to the camera
    // handler installed after this one in the chain.
    if (e.isWheelEvent() || e.isRight()) return MouseResult::Forward;

    // Qt remaps physical Ctrl -> Meta on macOS (Command), so accept either as
    // "Ctrl". A paper modifier held means the gesture is ours: we consume it even
    // on a miss so the camera never orbits and loses orientation.
    const bool ctrl  = e.isModifierActive(qt::ControlModifier) ||
                       e.isModifierActive(qt::MetaModifier);
    const bool shift = e.isModifierActive(qt::ShiftModifier);
    const bool paperMod = ctrl || shift;

    if (e.isPressEvent() && e.isLeft()) {
      if (!paperMod) {                     // plain left-drag -> camera orbit
        d.mode = None;
        return MouseResult::Forward;
      }
      if (ctrl && !shift) {                // Ctrl -> Fold (starts anywhere)
        d.mode = FoldMode; d.foldRay = ray;   // the cut plane forms with the release ray
        if (d.fold) d.fold->clearPreview();
        return MouseResult::Processed;
      }
      // Grab / Sheet still need an actual point on the paper
      Point32f p = d.paper->hit(ray);
      if (p.x < 0.f) {                     // modified press off the paper:
        d.mode = None;                     // consume it — never orbit while a
        return MouseResult::Processed;     // paper modifier is held
      }
      if (d.mover) {                       // Shift -> Grab, Shift+Ctrl -> Sheet
        d.mode = GrabMode;
        d.dragPlane = geom::PlaneEquation(d.paper->interpolatePosition(p), cam.getNorm());
        if (ctrl) d.mover->beginSheetGrab(p);   // Shift+Ctrl
        else      d.mover->beginGrab(p);         // Shift only
      }
      return MouseResult::Processed;
    }
    if (e.isReleaseEvent() && d.mode != None) {
      if (d.mode == FoldMode && d.fold) {
        d.fold->clearPreview();
        // crease = where the (eye, press-ray, release-ray) plane cuts the paper
        if (auto c = d.paper->projectScreenLine(d.foldRay, ray))
          d.fold->foldAlongLine(c->first, c->second);
      } else if (d.mode == GrabMode && d.mover) {
        d.mover->endGrab();
      }
      d.mode = None;
      return MouseResult::Processed;
    }
    if (e.isDragEvent() && d.mode != None) {
      if (d.mode == GrabMode && d.mover)
        d.mover->updateGrab(cam.estimate3DPosition(camPix, d.dragPlane));
      else if (d.mode == FoldMode && d.fold) {   // live crease preview along the cut
        if (auto c = d.paper->projectScreenLine(d.foldRay, ray))
          d.fold->setPreview(d.paper->interpolatePosition(c->first),
                             d.paper->interpolatePosition(c->second));
        else d.fold->clearPreview();
      }
      return MouseResult::Processed;
    }

    // Fall-through (plain moves, plain drags, a consumed modified miss, ...):
    // consume while a paper modifier is held (the gesture is ours, never orbit),
    // otherwise forward so the camera handler drives navigation.
    return paperMod ? MouseResult::Processed : MouseResult::Forward;
  }

} // namespace icl::physics2
