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
    std::fill(m_visible,m_visible+Primitive::PRIMITIVE_TYPE_COUNT,true);
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

  void Object2::addTexture(int a, int b, int c, int d, const Img8u &texture,bool deepCopy){
    m_primitives.push_back(Primitive(a,b,c,d,texture,deepCopy));
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

  Object2::Object2(const std::string &type,const float *params){
    std::fill(m_visible,m_visible+5,true);
    if(type == "cube"){
      float x = *params++;
      float y = *params++;
      float z = *params++;
      float k = *params/2.0;

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
      
      addQuad(0,1,2,3,GeomColor(0,100,120,155));//
      addQuad(7,6,5,4,GeomColor(0,100,140,155)); // ?
      addQuad(0,3,7,4,GeomColor(0,100,160,155));//
      addQuad(5,6,2,1,GeomColor(0,100,180,155)); // ?
      addQuad(4,5,1,0,GeomColor(0,100,200,155));
      addQuad(3,2,6,7,GeomColor(0,100,220,155));

    }else if(type == "cuboid"){
      float x = *params++;
      float y = *params++;
      float z = *params++;
      float dx = *params++/2.0;
      float dy = *params++/2.0;
      float dz = *params++/2.0;

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
      
      // Vertex order: alwas counter clock-wise
      addQuad(0,1,2,3,GeomColor(0,100,120,155));//
      addQuad(7,6,5,4,GeomColor(0,100,140,155)); // ?
      addQuad(0,3,7,4,GeomColor(0,100,160,155));//
      addQuad(5,6,2,1,GeomColor(0,100,180,155)); // ?
      addQuad(4,5,1,0,GeomColor(0,100,200,155));
      addQuad(3,2,6,7,GeomColor(0,100,220,155));
      
    }else if(type == "sphere"){
      float x = *params++;
      float y = *params++;
      float z = *params++;
      float r = *params++/2.0;
      int slices = (int)*params++;
      int steps = (int)*params;
      
      float dA = (2*M_PI)/slices;
      float dB = (2*M_PI)/steps;

      Vec v(r,0,0,1),offs(x,y,z),zAxis(0,0,1),yAxis(0,1,0);
      int idx = 0;
      
      for(int i=0;i<slices;++i){
        //float a = i*dA/2.0;//-M_PI/2.0;
        Vec v2 = rotate_vector(zAxis,i*dA,v);
        for(int j=0;j<steps;++j,++idx){
          addVertex(offs+rotate_vector(yAxis,j*dB,v2));
          if(i>0 && j>0){
            addLine(idx,idx-1);
            addLine(idx,idx-slices);
          }
        }
      }
    }else{
      ERROR_LOG("unknown type:" << type);
    }
  }
  
}
