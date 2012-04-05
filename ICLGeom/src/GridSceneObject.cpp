/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/GridSceneObject.cpp                        **
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

#include <ICLGeom/GridSceneObject.h>

namespace icl{
  void GridSceneObject::init(int nXCells, int nYCells, const std::vector<Vec> &allGridPoints, bool lines, bool quads) throw (ICLException){
    this->nXCells = nXCells;
    this->nYCells = nYCells;
    
    ICLASSERT_THROW((int)allGridPoints.size() == nXCells*nYCells,
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
                                   bool lines, bool quads) throw (ICLException){
    init(nXCells,nYCells,allGridPoints,lines,quads);
  }
    
  GridSceneObject::GridSceneObject(int nXCells, int nYCells, const Vec &origin, const Vec &dx, const Vec &dy,
                                   bool lines, bool quads)throw (ICLException){
    std::vector<Vec> allGridPoints; 
    allGridPoints.reserve(nXCells*nYCells);
    for(int x=0;x<nXCells;++x){
      for(int y=0;y<nYCells;++y){
        allGridPoints.push_back(origin+dx*float(x)+dy*float(y));
      }
    }
    init(nXCells,nYCells,allGridPoints,lines,quads);
  }

  

}
