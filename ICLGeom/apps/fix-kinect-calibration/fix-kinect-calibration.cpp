// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

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
