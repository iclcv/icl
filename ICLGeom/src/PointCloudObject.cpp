/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/PointCloudObject.cpp                       **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
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

#include <ICLGeom/PointCloudObject.h>

namespace icl{
  namespace geom{
  
    PointCloudObject::PointCloudObject(int width, int height, bool organized):
      m_organized(organized){
      if(organized){
        m_dim2D = Size(width,height);
      }else{
        m_dim2D = Size(width,1);
      }
      m_vertices.resize(m_dim2D.getDim(),Vec(0,0,0,1));
      m_vertexColors.resize(m_dim2D.getDim(),Vec(0,0,0,1));
    }
    
    bool PointCloudObject::supports(FeatureType t) {
      return t == RGBA32f || t == XYZ;
    }
    
    bool PointCloudObject::isOrganized() const{
      return m_organized;
    }
  
    Size PointCloudObject::getSize() const throw (ICLException){
      if(!isOrganized()) throw ICLException("SimplePointCloudObject:getSize(): instance is not 2D-ordered");
      return m_dim2D;
    }
    
    int PointCloudObject::getDim() const{
      return m_dim2D.getDim();
    }
  
    DataSegment<float,3> PointCloudObject::selectXYZ(){
      return DataSegment<float,3>(&m_vertices[0][0],4*sizeof(float),m_vertices.size(),m_dim2D.width);  
    }
    DataSegment<float,4> PointCloudObject::selectRGBA32f(){
      return DataSegment<float,4>(&m_vertexColors[0][0],4*sizeof(float),m_vertexColors.size(),m_dim2D.width);  
    }
    
    void PointCloudObject::customRender() {}
  
    void PointCloudObject::setSize(const Size &size){
      m_vertices.resize(size.getDim(),Vec(0,0,0,1));
      m_vertexColors.resize(size.getDim(),Vec(0,0,0,1));
      m_organized = (size.height > 0);
    }
  
  } // namespace geom
}
