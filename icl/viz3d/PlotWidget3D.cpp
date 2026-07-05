// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/viz3d/PlotWidget3D.h>
#include <icl/viz3d/Plot3D.h>
#include <icl/viz3d/PlotHandle3D.h>
#include <icl/viz3d/GroupNode.h>
#include <icl/viz3d/MeshNode.h>
#include <icl/viz3d/GeometryNode.h>
#include <icl/viz3d/TextNode.h>
#include <icl/viz3d/LightNode.h>
#include <icl/viz3d/detail/PlotFrame.h>
#include <icl/viz3d/Scene2MouseHandler.h>
#include <icl/viz3d/Material.h>
#include <icl/cv3d/Camera.h>
#include <icl/qt/GUIWidget.h>
#include <icl/qt/GLCallback.h>
#include <icl/qt/MouseHandler.h>
#include <icl/qt/MouseEvent.h>
#include <icl/math/transform/LinearTransform1D.h>
#include <icl/utils/dispatch/AssignRegistry.h>
#include <algorithm>
#include <functional>

namespace icl {
  using namespace utils;
  using namespace math;
  using namespace core;
  using namespace qt;
  using cv3d::Camera;
  using viz3d::Material;
  using cv3d::GeomColor;

  namespace viz3d {

    static const GeomColor white(255, 255, 255, 255);

    struct PlotWidget3D::Data {
      Scene2 scene;
      Range32f givenViewport[3];
      Range32f computedViewport[3];

      std::shared_ptr<GroupNode> rootObject;       // scaled data container
      std::shared_ptr<GroupNode> coordinateFrame;  // box + axes ([-1,1] space)
      std::shared_ptr<MeshNode>  box;
      std::shared_ptr<GroupNode> axes[3];
      Range32f frameRanges[3];
      int cornerSign[3] = { -1, -1, -1 };  // box corner the tick-edges meet at
      std::shared_ptr<qt::GLCallback> glCallback;  // camera-tracking render hook

      // --- hover: perpendicular section plane under the cursor ---
      std::shared_ptr<MeshNode> hoverPlane;
      std::shared_ptr<qt::MouseHandler> hoverHandler;
      int  hoverAxis = -1;
      float hoverC = 0;
      bool hoverVisible = false;

      float pointsize = 1, linewidth = 1;
      bool smoothfill = true;
      GeomColor color = cv3d::geom_red(255), fill = cv3d::geom_blue(255);

      // --- bounds (dynamic viewport) ---
      template<bool X, bool Y, bool Z>
      void update_bounds(Range32f vp[3], Node *o) {
        if (o != rootObject.get()) {
          if (auto *g = dynamic_cast<GeometryNode*>(o)) {
            for (const Vec &v : g->getVertices()) {
              if (X) { vp[0].minVal = std::min(vp[0].minVal, v[0]); vp[0].maxVal = std::max(vp[0].maxVal, v[0]); }
              if (Y) { vp[1].minVal = std::min(vp[1].minVal, v[1]); vp[1].maxVal = std::max(vp[1].maxVal, v[1]); }
              if (Z) { vp[2].minVal = std::min(vp[2].minVal, v[2]); vp[2].maxVal = std::max(vp[2].maxVal, v[2]); }
            }
          }
        }
        if (auto *grp = dynamic_cast<GroupNode*>(o))
          for (int i = 0; i < grp->getChildCount(); ++i)
            update_bounds<X,Y,Z>(vp, grp->getChild(i));
      }

      void updateBounds() {
        const bool dyn[3] = { !givenViewport[0].getLength(),
                              !givenViewport[1].getLength(),
                              !givenViewport[2].getLength() };
        for (int i = 0; i < 3; ++i)
          computedViewport[i] = dyn[i] ? Range32f::inv_limits() : givenViewport[i];
        switch (dyn[0] + dyn[1]*2 + dyn[2]*4) {
          case 1: update_bounds<true,false,false>(computedViewport, rootObject.get()); break;
          case 2: update_bounds<false,true,false>(computedViewport, rootObject.get()); break;
          case 3: update_bounds<true,true,false>(computedViewport, rootObject.get()); break;
          case 4: update_bounds<false,false,true>(computedViewport, rootObject.get()); break;
          case 5: update_bounds<true,false,true>(computedViewport, rootObject.get()); break;
          case 6: update_bounds<false,true,true>(computedViewport, rootObject.get()); break;
          case 7: update_bounds<true,true,true>(computedViewport, rootObject.get()); break;
          default: break;
        }
        // A dynamic axis with no (or degenerate) geometry keeps its inv_limits
        // seed (minVal > maxVal) — that would render as inf/huge ticks and blow
        // up the data→[-1,1] scale. Fall back to a sane finite range: a flat
        // axis (min==max) expands around its value, an empty one uses [-1,1].
        for (int i = 0; i < 3; ++i) {
          Range32f &r = computedViewport[i];
          if (r.minVal > r.maxVal)        r = Range32f(-1, 1);
          else if (r.minVal == r.maxVal)  r = Range32f(r.minVal - 1, r.maxVal + 1);
        }
      }

