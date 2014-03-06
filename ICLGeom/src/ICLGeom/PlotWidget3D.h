/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PlotWidget3D.h                     **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/DrawWidget3D.h>
#include <ICLGeom/Scene.h>
#include <ICLUtils/Function.h>


namespace icl{
  namespace geom{

    class ICLGeom_API PlotWidget3D : public qt::ICLDrawWidget3D{
      struct Data;
      Data *m_data;

      public:

      typedef SceneObject* Handle;

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

      Handle surf(utils::Function<float,float,float> fxy,
                  const utils::Range32f &rx=utils::Range32f(0,0),
                  const utils::Range32f &ry=utils::Range32f(0,0),
                  int nx=100, int ny=100, Handle reuseObj=0);

      Handle label(const Vec &p, const std::string &text);
    };
  }
}
