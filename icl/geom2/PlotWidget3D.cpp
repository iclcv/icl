// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/PlotWidget3D.h>
#include <icl/geom2/Plot3D.h>
#include <icl/geom2/PlotHandle3D.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/GeometryNode.h>
#include <icl/geom2/TextNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom/Material.h>
#include <icl/geom/Camera.h>
#include <icl/qt/GUIWidget.h>
#include <icl/math/transform/LinearTransform1D.h>
#include <icl/utils/dispatch/AssignRegistry.h>
#include <algorithm>

namespace icl {
  using namespace utils;
  using namespace math;
  using namespace core;
  using namespace qt;
  using geom::Camera;
  using geom::Material;
  using geom::GeomColor;

  namespace geom2 {

    static const GeomColor white(255, 255, 255, 255);

    static Range32f round_range(Range32f r) {
      if (r.minVal > r.maxVal) std::swap(r.minVal, r.maxVal);
      float m = std::fabs(r.maxVal - r.minVal), f = 1;
      if (m > 1) { while (m / f > 100) f *= 10; }
      else       { while (m / f < 10)  f *= 0.1f; }
      r.minVal = std::floor(r.minVal / f) * f;
      r.maxVal = std::ceil(r.maxVal / f) * f;
      return r;
    }

    static std::string create_label(float r) {
      return str(std::fabs(r) < 0.0000001f ? 0 : r);
    }

    // one axis (tic marks + numeric labels + axis name) in the local [-1,1] X
    // range; the caller rotates/translates it into place on the box.
    static std::shared_ptr<GroupNode> makeAxis(const Range32f &range, bool invertLabels,
                                               const std::string &name) {
      auto g = std::make_shared<GroupNode>();
      const Range32f rr = round_range(range);
      const float mn = rr.minVal, mx = rr.maxVal;
      const int N = 10;
      const float step = (mx - mn) / N;
      const float lenBase = 0.1f, d = 0.1f;

      auto ticks = std::make_shared<MeshNode>();
      for (int i = -N/2, l = 0; i <= N/2; ++i, ++l) {
        const float r = float(i) / (N/2);
        const float len = i ? lenBase : 2*lenBase;
        const int base = (int)ticks->getVertices().size();
        ticks->addVertex(Vec(r, 0, 0, 1), white);
        ticks->addVertex(Vec(r, len, 0, 1), white);
        ticks->addVertex(Vec(r, 0, len, 1), white);
        ticks->addLine(base, base+1, white);
        ticks->addLine(base, base+2, white);
        auto t = TextNode::create(create_label(mn + l*step), 0.08f, white);
        t->translate(invertLabels ? -r : r, -d, 0);
        g->addChild(t);
      }
      ticks->setPrimitiveVisible(PrimVertex, false);
      g->addChild(ticks);

      auto nameT = TextNode::create(name, 0.12f, white);
      nameT->translate((invertLabels ? -1 : 1) * (1 + 2*d), 0, 0);
      g->addChild(nameT);
      return g;
    }

    struct PlotWidget3D::Data {
      Scene2 scene;
      Range32f givenViewport[3];
      Range32f computedViewport[3];

      std::shared_ptr<GroupNode> rootObject;       // scaled data container
      std::shared_ptr<GroupNode> coordinateFrame;  // box + axes ([-1,1] space)
      std::shared_ptr<MeshNode>  box;
      std::shared_ptr<GroupNode> axes[3];
      Range32f frameRanges[3];

      float pointsize = 1, linewidth = 1;
      bool smoothfill = true;
      GeomColor color = geom::geom_red(255), fill = geom::geom_blue(255);

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
      }

      void updateTics() {
        if (frameRanges[0] == computedViewport[0] &&
            frameRanges[1] == computedViewport[1] &&
            frameRanges[2] == computedViewport[2]) return;
        std::copy(computedViewport, computedViewport+3, frameRanges);
        static const std::string names[3] = { "X", "Y", "Z" };
        for (int i = 0; i < 3; ++i) {
          if (axes[i]) coordinateFrame->removeChild(axes[i].get());
          axes[i] = makeAxis(computedViewport[i], i == 1, names[i]);
          coordinateFrame->addChild(axes[i]);
        }
        axes[0]->translate(0, -1, -1);
        axes[1]->rotate(0, 0, M_PI/2); axes[1]->translate(-1, 0, -1);
        axes[2]->rotate(-M_PI/2, 0, 0); axes[2]->rotate(0, M_PI/2, 0); axes[2]->translate(-1, -1, 0);
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

    PlotWidget3D::PlotWidget3D(QWidget *parent) : ICLDrawWidget3D(parent), m_data(new Data) {
      std::fill(m_data->givenViewport, m_data->givenViewport+3, Range32f(0, 0));
      std::fill(m_data->frameRanges, m_data->frameRanges+3, Range32f(1, 1));  // force first retic
      m_data->scene.setBounds(5);

      m_data->scene.addCamera(Camera::lookAt(Vec(8, 1.5, 1.5, 1), Vec(0, 0, 0, 1),
                                             Vec(0, 0, -1, 1), Size(640, 480), 30.0f));
      install(m_data->scene.getMouseHandler(0));
      link(m_data->scene.getGLCallback(0).get());

      m_data->rootObject = std::make_shared<GroupNode>();
      m_data->coordinateFrame = std::make_shared<GroupNode>();

      // box wireframe (8 corners of the [-1,1]^3 cube)
      m_data->box = std::make_shared<MeshNode>();
      const float c[8][3] = {{ 1,-1, 1},{ 1, 1, 1},{-1, 1, 1},{-1,-1, 1},
                             { 1,-1,-1},{ 1, 1,-1},{-1, 1,-1},{-1,-1,-1}};
      for (auto &p : c) m_data->box->addVertex(Vec(p[0], p[1], p[2], 1), white);
      for (int i = 0; i < 4; ++i) {
        m_data->box->addLine(i, (i+1)%4, white);
        m_data->box->addLine(4+i, 4+(i+1)%4, white);
        m_data->box->addLine(i, i+4, white);
      }
      m_data->box->setPrimitiveVisible(PrimVertex, false);
      m_data->coordinateFrame->addChild(m_data->box);

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
      // geom2 vertex colours are 0..255 → map the given colour range there
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

  } // namespace geom2
} // namespace icl