      void updateTics() {
        if (frameRanges[0] == computedViewport[0] &&
            frameRanges[1] == computedViewport[1] &&
            frameRanges[2] == computedViewport[2]) return;
        std::copy(computedViewport, computedViewport+3, frameRanges);
        static const std::string names[3] = { "X", "Y", "Z" };
        for (int i = 0; i < 3; ++i) {
          if (axes[i]) coordinateFrame->removeChild(axes[i].get());
          // labels are billboards pinned to their world tick position, so the
          // numeric value always reads correctly with no inversion (the old
          // invert=Y compensated for the pre-multiply axis mirroring that the
          // placePlotAxes transform-order fix removed).
          axes[i] = detail::makePlotAxis(computedViewport[i], false, names[i]);
          coordinateFrame->addChild(axes[i]);
        }
        detail::placePlotAxes(axes, cornerSign[0], cornerSign[1], cornerSign[2]);
      }

      // Re-place the tick-edges onto the corner FURTHEST from the current camera
      // (called every frame from the GL callback). Only re-runs when the corner
      // actually flips — a rare, discrete event as the view orbits — and then
      // just resets+reapplies the axis transforms (no label textures rebuilt).
      void updateCornerForCamera() {
        const Vec cp = scene.getCamera(0).getPosition();
        const int s[3] = { cp[0] > 0 ? -1 : 1, cp[1] > 0 ? -1 : 1, cp[2] > 0 ? -1 : 1 };
        if (s[0] == cornerSign[0] && s[1] == cornerSign[1] && s[2] == cornerSign[2]) return;
        std::scoped_lock lock(scene);
        std::copy(s, s+3, cornerSign);
        for (auto &a : axes) if (a) a->removeTransformation();
        detail::placePlotAxes(axes, cornerSign[0], cornerSign[1], cornerSign[2]);
      }

      // --- hover section plane ---

      // Screen-space pick: is the cursor (normalized 0..1) near one of the three
      // tick edges? If so, return the axis and the box coord [-1,1] under it.
      bool pickHover(float rx, float ry, int &axOut, float &cOut) {
        const Camera &cam = scene.getCamera(0);
        const float W = cam.getResolution().width;
        const float H = cam.getResolution().height;
        const Point32f cur(rx * W, ry * H);
        int best = -1; float bestSD = 1e9f, bestT = 0;
        for (int ax = 0; ax < 3; ++ax) {
          Vec P0(0,0,0,1), P1(0,0,0,1);
          for (int m = 0; m < 3; ++m) {
            P0[m] = (m == ax) ? -1.f : (float)cornerSign[m];
            P1[m] = (m == ax) ?  1.f : (float)cornerSign[m];
          }
          const Point32f a = cam.project(P0), b = cam.project(P1);
          const float abx = b.x - a.x, aby = b.y - a.y;
          const float L2 = abx*abx + aby*aby;
          if (L2 < 1e-6f) continue;
          float t = ((cur.x - a.x)*abx + (cur.y - a.y)*aby) / L2;
          t = std::max(0.f, std::min(1.f, t));
          const float dx = cur.x - (a.x + t*abx), dy = cur.y - (a.y + t*aby);
          const float sd = std::sqrt(dx*dx + dy*dy);
          if (sd < bestSD) { bestSD = sd; best = ax; bestT = t; }
        }
        if (best >= 0 && bestSD < 14.f) { axOut = best; cOut = -1.f + 2.f*bestT; return true; }
        return false;
      }

