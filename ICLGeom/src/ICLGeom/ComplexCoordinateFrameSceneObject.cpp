/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ComplexCoordinateFrameSceneObject. **
**          cpp                                                    **
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

#include <ICLGeom/ComplexCoordinateFrameSceneObject.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::math;
using namespace icl::qt;

#ifdef ICL_SYSTEM_WINDOWS
#define M_PI_2 2*3.14159265358979323846
#endif

namespace icl{
  namespace geom{
    ComplexCoordinateFrameSceneObject::ComplexCoordinateFrameSceneObject(float axisLength,float axisThickness, bool withLabels){
      setParams(axisLength,axisThickness,withLabels);
    }
    
    void ComplexCoordinateFrameSceneObject::setParams(float axisLength, float axisThickness, bool withLabels){
      Mutex::Locker lock(mutex);
      
      m_vertices.clear();
      m_normals.clear();
      m_vertexColors.clear();
      m_primitives.clear();
      m_children.clear();
      
      const float l=axisLength, d=axisThickness;
      const float p = M_PI_2;
      const float rxs[3]={0,p,0},rys[3]={p,0,0},rzs[3]={0,0,0};
      const GeomColor cs[]={geom_red(),geom_green(),geom_blue() };
      for(int i=0;i<3;++i){
        SceneObject *o = addCylinder(0,0,i==2 ? l/2 : -l/2,d,d,l,20);
        o->rotate(rxs[i],rys[i],rzs[i]);
        o->setVisible(Primitive::line,false);
        o->setVisible(Primitive::vertex,false);
        o->setColor(Primitive::quad,cs[i]);
        o->setColor(Primitive::polygon,cs[i]);
        switch(i){
          case 0:
            o = addCone(0,0,l,2*d,2*d,3*d,20);
            o->rotate(M_PI,0,0);
            break;
          case 1:
            o = addCone(0,0,l,2*d,2*d,3*d,20);
            o->rotate(M_PI,0,0);
            break;
          case 2:
            o = addCone(0,0,l,2*d,2*d,3*d,20);
            o->rotate(0,0,0);
            break;
        }
        
        o->rotate(rxs[i],rys[i],rzs[i]);
        o->setVisible(Primitive::line,false);
        o->setVisible(Primitive::vertex,false);
        o->setColor(Primitive::triangle,cs[i]);
        o->setColor(Primitive::polygon,cs[i]);
      }
      
      addVertex(Vec(l+3*d,0,0,1),GeomColor(0,0,0,0));
      addVertex(Vec(0,l+3*d,0,1),GeomColor(0,0,0,0));
      addVertex(Vec(0,0,l+3*d,1),GeomColor(0,0,0,0));
  
      if(withLabels){
        addText(0,"x",axisThickness*3);
        addText(1,"y",axisThickness*3);
        addText(2,"z",axisThickness*3);
      }
    }
  
  } // namespace geom
}
