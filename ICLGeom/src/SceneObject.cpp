/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/SceneObject.cpp                            **
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

#include <ICLGeom/SceneObject.h>
#include <fstream>
#include <ICLIO/File.h>
#include <ICLUtils/StringUtils.h>

namespace icl{
  const std::vector<Vec> &SceneObject::getVertices() const { 
    return m_vertices; 
  }
  std::vector<Vec> &SceneObject::getVertices() { 
    return m_vertices; 
  }

  const std::vector<Primitive> &SceneObject::getPrimitives() const { 
    return m_primitives; 
  }
  std::vector<Primitive> &SceneObject::getPrimitives() { 
    return m_primitives; 
  }
  
  void SceneObject::setVisible(Primitive::Type t, bool visible, bool recursive) {
    m_visible[t] = visible;
    if(recursive){
      for(unsigned int i=0;i<m_children.size();++i){
        m_children[i]->setVisible(t,visible);
      }
    }
  }
    
  bool SceneObject::isVisible(Primitive::Type t) const {
    return m_visible[t];
  }
  
  SceneObject::SceneObject():
    m_normalMode(AutoNormals),
    m_lineColorsFromVertices(false),
    m_triangleColorsFromVertices(false),
    m_quadColorsFromVertices(false),
    m_pointSize(1),
    m_lineWidth(1),
    m_useSmoothShading(true),
    m_transformation(Mat::id()),
    m_hasTransformation(false),
    m_parent(0)
  {

    std::fill(m_visible,m_visible+Primitive::PRIMITIVE_TYPE_COUNT,true);
  }
  
  void SceneObject::addVertex(const Vec &p, const GeomColor &color, const Vec &normal){
    m_vertices.push_back(p);
    m_vertexColors.push_back(color);
    if(m_normalMode == NormalsPerVertex){
      m_normals.push_back(normal);
    }
  }
  
  void SceneObject::addLine(int a, int b, const GeomColor &color, const Vec &normal){
    m_primitives.push_back(Primitive(a,b,color));
    if(m_normalMode == NormalsPerFace) m_normals.push_back(normal);
  }
    
  void SceneObject::addTriangle(int a, int b, int c, const GeomColor &color, const Vec &normal){
    m_primitives.push_back(Primitive(a,b,c,color));
    if(m_normalMode == NormalsPerFace) m_normals.push_back(normal);
  }
  
  void SceneObject::addQuad(int a, int b, int c, int d, const GeomColor &color, const Vec &normal){
    m_primitives.push_back(Primitive(a,b,c,d,color));
    if(m_normalMode == NormalsPerFace) m_normals.push_back(normal);
  }

  void SceneObject::addPolygon(const std::vector<int> &vertexIndices, 
                               const GeomColor &color, const Vec &normal){
    m_primitives.push_back(Primitive(vertexIndices,color));
    if(m_normalMode == NormalsPerFace) m_normals.push_back(normal);
  }

  void SceneObject::addTexture(int a, int b, int c, int d, const Img8u &texture,bool deepCopy){
    m_primitives.push_back(Primitive(a,b,c,d,texture,deepCopy));
  }
  
  void SceneObject::addTextTexture(int a, int b, int c, int d, const std::string &text,
                                   const GeomColor &color,int textSize, bool holdTextAR){
#warning holdTextAR is not supported yet
    m_primitives.push_back(Primitive(a,b,c,d,text,color,textSize));
  }

  SceneObject *SceneObject::copy() const{
    return new SceneObject(*this);
  }

  void SceneObject::setColor(Primitive::Type t,const GeomColor &color, bool recursive){
    for(unsigned int i=0;i<m_primitives.size();++i){
      if(m_primitives[i].type == t){
        m_primitives[i].color = color;
      }
    }
    if(recursive){
      for(unsigned int i=0;i<m_children.size();++i){
        m_children[i]->setColor(t,color);
      }
    }
  }

