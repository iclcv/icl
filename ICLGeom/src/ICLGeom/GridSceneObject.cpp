// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLGeom/GridSceneObject.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::math;
using namespace icl::qt;

namespace icl{
  namespace geom{
    void GridSceneObject::init(int nXCells, int nYCells, const std::vector<Vec> &allGridPoints, bool lines, bool quads){
      this->nXCells = nXCells;
      this->nYCells = nYCells;

      ICLASSERT_THROW(static_cast<int>(allGridPoints.size()) == nXCells*nYCells,
                      ICLException("GridSceneObject::Constructor: nXCells*nYCells differs from allGridPoints.size()!"));
      for(int i=0;i<nXCells*nYCells;++i){
        addVertex(allGridPoints[i]);
        m_vertices.back()[3]=1;
      }
      for(int x=1;x<nXCells;++x){
        for(int y=1;y<nYCells;++y){
          int a = getIdx(x,y);
          int b = getIdx(x-1,y);
          int c = getIdx(x,y-1);
          int d = getIdx(x-1,y-1);
          if(lines) {
            addLine(a,b);
              addLine(a,c);
          }
          if(quads){
            addQuad(a,b,d,c);
          }
        }
      }
      for(int x=1;x<nXCells;++x){
        if(lines) addLine(getIdx(x,0),getIdx(x-1,0));
      }
      for(int y=1;y<nYCells;++y){
        if(lines) addLine(getIdx(0,y),getIdx(0,y-1));
      }
    }

    GridSceneObject::GridSceneObject(int nXCells, int nYCells, const std::vector<Vec> &allGridPoints,
                                     bool lines, bool quads){
      init(nXCells,nYCells,allGridPoints,lines,quads);
    }

    GridSceneObject::GridSceneObject(int nXCells, int nYCells, const Vec &origin, const Vec &dx, const Vec &dy,
                                     bool lines, bool quads){
      std::vector<Vec> allGridPoints;
      allGridPoints.reserve(nXCells*nYCells);
      for(int y=0;y<nYCells;++y){
        for(int x=0;x<nXCells;++x){
          allGridPoints.push_back(origin+dx*float(x)+dy*float(y));
        }
      }
      init(nXCells,nYCells,allGridPoints,lines,quads);
    }



  } // namespace geom
}
