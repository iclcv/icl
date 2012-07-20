/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/PointCloudObjectBase.cpp                   **
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

#include <ICLGeom/PointCloudObjectBase.h>
#include <ICLQt/GLFragmentShader.h>

#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glew.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

namespace icl{

  void PointCloudObjectBase::customRender(){
    if(!supports(XYZ)) return;

    const DataSegment<float,3> xyz = selectXYZ(); 

    glDisable(GL_LIGHTING);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, xyz.stride, xyz.data);
    
    size_t numElements = xyz.numElements;

    static GLFragmentShader swapRB( "void main(){\n"
                                    "  gl_FragColor = vec4(gl_Color[2],gl_Color[1],gl_Color[0],gl_Color[3]);\n"
                                    "}\n");

    if(supports(RGBA32f)){
      const DataSegment<icl32f,4> rgba = selectRGBA32f(); 
      glEnableClientState(GL_COLOR_ARRAY);
      glColorPointer(4, GL_FLOAT, rgba.stride, rgba.data);
      numElements = iclMin(numElements,rgba.numElements);
    }else if(supports(BGR)){
      const DataSegment<icl8u,3> bgr = selectBGR(); 
      glEnableClientState(GL_COLOR_ARRAY);
      glColorPointer(3, GL_UNSIGNED_BYTE, bgr.stride, bgr.data);
      numElements = iclMin(numElements,bgr.numElements);
      swapRB.activate();
    }else if(supports(BGRA)){
      const DataSegment<icl8u,4> bgra = selectBGRA(); 
      glEnableClientState(GL_COLOR_ARRAY);
      glColorPointer(4, GL_UNSIGNED_BYTE, bgra.stride, bgra.data);
      numElements = iclMin(numElements,bgra.numElements);
      swapRB.activate();
    }
#if 0
    else if(supports(Intensity)){
      const DataSegment<float,1> rgba = selectIntensity(); 
      glEnableClientState(GL_COLOR_ARRAY);
      glColorPointer(3, GL_UNSIGNED_BYTE, rgb.padding, rgb.data);
      numElements = iclMin(numElements,rgba.numElements);
    }
#endif
    else{
      glColor3f(0,100/255.,255/255.);

    }
    
    glDrawArrays(GL_POINTS, 0, numElements);

    swapRB.deactivate();


    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glEnable(GL_LIGHTING);
  }
  
  std::map<std::string,std::string> & PointCloudObjectBase::getMetaData(){
      return m_metaData;
  }
  
  const std::map<std::string,std::string> & PointCloudObjectBase::getMetaData() const{
    return m_metaData;
  }
  
  const std::string & PointCloudObjectBase::getMetaData(const std::string &key) const throw (ICLException){
    std::map<std::string,std::string>::const_iterator it = m_metaData.find(key);
    if(it == m_metaData.end()) throw ICLException("PointCloudObjectBase::getMetaData(key): no meta data with given key ("
                                                   + key + ") was associated with this point cloud");
    return it->second;
  }
  
  
  bool PointCloudObjectBase::hasMetaData(const std::string &key) const{
    return m_metaData.find(key) != m_metaData.end();
  }
  
  
  bool PointCloudObjectBase::hasAnyMetaData() const{
    return m_metaData.size();
  }
    
  void PointCloudObjectBase::setMetaData(const std::string &key, const std::string &value){
    m_metaData[key] = value;
  }
  
  void PointCloudObjectBase::clearAllMetaData(){
    m_metaData.clear();
  }
  
  void PointCloudObjectBase::clearMetaData(const std::string &key){
    std::map<std::string,std::string>::iterator it = m_metaData.find(key);
    if(it != m_metaData.end()){
      m_metaData.erase(it);
    }
  }
  
  std::vector<std::string> PointCloudObjectBase::getAllMetaDataEntries() const{
    std::vector<std::string> all(m_metaData.size());
    int i = 0;
    for(std::map<std::string,std::string>::const_iterator it = m_metaData.begin();
        it != m_metaData.end(); ++it){
      all[i++] = it->first;
    }
    return all;
  }

  std::ostream &operator<<(std::ostream &s, const PointCloudObjectBase::FeatureType t){
    static std::string names[PointCloudObjectBase::NUM_FEATURES] = {
      "Intensity","Label", "BGR", "BGRA", "BGRA32s", "XYZ", "Normal", "RGBA32f"
    };
    if( (unsigned int)t < (int)PointCloudObjectBase::NUM_FEATURES){
      return s << names[(int)t];
    }else{
      return s << "*unknown feature type*";
    }
  }

}
