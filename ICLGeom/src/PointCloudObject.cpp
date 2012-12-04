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

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;


namespace icl{
  namespace geom{

    PointCloudObject::PointCloudObject(int numPoints, bool withNormals, bool withColors):
      m_organized(false){
      m_dim2D = Size(numPoints,1);
      m_vertices.resize(m_dim2D.getDim(),Vec(0,0,0,1));
      m_hasNormals = withNormals;
      m_hasColors = withColors;

      if(withColors){
        m_vertexColors.resize(m_dim2D.getDim(),Vec(0,0,0,1));
      }
      if(withNormals){
        m_normals.resize(m_dim2D.getDim(),Vec(0,0,0,1));
      }
    }
  
    PointCloudObject::PointCloudObject(int width, int height, bool organized, bool withNormals, bool withColors):
      m_organized(organized){
      if(organized){
        m_dim2D = Size(width,height);
      }else{
        m_dim2D = Size(width,1);
      }
      m_vertices.resize(m_dim2D.getDim(),Vec(0,0,0,1));

      m_hasNormals = withNormals;
      m_hasColors = withColors;
      
      if(m_hasColors){
        m_vertexColors.resize(m_dim2D.getDim(),Vec(0,0,0,1));
      }
      if(withNormals){
        m_normals.resize(m_dim2D.getDim(),Vec(0,0,0,1));
      }
    }
    
    bool PointCloudObject::supports(FeatureType t) const{
      if(t == Normal && m_hasNormals) return true;
      else if ( t == RGBA32f && m_hasColors) return true;
      else return (t == XYZ) || (t == XYZH);
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

    DataSegment<float,4> PointCloudObject::selectXYZH(){
      return DataSegment<float,4>(&m_vertices[0][0],4*sizeof(float),m_vertices.size(),m_dim2D.width);  
    }

    DataSegment<float,4> PointCloudObject::selectNormal(){
      if(m_hasNormals){
        return DataSegment<float,4>(&m_normals[0][0],4*sizeof(float),m_normals.size(),m_dim2D.width);  
      }else{
        return error<float,4>(__FUNCTION__);        
      }
    }

    DataSegment<float,4> PointCloudObject::selectRGBA32f(){
      if(m_hasColors){
        return DataSegment<float,4>(&m_vertexColors[0][0],4*sizeof(float),m_vertexColors.size(),m_dim2D.width);  
      }else{
        return error<float,4>(__FUNCTION__);        
      }
    }
    
    void PointCloudObject::customRender() {
      drawNormalLines();
    }
  
    void PointCloudObject::setSize(const Size &size){
      m_organized = (size.height > 0);
      m_dim2D = size;
      m_vertices.resize(size.getDim(),Vec(0,0,0,1));
      
      if(m_hasColors){
        m_vertexColors.resize(size.getDim(),Vec(0,0,0,1));
      }
      if(m_hasNormals){
        m_normals.resize(size.getDim(),Vec(0,0,0,1));
      }
    }
  
  } // namespace geom
}