  SceneObject::SceneObject(const std::string &type,const float *params):
    m_lineColorsFromVertices(false),
    m_triangleColorsFromVertices(false),
    m_quadColorsFromVertices(false),
    m_polyColorsFromVertices(false),
    m_pointSize(1),
    m_lineWidth(1),
    m_useSmoothShading(true),
    m_transformation(Mat::id()),
    m_hasTransformation(false),
    m_parent(0)
  {
    std::fill(m_visible,m_visible+5,true);

    if(type == "cuboid" || type == "cube"){
      m_normalMode = NormalsPerFace;

      float x = *params++;
      float y = *params++;
      float z = *params++;
      float dx = *params++/2.0;
      float dy = dx;
      float dz = dx;

      if(type == "cuboid"){
        dy = *params++/2.0;
        dz = *params++/2.0;
      }

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
      addQuad(0,1,2,3,GeomColor(0,100,255,200),Vec(0,0,1,1));//
      addQuad(7,6,5,4,GeomColor(0,100,255,200),Vec(0,0,-1,1)); // ?
      addQuad(0,3,7,4,GeomColor(0,100,255,200),Vec(0,-1,0,1));//
      addQuad(5,6,2,1,GeomColor(0,100,255,200),Vec(0,1,0,1)); // ?
      addQuad(4,5,1,0,GeomColor(0,100,255,200),Vec(1,0,0,1));
      addQuad(3,2,6,7,GeomColor(0,100,255,200),Vec(-1,0,0,1));
      
    }else if(type == "sphere" || type == "spheroid"){
      m_normalMode = NormalsPerVertex;
      float x = *params++;
      float y = *params++;
      float z = *params++;
      float rx = *params++;
      float ry=rx,rz=rx;
      if(type == "spheroid"){
        ry = *params++;
        rz = *params++;
      }
      int na = *params++;
      int nb = *params++;

      const float dAlpha = 2*M_PI/na;
      const float dBeta = M_PI/(nb-1);
      for(int j=0;j<nb;++j){
        for(int i=0;i<na;++i){
          float alpha = i*dAlpha; //float(i)/na * 2 * M_PI;
          float beta = j*dBeta; //float(j)/nb * M_PI;
          
          addVertex(Vec(x+rx*cos(alpha)*sin(beta),
                        y+ry*sin(alpha)*sin(beta),
                        z+rz*cos(beta),1),
                    geom_blue(200),
                    normalize3(Vec(cos(alpha)*sin(beta),
                                   sin(alpha)*sin(beta),
                                   cos(beta))));
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
    }else{
      ERROR_LOG("unknown type:" << type);
    }
  }

  int count_slashes(const std::string &s){
    int n = 0;
    for(unsigned int i=0;i<s.length();++i){
      if(s[i] == '/') ++n;
    }
    return n;
  }

  // A: f v1 v2 v3 .. 
  // B: f v1/vt1 v2/vt2 v3/vt3 .. with texture coordinate
  // C: f v1/vt1/n1 ..            with texture and normal index
  // D: f v1//n1 ..               with normal indices only
  char get_format(const std::string &s, int lineForError){
    int n = count_slashes(s);
    switch(n){
      case 0: return 'A';
      case 1: return 'B';
      case 2: return tok(s,"/").size() == 3 ? 'C' : 'D';
      default:
        throw ICLException(".obj parsing error in line " 
                           + str(lineForError) 
                           + " ( invalid face index: \"" + s +"\")");
    }
    return 'x';
  }


  bool SceneObject::getSmoothShading() const{
    return m_useSmoothShading;
  }

  void  SceneObject::setSmoothShading(bool on, bool recursive){
    m_useSmoothShading = on;
    if(recursive){
      for(unsigned int i=0;i<m_children.size();++i){
        m_children[i]->setSmoothShading(on);
      }
    }
  }
  
  SceneObject::SceneObject(const std::string &objFileName) throw (ICLException):
    m_normalMode(AutoNormals),
    m_lineColorsFromVertices(false),
    m_triangleColorsFromVertices(false),
    m_quadColorsFromVertices(false),
    m_polyColorsFromVertices(false),
    m_pointSize(1),
    m_lineWidth(1),
    m_transformation(Mat::id()),
    m_hasTransformation(false),
    m_parent(0)
  {
    File file(objFileName,File::readText);
    if(!file.exists()) throw ICLException("Error in SceneObject(objFilename): unable to open file " + objFileName);
    
    setSmoothShading(true);
    
    typedef FixedColVector<float,3> F3;
    typedef FixedColVector<int,3> I3;
    
    int nSkippedVT = 0;
    int nSkippedVN = 0;
    int nSkippedO = 0;
    int nSkippedG = 0;
    int nSkippedS = 0;
    int nSkippedMTLLIB = 0;
    int nSkippedUSEMTL = 0;
    
    int lineNr=0;
    
    while(file.hasMoreLines()){
      ++lineNr;
      std::string line = file.readLine();
      if(line.length()<2) continue;
      
      else if(line[0] == 'v'){ // most common: vertex
        switch(line[1]){
          case ' ':
            m_vertices.push_back(parse<F3>(line.substr(2)).resize<1,4>(1));

            break;
          case 't': // texture coordinates u,v,[w] (w is optional)
            ++nSkippedVT;
            break;
          case 'n': // normal for vertex x,y,z (might not be unit!)
            ++nSkippedVN;
            break;
          default:
            ERROR_LOG("skipping line " + str(lineNr) + ":\"" + line + "\" [unknown format]");
            break;
        }
      }else if(line[0] == 'f'){ 
        // A: f v1 v2 v3 .. 
        // B: f v1/vt1 v2/vt2 v3/vt3 .. with texture coordinate
        // C: f v1/vt1/n1 ..            with texture and normal index
        // D: f v1//n1 ..               with normal indices only
        const std::vector<std::string> x = tok(line.substr(2)," ");
        if(!x.size()) {
          ERROR_LOG("skipping line " + str(lineNr) + ":\"" + line + "\" [face definition expected]" );   
          continue;
        }
        char C = get_format(x[0], lineNr); // we assume, that the format is the same here
        int n = (int)x.size();
        if( n < 3 ){
          ERROR_LOG("skipping line " + str(lineNr) + ":\"" + line + "\" [unsupported number of face vertices]" );   
          continue;
        }
        switch(C){
          case 'A':
            if(n == 3){
              addTriangle(parse<int>(x[0])-1,parse<int>(x[1])-1,parse<int>(x[2])-1);
            }else if(n==4){
              addQuad(parse<int>(x[0])-1,parse<int>(x[1])-1,parse<int>(x[2])-1,parse<int>(x[3])-1);
            }else{
              std::vector<int> xx(x.size());
              for(unsigned int i=0;i<x.size();++i){
                xx[i] = parse<int>(x[i])-1;
              }
              addPolygon(xx);
            }
            break;
          case 'B': // for now, this is simple, we simple dont use the 2nd and 3rd token;
          case 'C':
          case 'D':{
            std::vector<int> is(n);
            for(int i=0;i<n;++i){
              is[i] = parseVecStr<int>(x[i],"/").at(0);
            }
            if(n == 3){
              addTriangle(is[0]-1,is[1]-1,is[2]-1);
            }else if (n == 4){
              addQuad(is[0]-1,is[1]-1,is[2]-1,is[3]-1);
            }else{
              std::vector<int> xx(x.size());
              for(unsigned int i=0;i<x.size();++i){
                xx[i] = parse<int>(x[i])-1;
              }
              addPolygon(xx);
            }
          }
        }
      }else if(line[0] == '#') { 
        continue;
      }else if(line[0] == 's') { 
        ++nSkippedS;
        continue;
      }else if(line[0] == 'o'){
        ++nSkippedO;
        continue;
      }else if(line[0] == 'g'){
        ++nSkippedG;
        continue;
      }else if(!line.find("#")) {
        continue; // comment
      }else if(!line.find("usemtl")) {
        ++nSkippedUSEMTL;
        continue; // todo try to load material description
      }else if(!line.find("mtllib")){
        ++nSkippedMTLLIB;
        continue;
      }else{
        ERROR_LOG("skipping line " + str(lineNr) + ":\"" + line + "\" [unknown format]" );   
        continue;
      }
    }
    setVisible(Primitive::line,true);
    setVisible(Primitive::triangle,true);
    setVisible(Primitive::quad,true);
    setVisible(Primitive::polygon,true);
  }

  void SceneObject::setColorsFromVertices(Primitive::Type t, bool on, bool recursive){
    switch(t){
      case Primitive::line:
        m_lineColorsFromVertices = on;
        break;
      case Primitive::triangle:
        m_triangleColorsFromVertices = on;
        break;
      case Primitive::quad:
        m_quadColorsFromVertices = on;
        break;
      case Primitive::polygon:
        m_polyColorsFromVertices = on;
        break;
      default:
        ERROR_LOG("this operations is only supported for line, triangle and quad primitive types");
        break;
    }
    if(recursive){
      for(unsigned int i=0;i<m_children.size();++i){
        m_children[i]->setColorsFromVertices(t,on);
      }
    }
      
  }

  void SceneObject::setTransformation(const Mat &m){
    m_transformation = m;
    m_hasTransformation = true;
  }
    
  void SceneObject::removeTransformation(){
    m_transformation = Mat::id();
    m_hasTransformation = false;
  }
  
  void SceneObject::transform(const Mat &m){
    m_transformation = m*m_transformation;
    m_hasTransformation = true;
  }
  
  void SceneObject::rotate(float rx, float ry, float rz){
    transform(create_hom_4x4<float>(rx,ry,rz));
  }
  
  void SceneObject::translate(float dx, float dy, float dz){
    transform(create_hom_4x4<float>(0,0,0,dx,dy,dz));
  }

  void SceneObject::scale(float sx, float sy, float sz){
    transform(Mat(sx,0,0,0,
                  0,sy,0,0,
                  0,0,sz,0,
                  0,0,0,1));
  }
  
  Mat SceneObject::getTransformation(bool relative) const{
    if(relative || !getParent()) return m_transformation;
    return getParent()->getTransformation() * m_transformation;
  }
    
  /// returns whether the SceneObject has currently a non-ID-transformation
  bool SceneObject::hasTransformation(bool relative) const{
    if(relative || !getParent()) return m_hasTransformation;
    return m_hasTransformation || getParent()->hasTransformation();
  }
    
  /// returns the parent scene object
  SceneObject *SceneObject::getParent(){
    return m_parent;
  }

  const SceneObject *SceneObject::getParent() const{
    return m_parent;
  }

    
  void SceneObject::addChild(SceneObject *child, bool passOwnerShip){
    m_children.push_back(SmartPtr<SceneObject>(child,passOwnerShip));
    child->m_parent = this;
  }
    
  void SceneObject::removeChild(SceneObject *child){
    for(unsigned int i=0;i<m_children.size();++i){
      if(m_children[i].get() == child){
        m_children[i]->m_parent = 0;
        m_children.erase(m_children.begin()+i);
        return;
      }
    }
  }
  
  void SceneObject::removeAllChildren(){
    m_children.clear();
  }
  
  bool SceneObject::hasChildren() const{
    return m_children.size();
  }
  
  int SceneObject::getChildCount() const{
    return (int)m_children.size();
  }
  

  SceneObject *SceneObject::getChild(int index){
    if(index < 0 || index >= (int)m_children.size()) return 0;
    return m_children[index].get();
  }

  void SceneObject::setPointSize(float pointSize, bool recursive){
    m_pointSize = pointSize;
    if(recursive){
      for(unsigned int i=0;i<m_children.size();++i){
        m_children[i]->setPointSize(pointSize);
      }
    }
  }
  
  void SceneObject::setLineWidth(float lineWidth, bool recursive){
    m_lineWidth = lineWidth;
    if(recursive){
      for(unsigned int i=0;i<m_children.size();++i){
        m_children[i]->setLineWidth(lineWidth);
      }
    }
  
  }
  SceneObject &SceneObject::operator=(const SceneObject &other){
    if(this == &other) return *this;
#define DEEP_COPY(X) X = other.X
#define DEEP_COPY_2(X,Y) DEEP_COPY(X); DEEP_COPY(Y)
#define DEEP_COPY_4(X,Y,A,B) DEEP_COPY_2(X,Y); DEEP_COPY_2(A,B)
    DEEP_COPY(m_normalMode);
    DEEP_COPY_2(m_vertices,m_vertexColors);
    DEEP_COPY_4(m_primitives,m_lineColorsFromVertices,m_triangleColorsFromVertices,m_quadColorsFromVertices);
    DEEP_COPY_4(m_polyColorsFromVertices,m_pointSize,m_lineWidth,m_useSmoothShading);
    DEEP_COPY_2(m_transformation,m_hasTransformation);
#undef DEEP_COPY
#undef DEEP_COPY_2
#undef DEEP_COPY_4
    std::copy(other.m_visible,other.m_visible+(int)Primitive::PRIMITIVE_TYPE_COUNT,m_visible);
    m_children.clear();
    m_children.resize(other.m_children.size());
    for(unsigned int i=0;i<other.m_children.size();++i){
      m_children[i] = SmartPtr<SceneObject>(other.m_children[i]->copy());
    }
    return *this;
  }

  SceneObject *SceneObject::addCuboid(float x, float y, float z, float dx, float dy, float dz){
    float params[] = {x,y,z,dx,dy,dz};
    SceneObject *o = new SceneObject("cuboid",params);
    addChild(o);
    return o;
  }

  SceneObject *SceneObject::addSpheroid(float x, float y, float z, float rx, float ry, float rz, int rzSteps, int xySlices){
    float params[] = {x,y,z,rx,ry,rz,rzSteps,xySlices};
    SceneObject *o = new SceneObject("spheroid",params);
    addChild(o);
    return o;
  }

  SceneObject::NormalMode SceneObject::getNormalMode() const{
    return m_normalMode;
  }
  void SceneObject::setNormalMode(NormalMode mode){
    m_normalMode = mode;
  }


}
