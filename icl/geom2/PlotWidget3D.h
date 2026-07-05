// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/DrawWidget3D.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Node.h>
#include <icl/cv3d/GeomDefs.h>
#include <icl/utils/Range.h>
#include <functional>
#include <vector>

namespace icl::geom2 {

  /// 3D box-plot widget (geom2 reimplementation of geom::PlotWidget3D).
  /** A self-contained ICLDrawWidget3D that owns a Scene2, a camera, a scaled
      root GroupNode, and a coordinate-frame box with tics/labels. Data added via
      scatter/surf/linestrip/label is parented under the root node, whose
      transform maps the data view-port into the [-1,1]^3 cube so everything fits
      the box. The view-port can be fixed (setViewPort) or grow with the data. */
  class ICLGeom2_API PlotWidget3D : public qt::ICLDrawWidget3D {
    struct Data;
    Data *m_data;

  public:
    using Handle = Node*;

    PlotWidget3D(QWidget *parent = 0);
    ~PlotWidget3D();

    void setViewPort(const utils::Range32f &xrange,
                     const utils::Range32f &yrange,
                     const utils::Range32f &zrange);
    const utils::Range32f *getViewPort() const;

    Scene2 &getScene();
    const Scene2 &getScene() const;

    Node *getRootObject();
    const Node *getRootObject() const;

    const geom::Camera &getCamera() const;
    void setCamera(const geom::Camera &cam);

    void add(std::shared_ptr<Node> obj);
    void remove(Handle h);

    void color(int r, int g, int b, int a);
    void nocolor();
    void fill(int r, int g, int b, int a);
    void nofill();
    void smoothfill(bool on);
    void pointsize(float size);
    void linewidth(float width);
    void lock();
    void unlock();
    void clear();

    Handle scatter(const std::vector<geom2::Vec> &points);
    Handle scatter(const std::vector<geom2::Vec> &points,
                   const std::vector<geom::GeomColor> &colors,
                   const utils::Range32f &colorRange = utils::Range32f(0, 255));
    Handle linestrip(const std::vector<geom2::Vec> &points);
    Handle surf(const std::vector<geom2::Vec> &points, int nx, int ny);
    Handle surf(std::function<float(float, float)> fxy,
                const utils::Range32f &rx = utils::Range32f(0, 0),
                const utils::Range32f &ry = utils::Range32f(0, 0),
                int nx = 100, int ny = 100, Handle reuseObj = 0);
    Handle label(const geom2::Vec &p, const std::string &text);
  };

} // namespace icl::geom2
