// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLGeom/ComplexCoordinateFrameSceneObject.h>
#include <mutex>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::math;
using namespace icl::qt;

namespace icl::geom {
    ComplexCoordinateFrameSceneObject::ComplexCoordinateFrameSceneObject(float axisLength,float axisThickness, bool withLabels,
                                                                          const std::string &xLabel,
                                                                          const std::string &yLabel,
                                                                          const std::string &zLabel):
      xLabel(xLabel),yLabel(yLabel),zLabel(zLabel){
      float al[3] = { axisLength, axisLength, axisLength };
      setParams(al,axisThickness,withLabels);
    }

    ComplexCoordinateFrameSceneObject::ComplexCoordinateFrameSceneObject(float axisLengths[3],float axisThickness,
                                                                         bool withXYZLabels,
                                                                         const std::string &xLabel,
                                                                         const std::string &yLabel,
                                                                         const std::string &zLabel,
                                                                         const GeomColor &xAxisColor,
                                                                         const GeomColor &yAxisColor,
                                                                         const GeomColor &zAxisColor,
                                                                         const GeomColor &textLabelColor,
                                                                         float textScaling):
      xLabel(xLabel),yLabel(yLabel),zLabel(zLabel){
      setParams(axisLengths,axisThickness,withXYZLabels,xAxisColor, yAxisColor, zAxisColor, textLabelColor, textScaling);
    }


    void ComplexCoordinateFrameSceneObject::setParams(float axisLengths[3], float axisThickness, bool withLabels,
                                                      const GeomColor &xAxisColor, const GeomColor &yAxisColor,
                                                      const GeomColor &zAxisColor, const GeomColor &textLabelColor,
                                                      float textScaling){
      std::lock_guard<std::recursive_mutex> lock(mutex);

      m_vertices.clear();
      m_normals.clear();
      m_vertexColors.clear();
      m_primitives.clear();
      m_children.clear();

      const float d=axisThickness;
      const float p = M_PI_2;
      const float rxs[3]={0,-p,0},rys[3]={p,0,0},rzs[3]={0,0,0};
      const GeomColor cs[]={xAxisColor, yAxisColor, zAxisColor };
      for(int i=0;i<3;++i){
        float l = axisLengths[i];
        SceneObject *o = addCylinder(0,0,l/2, d,d,l, 20);
        o->rotate(rxs[i],rys[i],rzs[i]);
        o->setVisible(Primitive::line,false);
        o->setVisible(Primitive::vertex,false);
        o->setColor(Primitive::quad,cs[i]);
        o->setColor(Primitive::polygon,cs[i]);

        o = addCone(0,0,l, 2*d,2*d,3*d, 20);
        o->rotate(rxs[i],rys[i],rzs[i]);
        o->setVisible(Primitive::line,false);
        o->setVisible(Primitive::vertex,false);
        o->setColor(Primitive::triangle,cs[i]);
        o->setColor(Primitive::polygon,cs[i]);
      }

      float dFac = (xLabel.length() == 1 && yLabel.length() == 1 && zLabel.length() == 1) ? 3 : 5;
      addVertex(Vec(axisLengths[0]+dFac*d,0,0,1),GeomColor(0,0,0,0));
      addVertex(Vec(0,axisLengths[1]+dFac*d,0,1),GeomColor(0,0,0,0));
      addVertex(Vec(0,0,axisLengths[2]+dFac*d,1),GeomColor(0,0,0,0));

      if(withLabels){
        addText(0,xLabel,textScaling * axisThickness*3, textLabelColor);
        addText(1,yLabel,textScaling * axisThickness*3, textLabelColor);
        addText(2,zLabel,textScaling * axisThickness*3, textLabelColor);
      }
    }

  } // namespace icl::geom