/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ComplexCoordinateFrameSceneObject.cpp      **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/
#include <ICLGeom/ComplexCoordinateFrameSceneObject.h>

namespace icl{
  ComplexCoordinateFrameSceneObject::ComplexCoordinateFrameSceneObject(float axisLength,float axisThickness){
    setParams(axisLength,axisThickness);
  }
  
  void ComplexCoordinateFrameSceneObject::setParams(float axisLength, float axisThickness){
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
#ifdef HAVE_OPENGL
    addText(0,"x");
    addText(1,"y");
    addText(2,"z");
#endif
  }

}
