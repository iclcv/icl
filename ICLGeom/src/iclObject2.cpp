#include <iclObject2.h>

namespace icl{
  const std::vector<Vec> &Object2::getVertices() const { 
    return m_vertices; 
  }
  std::vector<Vec> &Object2::getVertices() { 
    return m_vertices; 
  }

  const std::vector<Primitive> &Object2::getPrimitives() const { 
    return m_primitives; 
  }
  std::vector<Primitive> &Object2::getPrimitives() { 
    return m_primitives; 
  }
  
  void Object2::setVisible(Primitive::Type t, bool visible) {
    m_visible[t] = visible;
  }
    
  bool Object2::isVisible(Primitive::Type t) const {
    return m_visible[t];
  }
  
  Object2::Object2(){
    std::fill(m_visible,m_visible+5,true);
  }
  
  void Object2::addVertex(const Vec &p, const GeomColor &color){
    m_vertices.push_back(p);
    m_vertexColors.push_back(color);
  }
  
  void Object2::addLine(int a, int b, const GeomColor &color){
    m_primitives.push_back(Primitive(a,b,color));
  }
    
  void Object2::addTriangle(int a, int b, int c, const GeomColor &color){
    m_primitives.push_back(Primitive(a,b,c,color));
  }
  
  void Object2::addQuad(int a, int b, int c, int d, const GeomColor &color){
    m_primitives.push_back(Primitive(a,b,c,d,color));
  }

  Object2 *Object2::copy() const{
    return new Object2(*this);
  }

  void Object2::updateZFromPrimitives(){
    m_z = 0;
    if(m_primitives.size()){
      for(unsigned int i=0;i<m_primitives.size();++i){
        m_z += m_primitives[i].z;
      }
      m_z /= m_primitives.size();
    }
  }

  void Object2::setColor(Primitive::Type t,const GeomColor &color){
    for(unsigned int i=0;i<m_primitives.size();++i){
      if(m_primitives[i].type == t){
        m_primitives[i].color = color;
      }
    }
  }

  Object2::Object2(const std::string &type, float *params){
    std::fill(m_visible,m_visible+5,true);
    if(type == "cube"){
      float x = *params++;
      float y = *params++;
      float z = *params++;
      float k = *params/2;

      addVertex(Vec(x+k,y-k,z+k,1));
      addVertex(Vec(x+k,y+k,z+k,1));
      addVertex(Vec(x-k,y+k,z+k,1));
      addVertex(Vec(x-k,y-k,z+k,1));
      
      addVertex(Vec(x+k,y-k,z-k,1));
      addVertex(Vec(x+k,y+k,z-k,1));
      addVertex(Vec(x-k,y+k,z-k,1));
      addVertex(Vec(x-k,y-k,z-k,1));
    
      addLine(0,1);
      addLine(1,2);
      addLine(2,3);
      addLine(3,0);
      
      addLine(4,5);
      addLine(5,6);
      addLine(6,7);
      addLine(7,4);
      
      addLine(0,4);
      addLine(1,5);    
      addLine(2,6);
      addLine(3,7);
      
      addQuad(0,1,2,3,GeomColor(0,100,120,155));
      addQuad(4,5,6,7,GeomColor(0,100,140,155));
      addQuad(0,3,7,4,GeomColor(0,100,160,155));
      addQuad(1,2,6,5,GeomColor(0,100,180,155));
      addQuad(0,1,5,4,GeomColor(0,100,200,155));
      addQuad(3,2,6,7,GeomColor(0,100,220,155));
    }else if(type == "cuboid"){
      float x = *params++;
      float y = *params++;
      float z = *params++;
      float dx = *params++;
      float dy = *params++;
      float dz = *params++;

      addVertex(Vec(x+dx,y-dy,z+dz,1));
      addVertex(Vec(x+dx,y+dy,z+dz,1));
      addVertex(Vec(x-dx,y+dy,z+dz,1));
      addVertex(Vec(x-dx,y-dy,z+dz,1));
      
      addVertex(Vec(x+dx,y-dy,z-dz,1));
      addVertex(Vec(x+dx,y+dy,z-dz,1));
      addVertex(Vec(x-dx,y+dy,z-dz,1));
      addVertex(Vec(x-dx,y-dy,z-dz,1));
    
      addLine(0,1);
      addLine(1,2);
      addLine(2,3);
      addLine(3,0);
      
      addLine(4,5);
      addLine(5,6);
      addLine(6,7);
      addLine(7,4);
      
      addLine(0,4);
      addLine(1,5);    
      addLine(2,6);
      addLine(3,7);
      
      addQuad(0,1,2,3,GeomColor(0,100,120,155));
      addQuad(4,5,6,7,GeomColor(0,100,140,155));
      addQuad(0,3,7,4,GeomColor(0,100,160,155));
      addQuad(1,2,6,5,GeomColor(0,100,180,155));
      addQuad(0,1,5,4,GeomColor(0,100,200,155));
      addQuad(3,2,6,7,GeomColor(0,100,220,155));
    }else{
      ERROR_LOG("unknown type:" << type);
    }
  }
  
}
