/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/OctreeObject.cpp                   **
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
#include <ICLGeom/OctreeObject.h>

namespace icl{
  namespace geom{


    void octree_object_render_box(float x0, float y0, float z0,
                                  float x1, float y1, float z1){
      glColor4f(0,1,0,0.25);
      glBegin(GL_LINES);

      //top
      glVertex3f(x0,y0,z0);
      glVertex3f(x1,y0,z0);

      glVertex3f(x1,y0,z0);
      glVertex3f(x1,y1,z0);

      glVertex3f(x1,y1,z0);
      glVertex3f(x0,y1,z0);

      glVertex3f(x0,y1,z0);
      glVertex3f(x0,y0,z0);

      // downwards
      glVertex3f(x0,y0,z0);
      glVertex3f(x0,y0,z1);

      glVertex3f(x1,y0,z0);
      glVertex3f(x1,y0,z1);

      glVertex3f(x0,y1,z0);
      glVertex3f(x0,y1,z1);

      glVertex3f(x1,y1,z0);
      glVertex3f(x1,y1,z1);

      // bottom
      glVertex3f(x0,y0,z1);
      glVertex3f(x1,y0,z1);

      glVertex3f(x1,y0,z1);
      glVertex3f(x1,y1,z1);

      glVertex3f(x1,y1,z1);
      glVertex3f(x0,y1,z1);

      glVertex3f(x0,y1,z1);
      glVertex3f(x0,y0,z1);
      glEnd();
    }
  }
}
