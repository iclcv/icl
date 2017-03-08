/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PointCloudObject.cpp               **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
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

#include <ICLGeom/PointCloudObject.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;


namespace icl{
  namespace geom{

    PointCloudObject::PointCloudObject(bool withNormals, bool withColors, bool withLabels, bool withDepth):
      m_organized(false){
      m_dim2D = Size(0,0);
      m_hasNormals = withNormals;
      m_hasColors = withColors;
      m_hasLabels = withLabels;
      m_hasDepth = withDepth;
      
      setLockingEnabled(true);
    }

    PointCloudObject::PointCloudObject(int numPoints, bool withNormals, bool withColors, bool withLabels, bool withDepth):
      m_organized(false){
      m_dim2D = Size(numPoints,1);
      m_vertices.resize(m_dim2D.getDim(),Vec(0,0,0,1));
      m_hasNormals = withNormals;
      m_hasColors = withColors;
      m_hasLabels = withLabels;
      m_hasDepth = withDepth;

      setLockingEnabled(true);

      if(withColors){
        m_vertexColors.resize(m_dim2D.getDim(),Vec(0,0,0,1));
      }
      if(withNormals){
        m_normals.resize(m_dim2D.getDim(),Vec(0,0,0,1));
      }
      if(withLabels){
        m_labels.resize(m_dim2D.getDim(),0);
      }
      if(withDepth){
        m_depth.resize(m_dim2D.getDim(),0);
      }
    }
  
    PointCloudObject::PointCloudObject(int width, int height, bool organized, bool withNormals, bool withColors, bool withLabels, bool withDepth):
      m_organized(organized){
      if(organized){
        m_dim2D = Size(width,height);
      }else{
        m_dim2D = Size(width,1);
      }
      m_vertices.resize(m_dim2D.getDim(),Vec(0,0,0,1));

      m_hasNormals = withNormals;
      m_hasColors = withColors;
      m_hasLabels = withLabels;
      m_hasDepth = withDepth;

      setLockingEnabled(true);
      
      if(m_hasColors){
        m_vertexColors.resize(m_dim2D.getDim(),Vec(0,0,0,1));
      }
      if(withNormals){
        m_normals.resize(m_dim2D.getDim(),Vec(0,0,0,1));
      }
      if(withLabels){
        m_labels.resize(m_dim2D.getDim(),0);
      }
      if(withDepth){
        m_depth.resize(m_dim2D.getDim(),0);
      }
    }
    
    bool PointCloudObject::supports(FeatureType t) const{
      if(t == Normal && m_hasNormals) return true;
      else if(t == Label && m_hasLabels) return true;
      else if(t == RGBA32f && m_hasColors) return true;
      else if(t == Depth && m_hasDepth) return true;
      else return (t == XYZ) || (t == XYZH);
    }

    bool PointCloudObject::canAddFeature(FeatureType t) const{
      return t == Normal || t == RGBA32f || t == Label || t == Depth;
    }
    
    void PointCloudObject::addFeature(FeatureType t) throw (utils::ICLException){
      if(!canAddFeature(t)){
        PointCloudObjectBase::addFeature(t);
        return;
      }
      lock();
      
      if(t == Normal && ! m_hasNormals){
        m_hasNormals = true;
        m_normals.resize(getDim());
      }else if(t == RGBA32f &&!m_hasColors){
        m_hasColors = true;
        m_vertexColors.resize(getDim());
      }else if(t == Label && !m_hasLabels){
        m_hasLabels = true;
        m_labels.resize(getDim());
      }else if(t == Depth && !m_hasDepth){
        m_hasDepth = true;
        m_depth.resize(getDim());
      }
      unlock();
    }
    
    
    bool PointCloudObject::isOrganized() const{
      return m_organized;
    }
  
    Size PointCloudObject::getSize() const throw (ICLException){
      if(!isOrganized()) throw ICLException("SimplePointCloudObject:getSize(): instance is not 2D-ordered");
      return m_dim2D;
    }
    
    int PointCloudObject::getDim() const{
      // we need abs here, since un-organized point clouds would have
      // a negative dim otherwise ...
      return abs(m_dim2D.getDim());
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

    DataSegment<icl32s,1> PointCloudObject::selectLabel(){
      if(m_hasLabels){
        return DataSegment<icl32s,1>(&m_labels[0], sizeof(icl32s), m_labels.size(), m_dim2D.width);
      }else{
        return error<icl32s,1>(__FUNCTION__);
      }
    }

    DataSegment<float,1> PointCloudObject::selectDepth(){
      if(m_hasDepth){
        return DataSegment<float,1>(&m_depth[0], sizeof(float), m_labels.size(), m_dim2D.width);
      }else{
        return error<float,1>(__FUNCTION__);
      }
    }
    
    
    void PointCloudObject::customRender() {
      if(m_useCustomRender){
        PointCloudObjectBase::customRender();
      }else{
        drawNormalLines();
      }
    }
  
    void PointCloudObject::setSize(const Size &size){
      lock();
      //      SHOW(size);
      m_organized = (size.height > 0);
      m_dim2D = size;
      const size_t len = m_organized ? size.getDim() : size.width;
      m_vertices.resize(len,Vec(0,0,0,1));
      
      if(m_hasColors){
        m_vertexColors.resize(len,Vec(0,0,0,1));
      }
      if(m_hasNormals){
        m_normals.resize(len,Vec(0,0,0,1));
      }
      if(m_hasLabels){
        m_labels.resize(len, 0);
      }
      if(m_hasDepth){
        m_depth.resize(len,0);
      }
      unlock();
    }
  
  } // namespace geom
}
