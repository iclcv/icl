/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/SceneLightObject.cpp               **
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


#include <ICLGeom/SceneLightObject.h>

using namespace icl;
using namespace icl::utils;

namespace icl{
  namespace geom{
    
    class SceneLightObject::ThreadPart : public SceneObject{
      public:
      
      virtual void prepareForRendering(){
        glEnable(GL_LIGHTING);
      }

      ThreadPart(){
        const float size = 1;
        const float x = 0;
        const float y = 0;
        const float z = 0;
        const float rx = size;
        const float ry=rx,rz=rx;
        const int na = 15;
        const int nb = 50;
        
        const float dAlpha = 2*M_PI/na;
        const float dBeta = M_PI/(nb-1);
        for(int j=0;j<nb;++j){
          for(int i=0;i<na;++i){
            const float alpha = i*dAlpha; //float(i)/na * 2 * M_PI;
            const float beta = j*dBeta; //float(j)/nb * M_PI;
            
            const float sa = sin(alpha), ca = cos(alpha);
            const float sb = sin(beta), cb = cos(beta);
            
            float rel = float(j)/(float(nb)-1);
            float scaleXY = (0.8 + 0.2*sqr(sin(24*rel)))*(rel == 1 ? 0 : 
                                                          rel > 0.5 ? pow(sb,1./3) : 
                                                          rel < 0.5 ? pow(sb,1./5) :
                                                          1);
            
            addVertex(Vec(x+rx*ca*scaleXY,
                          y+ry*sa*scaleXY,
                          z+rz*cb,1),
                      geom_blue(200));
            
            if(j){
              if( j != (nb-1)){
                addLine(i+na*j, i ? (i+na*j-1) : (na-1)+na*j);
              }            
              if(j){
                int a = i+na*j;
                int b = i ? (i+na*j-1) : (na-1)+na*j;
                int c = i ? (i+na*(j-1)-1) : (na-1)+na*(j-1);
                int d = i+na*(j-1);
                if(j == nb-1){
                  addQuad(a,d,c,b);
                }else{
                  addQuad(d,c,b,a);
                }
              }
              
            }
            if(j) addLine(i+na*j, i+na*(j-1));
          }
        }
      }
    };
    
    static float get_scale_factor_xy(float rel, float sb){
      if(rel < 0.5) return sb;
      // rel [0.5,1]
      rel = (rel-0.5)*2; 
      // rel[0,1]
      return sqr(cos(rel*1.5)) + (rel/1.7)*(rel/1.7);
    }
    static float get_scale_factor_z(float rel, float cb){
      if(rel < 0.5) return cb;
      // rel [0.5,1]
      rel = (rel-0.5)*2; 
      // rel[0,1]
      return 2 * pow(rel,0.2) * cb;
    }
  
    void SceneLightObject::prepareForRendering(){
      glDisable(GL_LIGHTING);
      if(!m_hasText){
        m_hasText = true;
        addVertex(Vec(0,0,-3.0,1));
        addText(m_vertices.size()-1, "light "+str(m_lightID), 0.05);
      }
    }
    
    SceneLightObject::SceneLightObject(Scene *scene, int lightID):
      m_scene(scene),m_lightID(lightID), m_hasText(false){
      const float size = 1;
      
      SceneObject *thread = new ThreadPart;
      thread->translate(0,0,-4*size);
      thread->scale(0.53,0.53,0.53);

      addChild(thread);
      

      const float x = 0;
      const float y = 0;
      const float z = 0;
      const float rx = size;
      const float ry=rx,rz=rx;
      const int na = 15;
      const int nb = 15;
    
      const float dAlpha = 2*M_PI/na;
      const float dBeta = M_PI/(nb-1);
      for(int j=0;j<nb;++j){
        for(int i=0;i<na;++i){
          const float alpha = i*dAlpha; //float(i)/na * 2 * M_PI;
          const float beta = j*dBeta; //float(j)/nb * M_PI;
          
          const float sa = sin(alpha), ca = cos(alpha);
          const float sb = sin(beta), cb = cos(beta);
          
          float rel = float(j)/(float(nb)-1);
          float scaleXY = get_scale_factor_xy(rel,sb);
          float scaleZ = get_scale_factor_z(rel,cb);
          
          addVertex(Vec(x+rx*ca*scaleXY,
                        y+ry*sa*scaleXY,
                        z+rz*scaleZ,1),
                    geom_blue(200));
          if(j){
            if( j != (nb-1)){
              addLine(i+na*j, i ? (i+na*j-1) : (na-1)+na*j);
            }            
            if(j){
              int a = i+na*j;
              int b = i ? (i+na*j-1) : (na-1)+na*j;
              int c = i ? (i+na*(j-1)-1) : (na-1)+na*(j-1);
              int d = i+na*(j-1);
              if(j == nb-1){
                addQuad(a,d,c,b);
              }else{
                addQuad(d,c,b,a);
              }
            }
            
          }
          if(j) addLine(i+na*j, i+na*(j-1));
        }
      }
      
      setColor(Primitive::quad,GeomColor(255,240,230,255));
      setVisible(Primitive::line,false);
      setVisible(Primitive::vertex,false);

      createAutoNormals();
      
      thread->setColor(Primitive::quad,GeomColor(250,250,250,255));
      thread->setVisible(Primitive::line,false);
      thread->setShininess(255);
      thread->createAutoNormals();

    }
  }
}