      // (Re)build the translucent section plane perpendicular to axis \a ax at
      // box coord \a c: a filled quad (20% alpha) + a tick-aligned grid (50%).
      void buildHoverPlane(int ax, float c) {
        const int j = (ax+1)%3, k = (ax+2)%3;
        auto P = [&](float vj, float vk) { Vec p(0,0,0,1); p[ax]=c; p[j]=vj; p[k]=vk; return p; };
        static const GeomColor line(0, 120, 255, 128);
        hoverPlane->clearGeometry();
        hoverPlane->addVertex(P(-1,-1), line); hoverPlane->addVertex(P(1,-1), line);
        hoverPlane->addVertex(P(1,1), line);   hoverPlane->addVertex(P(-1,1), line);
        hoverPlane->addQuad(0, 1, 2, 3);
        int base = 4;
        for (int t = 0; t <= 10; ++t) {
          const float p = -1.f + 0.2f*t;
          hoverPlane->addVertex(P(-1, p), line); hoverPlane->addVertex(P(1, p), line);
          hoverPlane->addLine(base, base+1, line); base += 2;
          hoverPlane->addVertex(P(p, -1), line); hoverPlane->addVertex(P(p, 1), line);
          hoverPlane->addLine(base, base+1, line); base += 2;
        }
        hoverPlane->createAutoNormals(false);
        hoverPlane->setPrimitiveVisible(PrimVertex, false);
        hoverPlane->setVisible(true);
      }

      // Returns true if the visible state changed (→ caller repaints).
      bool updateHover(float rx, float ry) {
        std::scoped_lock lock(scene);
        int ax; float c;
        if (pickHover(rx, ry, ax, c)) {
          if (hoverVisible && ax == hoverAxis && std::abs(c - hoverC) < 1e-3f) return false;
          buildHoverPlane(ax, c);
          hoverAxis = ax; hoverC = c; hoverVisible = true;
          return true;
        }
        if (!hoverVisible) return false;
        hoverPlane->setVisible(false); hoverVisible = false;
        return true;
      }

      bool hideHover() {
        std::scoped_lock lock(scene);
        if (!hoverVisible) return false;
        hoverPlane->setVisible(false); hoverVisible = false;
        return true;
      }

      // recompute root transform (data viewport -> [-1,1]^3) + retic
      void recompute() {
        updateBounds();
        LinearTransform1D tx(computedViewport[0], Range32f(-1, 1));
        LinearTransform1D ty(computedViewport[1], Range32f(-1, 1));
        LinearTransform1D tz(computedViewport[2], Range32f(-1, 1));
        rootObject->setTransformation(Mat(tx.m, 0, 0, tx.b,
                                          0, ty.m, 0, ty.b,
                                          0, 0, tz.m, tz.b,
                                          0, 0, 0, 1));
        updateTics();
      }

      void style(GeometryNode *g) {
        g->setPointSize(pointsize);
        g->setLineWidth(linewidth);
        if (color[3]) {
          g->setPrimitiveVisible(PrimVertex | PrimLine, true);
          if (auto *m = dynamic_cast<MeshNode*>(g)) {
            auto &vc = m->getVertexColors();
            std::fill(vc.begin(), vc.end(), color);
          }
        } else {
          g->setPrimitiveVisible(PrimVertex | PrimLine, false);
        }
        g->setPrimitiveVisible(PrimTriangle | PrimQuad, fill[3] != 0);
        if (fill[3] && color[3])      g->setMaterial(Material::fromColors(fill, color));
        else if (fill[3])             g->setMaterial(Material::fromColor(fill));
        else if (color[3])            g->setMaterial(Material::fromColor(color));
      }

      void add(std::shared_ptr<Node> obj, bool styleIt) {
        if (styleIt) if (auto *g = dynamic_cast<GeometryNode*>(obj.get())) style(g);
        std::scoped_lock lock(scene);
        rootObject->addChild(obj);
        recompute();
      }
    };

    namespace {
      // Wraps the scene's GL callback so a per-frame hook runs (on the GUI
      // thread, before the render) — used to keep the tick corner on the edge
      // furthest from the live camera as the view orbits.
      struct HookedGLCallback : qt::GLCallback {
        std::function<void()> onFrame;
        std::shared_ptr<qt::GLCallback> inner;
        void draw(qt::ICLDrawWidget3D *w) override {
          if (onFrame) onFrame();
          if (inner) inner->draw(w);
        }
      };
      // Passive mouse handler: forwards every event to fn and never consumes it
      // (installed BEFORE the camera handler so hover is seen even during drags).
      struct FnMouseHandler : qt::MouseHandler {
        std::function<void(const qt::MouseEvent &)> fn;
        qt::MouseResult process(const qt::MouseEvent &e) override {
          if (fn) fn(e);
          return qt::MouseResult::Forward;
        }
      };
    }

