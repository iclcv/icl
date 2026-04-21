// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/Scene2MouseHandler.h>

#ifdef ICL_HAVE_QT

#include <icl/geom2/Scene2.h>
#include <icl/geom/Camera.h>
#include <icl/geom/ViewRay.h>
#include <icl/math/HomogeneousMath.h>
#include <icl/qt/MouseEvent.h>
#include <cstring>

using namespace icl::utils;
using namespace icl::qt;
using namespace icl::math;
using namespace icl::geom;

namespace icl::geom2 {

  // --- Sensitivity levels ---
  struct Sens {
    float translation, rotation, mouse, wheel;
  };

  enum SensLevel { Low = 0, Normal = 1, High = 2, NUM_SENS = 3 };

  // --- Action function signature ---
  using ActionFn = void(*)(const MouseEvent&, const Point32f&, const Point32f&,
                           Camera&, Scene2&, void*);
  struct Mapping { ActionFn fn; void *data; };

  // --- Camera action implementations ---

  static void freeView(const MouseEvent &e, const Point32f &pos,
                       const Point32f &delta, Camera &cam, Scene2 &scene, void *data) {
    auto *s = static_cast<Sens*>(data);
    float rf = s->rotation;
    float df = e.isWheelEvent() ? s->wheel : s->mouse;

    Vec norm = cam.getNorm();
    Vec up   = cam.getUp();
    norm = rotate_vector(up, -rf * df * delta.x, norm);
    norm = rotate_vector(cam.getHoriz(), rf * df * delta.y, norm);
    up   = rotate_vector(cam.getHoriz(), rf * df * delta.y, up);
    cam.setNorm(norm);
    cam.setUp(up);
  }

  static void strafe(const MouseEvent &e, const Point32f &pos,
                     const Point32f &delta, Camera &cam, Scene2 &scene, void *data) {
    auto *s = static_cast<Sens*>(data);
    float tf = s->translation;
    float df = e.isWheelEvent() ? s->wheel : s->mouse;

    Vec t = cam.getUp() * (-tf * df * delta.y) + cam.getHoriz() * (-tf * df * delta.x);
    t[3] = 0;
    cam.translate(t);
  }

  static void rotateAroundOrigin(const MouseEvent &e, const Point32f &pos,
                                 const Point32f &delta, Camera &cam, Scene2 &scene, void *data) {
    auto *s = static_cast<Sens*>(data);
    float rf = 2.0f * s->rotation;
    float df = e.isWheelEvent() ? s->wheel : -s->mouse;

    Vec cursor = scene.getCursor();
    Vec position = cam.getPosition() - cursor;
    Vec norm  = cam.getNorm();
    Vec up    = cam.getUp();
    Vec horiz = cam.getHoriz();

    position = rotate_vector(up,    -rf * df * delta.x, position);
    position = rotate_vector(horiz,  rf * df * delta.y, position);
    norm = rotate_vector(horiz, rf * df * delta.y, norm);
    up   = rotate_vector(horiz, rf * df * delta.y, up);
    norm = rotate_vector(up,   -rf * df * delta.x, norm);

    cam.setPosition(position + cursor);
    cam.setNorm(norm);
    cam.setUp(up);
  }

  static void rollAndDistance(const MouseEvent &e, const Point32f &pos,
                              const Point32f &delta, Camera &cam, Scene2 &scene, void *data) {
    auto *s = static_cast<Sens*>(data);
    float rf = s->rotation;
    float tf = s->translation;
    float df = e.isWheelEvent() ? s->wheel : -s->mouse;

    Vec up = rotate_vector(cam.getNorm(), rf * df * delta.x, cam.getUp());
    cam.setUp(up);

    Vec t = cam.getNorm() * (tf * df * delta.y);
    t[3] = 0;
    cam.translate(t);
  }

  static void placeCursor(const MouseEvent &e, const Point32f &pos,
                          const Point32f &delta, Camera &cam, Scene2 &scene, void *data) {
    float px = pos.x * cam.getResolution().width;
    float py = pos.y * cam.getResolution().height;
    auto hit = scene.findObject(cam.getViewRay(Point32f(px, py)));
    if (hit) {
      scene.setCursor(hit.pos);
    }
  }

  // --- Data ---

  struct Scene2MouseHandler::Data {
    Scene2 *scene;
    int camIndex;
    Sens sens[NUM_SENS];
    Camera camBackup;
    Point32f anchor;
    int modBackup = 0;

    // Mapping table: [eventType][left][mid][right][shift][ctrl][alt]
    Mapping table[MAX_MOUSE_EVENT + 1][2][2][2][2][2][2];

