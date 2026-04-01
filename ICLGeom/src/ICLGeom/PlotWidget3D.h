// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/DrawWidget3D.h>
#include <ICLGeom/Scene.h>
#include <functional>


namespace icl{
  namespace geom{

    class ICLGeom_API PlotWidget3D : public qt::ICLDrawWidget3D{
      struct Data;
      Data *m_data;

      public:

      using Handle = SceneObject*;

      PlotWidget3D(QWidget *parent=0);
      ~PlotWidget3D();

      void setViewPort(const utils::Range32f &xrange,
                       const utils::Range32f &yrange,
                       const utils::Range32f &zrange);

      const utils::Range32f *getViewPort() const;

      Scene &getScene();
      const Scene &getScene() const;

      SceneObject *getRootObject();
      const SceneObject *getRootObject() const;

      const Camera &getCamera() const;
      void setCamera(const Camera &cam);

      void add(SceneObject *obj, bool passOwnerShip=true);
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

      Handle scatter(const std::vector<Vec> &points);

      Handle scatter(const std::vector<Vec> &points,
                     const std::vector<GeomColor> &colors,
                     const utils::Range32f &colorRange=utils::Range32f(0,255));

      Handle linestrip(const std::vector<Vec> &points);

      Handle surf(const std::vector<Vec> &points, int nx, int ny);

      Handle surf(std::function<float(float,float)> fxy,
                  const utils::Range32f &rx=utils::Range32f(0,0),
                  const utils::Range32f &ry=utils::Range32f(0,0),
                  int nx=100, int ny=100, Handle reuseObj=0);

      Handle label(const Vec &p, const std::string &text);
    };
  }
}