    PlotWidget3D::PlotWidget3D(QWidget *parent) : ICLDrawWidget3D(parent), m_data(new Data) {
      std::fill(m_data->givenViewport, m_data->givenViewport+3, Range32f(0, 0));
      std::fill(m_data->frameRanges, m_data->frameRanges+3, Range32f(1, 1));  // force first retic
      m_data->scene.setBounds(5);

      // default view: +X to the right, +Y up, +Z toward the viewer, slight tilt
      // for 3D — the textbook axis triad (see plot-orient-tuner to re-dial these).
      m_data->scene.addCamera(Camera::lookAt(Vec(2.5, 2.5, 7, 1), Vec(0, 0, 0, 1),
                                             Vec(0, 1, 0, 1), Size(640, 480), 30.0f));
      // hover handler first so it sees every event (incl. drags) and Forwards it
      { auto mh = std::make_shared<FnMouseHandler>();
        Data *d = m_data; PlotWidget3D *self = this;
        mh->fn = [d, self](const qt::MouseEvent &e) {
          const bool changed = (e.getType() == qt::MouseMoveEvent)
              ? d->updateHover(e.getRelPos().x, e.getRelPos().y)
              : d->hideHover();
          if (changed) self->render();
        };
        m_data->hoverHandler = mh;
        install(mh.get()); }
      install(m_data->scene.getMouseHandler(0));
      { auto hooked = std::make_shared<HookedGLCallback>();
        hooked->inner = m_data->scene.getGLCallback(0);
        Data *d = m_data;
        hooked->onFrame = [d] { d->updateCornerForCamera(); };
        m_data->glCallback = hooked;
        link(hooked.get()); }

      m_data->rootObject = std::make_shared<GroupNode>();
      m_data->coordinateFrame = std::make_shared<GroupNode>();

      // box wireframe (8 corners of the [-1,1]^3 cube)
      m_data->box = detail::makePlotBox();
      m_data->coordinateFrame->addChild(m_data->box);

      // hover section plane (hidden until the cursor points at an axis)
      m_data->hoverPlane = std::make_shared<MeshNode>();
      m_data->hoverPlane->setMaterial(
        Material::fromColors(GeomColor(0, 120, 255, 51), GeomColor(0, 120, 255, 128)));
      m_data->hoverPlane->setVisible(false);
      m_data->coordinateFrame->addChild(m_data->hoverPlane);

      m_data->scene.addNode(m_data->coordinateFrame);  // first: drives retic
      m_data->scene.addNode(m_data->rootObject);
      m_data->recompute();

      auto light = std::make_shared<LightNode>(LightNode::Point);
      light->setIntensity(1.2f);
      light->translate(0, 0, 10);
      m_data->scene.addLight(light);
    }

    PlotWidget3D::~PlotWidget3D() { delete m_data; }

    const Range32f *PlotWidget3D::getViewPort() const { return m_data->computedViewport; }
    Scene2 &PlotWidget3D::getScene() { return m_data->scene; }
    const Scene2 &PlotWidget3D::getScene() const { return m_data->scene; }
    Node *PlotWidget3D::getRootObject() { return m_data->rootObject.get(); }
    const Node *PlotWidget3D::getRootObject() const { return m_data->rootObject.get(); }
    const Camera &PlotWidget3D::getCamera() const { return m_data->scene.getCamera(0); }
    void PlotWidget3D::setCamera(const Camera &cam) { m_data->scene.getCamera(0) = cam; }

    void PlotWidget3D::setViewPort(const Range32f &x, const Range32f &y, const Range32f &z) {
      m_data->givenViewport[0] = x; m_data->givenViewport[1] = y; m_data->givenViewport[2] = z;
      std::scoped_lock lock(m_data->scene);
      m_data->recompute();
    }

    void PlotWidget3D::add(std::shared_ptr<Node> obj) { m_data->add(obj, true); }
    void PlotWidget3D::remove(Handle h) { std::scoped_lock l(m_data->scene); m_data->rootObject->removeChild(h); }
    void PlotWidget3D::color(int r, int g, int b, int a) { m_data->color = GeomColor(r, g, b, a); }
    void PlotWidget3D::fill(int r, int g, int b, int a) { m_data->fill = GeomColor(r, g, b, a); }
    void PlotWidget3D::nocolor() { m_data->color[3] = 0; }
    void PlotWidget3D::nofill() { m_data->fill[3] = 0; }
    void PlotWidget3D::smoothfill(bool on) { m_data->smoothfill = on; }
    void PlotWidget3D::pointsize(float s) { m_data->pointsize = s; }
    void PlotWidget3D::linewidth(float w) { m_data->linewidth = w; }
    void PlotWidget3D::lock() { m_data->scene.lock(); }
    void PlotWidget3D::unlock() { m_data->scene.unlock(); }
    void PlotWidget3D::clear() { std::scoped_lock l(m_data->scene); m_data->rootObject->removeAllChildren(); }

    PlotWidget3D::Handle PlotWidget3D::scatter(const std::vector<Vec> &points) {
      auto m = std::make_shared<MeshNode>();
      for (const Vec &p : points) m->addVertex(p, white);
      m_data->add(m, true);
      return m.get();
    }