    void setMapping(MouseEventType evt, bool l, bool m, bool r,
                    bool shift, bool ctrl, bool alt, ActionFn fn, void *d) {
      table[evt][l][m][r][shift][ctrl][alt] = {fn, d};
    }

    void setDefaultMappings() {
      // Left: freeView
      setMapping(MouseDragEvent, 1,0,0, 0,0,0, freeView, &sens[Normal]);
      setMapping(MouseDragEvent, 1,0,0, 1,0,0, freeView, &sens[Low]);
      setMapping(MouseDragEvent, 1,0,0, 0,1,0, freeView, &sens[High]);

      // Middle: strafe
      setMapping(MouseDragEvent, 0,1,0, 0,0,0, strafe, &sens[Normal]);
      setMapping(MouseDragEvent, 0,1,0, 1,0,0, strafe, &sens[Low]);
      setMapping(MouseDragEvent, 0,1,0, 0,1,0, strafe, &sens[High]);

      // Right: rotate around cursor
      setMapping(MouseDragEvent, 0,0,1, 0,0,0, rotateAroundOrigin, &sens[Normal]);
      setMapping(MouseDragEvent, 0,0,1, 1,0,0, rotateAroundOrigin, &sens[Low]);
      setMapping(MouseDragEvent, 0,0,1, 0,1,0, rotateAroundOrigin, &sens[High]);

      // Wheel: roll & dolly
      setMapping(MouseWheelEvent, 0,0,0, 0,0,0, rollAndDistance, &sens[Normal]);
      setMapping(MouseWheelEvent, 0,0,0, 1,0,0, rollAndDistance, &sens[Low]);
      setMapping(MouseWheelEvent, 0,0,0, 0,1,0, rollAndDistance, &sens[High]);

      // Left+Right: roll & dolly
      setMapping(MouseDragEvent, 1,0,1, 0,0,0, rollAndDistance, &sens[Normal]);
      setMapping(MouseDragEvent, 1,0,1, 1,0,0, rollAndDistance, &sens[Low]);
      setMapping(MouseDragEvent, 1,0,1, 0,1,0, rollAndDistance, &sens[High]);

      // Shift+Ctrl+Click: place cursor
      setMapping(MousePressEvent, 1,0,0, 1,1,0, placeCursor, nullptr);
    }
  };

  // --- Implementation ---

  Scene2MouseHandler::Scene2MouseHandler(int cameraIndex, Scene2 *scene)
    : m_data(std::make_unique<Data>()) {
    m_data->scene = scene;
    m_data->camIndex = cameraIndex;
    std::memset(m_data->table, 0, sizeof(m_data->table));
    setSensitivities(10.0f);
    m_data->setDefaultMappings();
  }

  Scene2MouseHandler::~Scene2MouseHandler() = default;

  void Scene2MouseHandler::setSensitivities(float translation, float rotation,
                                            float mouse, float wheel, float mod) {
    if (translation <= 0) translation = 1000.0f;
    if (mod == 0) mod = 10.0f;
    m_data->sens[Low]    = {translation, rotation, mouse / mod, wheel / mod};
    m_data->sens[Normal] = {translation, rotation, mouse, wheel};
    m_data->sens[High]   = {translation, rotation, mouse * mod, wheel * mod};
  }

  void Scene2MouseHandler::process(const MouseEvent &e) {
    Camera &cam = m_data->scene->getCamera(m_data->camIndex);
    int mods = e.getKeyboardModifiers();

    auto &m = m_data->table[e.getType()]
                           [e.isLeft()][e.isMiddle()][e.isRight()]
                           [e.isModifierActive(ShiftModifier)]
                           [e.isModifierActive(ControlModifier)]
                           [e.isModifierActive(AltModifier)];

    if (e.isPressEvent() || mods != m_data->modBackup) {
      m_data->anchor = e.getRelPos();
      m_data->camBackup = cam;
      if (m.fn) m.fn(e, e.getRelPos(), Point32f(0,0), cam, *m_data->scene, m.data);
    }

    if (e.isDragEvent()) {
      Point32f delta = e.getRelPos() - m_data->anchor;
      cam = m_data->camBackup;
      if (m.fn) m.fn(e, e.getRelPos(), delta, cam, *m_data->scene, m.data);
    }

    if (e.isWheelEvent()) {
      if (m.fn) m.fn(e, e.getRelPos(), e.getWheelDelta(), cam, *m_data->scene, m.data);
    }

    m_data->modBackup = mods;
  }

} // namespace icl::geom2

#endif
