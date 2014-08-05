/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/demos/point-cloud-viewer/fix-kinect-calibration.cpp**
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

#include <ICLQt/Common.h>
#include <ICLGeom/Camera.h>
#include <fstream>

int main(int n, char **ppc){
  pa_init(n,ppc,
          "[m]-di(depthCamInFile) "
          "[m]-ci(colorCamInfile) "
          "[m]-do(depthCamOutFile) "
          "[m]-co(colorCamOutFile)");

  Camera d(*pa("-di"));
  Camera c(*pa("-ci"));
  
  Size res = d.getResolution();
  PlaneEquation pe(c.getPosition(), c.getNorm());
  ViewRay vr = d.getViewRay(Point(res.width/2, res.height/2));
  Vec p = vr.getIntersection(pe);
  
  Vec pos = d.getPosition();
  pos[3] = 1;
  d.setPosition(p);
  Vec delta = p - pos;
  
  Vec cPos = c.getPosition();
  cPos += delta;
  cPos[3] = 1;
  c.setPosition(cPos);

  
  std::ofstream cos((*pa("-co")).c_str());
  cos << c;

  std::ofstream dos((*pa("-do")).c_str());
  dos << d;

  
}