    PlotWidget3D::Handle PlotWidget3D::scatter(const std::vector<Vec> &points,
                                               const std::vector<GeomColor> &colors,
                                               const Range32f &colorRange) {
      ICLASSERT_THROW(points.size() == colors.size(),
                      ICLException("PlotWidget3D::scatter: points.size() must equal colors.size()"));
      auto m = std::make_shared<MeshNode>();
      // viz3d vertex colours are 0..255 → map the given colour range there
      LinearTransform1D t(colorRange, Range32f(0, 255));
      const bool identity = (colorRange == Range32f(0, 255));
      for (size_t i = 0; i < points.size(); ++i) {
        const GeomColor &c = colors[i];
        m->addVertex(points[i], identity ? c
                     : GeomColor(t(c[0]), t(c[1]), t(c[2]), t(c[3])));
      }
      m->setPointSize(m_data->pointsize);
      m_data->add(m, false);
      return m.get();
    }

    PlotWidget3D::Handle PlotWidget3D::linestrip(const std::vector<Vec> &points) {
      auto m = std::make_shared<MeshNode>();
      for (const Vec &p : points) m->addVertex(p, white);
      for (size_t i = 1; i < points.size(); ++i) m->addLine(i-1, i, white);
      m_data->add(m, true);
      return m.get();
    }

    static std::shared_ptr<MeshNode> makeGrid(int nx, int ny, const std::vector<Vec> &pts) {
      auto m = std::make_shared<MeshNode>();
      for (const Vec &p : pts) m->addVertex(p, white);
      for (int y = 0; y < ny-1; ++y)
        for (int x = 0; x < nx-1; ++x) {
          const int a = x + nx*y, b = a+1, c = a+nx+1, d = a+nx;
          m->addQuad(a, b, c, d);
          m->addLine(a, b, white); m->addLine(a, d, white);
        }
      return m;
    }

    PlotWidget3D::Handle PlotWidget3D::surf(const std::vector<Vec> &points, int nx, int ny) {
      auto grid = makeGrid(nx, ny, points);
      grid->createAutoNormals(m_data->smoothfill);
      m_data->add(grid, true);
      return grid.get();
    }

    PlotWidget3D::Handle PlotWidget3D::surf(std::function<float(float,float)> fxy,
                                            const Range32f &rx, const Range32f &ry,
                                            int nx, int ny, Handle reuseObject) {
      std::vector<Vec> points(nx*ny);
      const float dx = rx.getLength() / (nx-1), dy = ry.getLength() / (ny-1);
      for (int y = 0; y < ny; ++y) {
        const float yv = ry.minVal + y*dy;
        for (int x = 0; x < nx; ++x) {
          const float xv = rx.minVal + x*dx;
          points[x + nx*y] = Vec(xv, yv, fxy(xv, yv), 1);
        }
      }
      auto *mesh = dynamic_cast<MeshNode*>(reuseObject);
      if (mesh) {
        std::scoped_lock l(m_data->scene);
        mesh->getVertices() = points;
        mesh->createAutoNormals(m_data->smoothfill);
        m_data->recompute();
        return mesh;
      }
      return surf(points, nx, ny);
    }

    PlotWidget3D::Handle PlotWidget3D::label(const Vec &p, const std::string &text) {
      auto t = TextNode::create(text, 0.1f, m_data->color[3] ? m_data->color : white);
      t->translate(p[0], p[1], p[2]);
      m_data->add(t, false);
      return t.get();
    }

    namespace {  // GUI component registration
      struct Plot3DGUIWidget : public GUIWidget {
        PlotWidget3D *draw;
        Plot3DGUIWidget(const Plot3D &c, const CreateContext &ctx)
          : GUIWidget(c, ctx, GUIWidget::gridLayout, Size(16, 12)) {
          draw = new PlotWidget3D(this);
          draw->setViewPort(c.xrange, c.yrange, c.zrange);
          addToGrid(draw);
          if (!c.options().handle.empty()) {
            getGUI()->lockData();
            getGUI()->allocValue<PlotHandle3D>(c.options().handle, PlotHandle3D(draw, this));
            getGUI()->unlockData();
          }
        }
      };

      struct Plot3DRegisterer {
        Plot3DRegisterer() { utils::AssignRegistry::enroll_identity<PlotHandle3D>(); }
      } plot3DRegisterer;
    }

    GUIWidget *Plot3D::createWidget(const CreateContext &ctx) const {
      return new Plot3DGUIWidget(*this, ctx);
    }

  } // namespace viz3d
} // namespace icl
