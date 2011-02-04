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
  
  void SceneObject::addVertex(const Vec &p, const GeomColor &color){
    m_vertices.push_back(p);
    m_vertexColors.push_back(color);
  }
  
  /// adds a new normal to this object
  void SceneObject::addNormal(const Vec &n){
    m_normals.push_back(n);
  }

  void SceneObject::addLine(int a, int b, int na, int nb, const GeomColor &color){
    if(na>=0 && nb>=0){
      m_primitives.push_back(Primitive(a,b,color,na,nb));
    }else{
      m_primitives.push_back(Primitive(a,b,color));
    }
  }
    
  void SceneObject::addTriangle(int a, int b, int c, int na, int nb, int nc, const GeomColor &color){
    if(na>=0 && nb>=0 && nc>=0){
      m_primitives.push_back(Primitive(a,b,c,color,na,nb,nc));
    }else{
      m_primitives.push_back(Primitive(a,b,c,color));    
    }
  }
  
  void SceneObject::addQuad(int a, int b, int c, int d, int na, int nb, int nc, int nd, const GeomColor &color){
    if(na>=0 && nb>=0 && nc>=0 && nd>=0){
      m_primitives.push_back(Primitive(a,b,c,d,color,na,nb,nc,nd));
    }else{
      m_primitives.push_back(Primitive(a,b,c,d,color));
    }
  }

  void SceneObject::addPolygon(const std::vector<int> &vertexIndices, 
                               const std::vector<int> &normalIndices, 
                               const GeomColor &color){
    ICLASSERT_RETURN(!normalIndices.size() || (vertexIndices.size() == normalIndices.size()));
    if(normalIndices.size()){
      m_primitives.push_back(Primitive(vertexIndices,color,normalIndices));
    }else{
      m_primitives.push_back(Primitive(vertexIndices,color));
    }
  }

  void SceneObject::addTexture(int a, int b, int c, int d,const Img8u &texture, int na, int nb, int nc, int nd, bool deepCopy){
    if(na>=0 && nb>=0 && nc>=0 && nd>=0){
      m_primitives.push_back(Primitive(a,b,c,d,texture,deepCopy,interpolateLIN,na,nb,nc,nd));
    }else{
      m_primitives.push_back(Primitive(a,b,c,d,texture,deepCopy,interpolateLIN));
    }
  }
  
  void SceneObject::addTextTexture(int a, int b, int c, int d, const std::string &text,
                                   const GeomColor &color,
                                   int na, int nb, int nc, int nd,
                                   int textSize, bool holdTextAR){
#warning holdTextAR is not supported yet
    if(na>=0 && nb>=0 && nc>=0 && nd>=0){
      m_primitives.push_back(Primitive(a,b,c,d,text,color,textSize,interpolateLIN,na,nb,nc,nd));
    }else{
      m_primitives.push_back(Primitive(a,b,c,d,text,color,textSize,interpolateLIN));
    }
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

  void SceneObject::setTextureInterpolation(scalemode mode, bool recursive) throw (ICLException){
    if(mode != interpolateLIN && mode != interpolateNN){
      throw ICLException("SceneObject::setTextureInterpolation invalid interpolation mode");
    }
    for(unsigned int i=0;i<m_primitives.size();++i){
      m_primitives[i].mode = mode;
    }
    if(recursive){
      for(unsigned i=0;i<m_children.size();++i){
        m_children[i]->setTextureInterpolation(mode);
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
    
      addNormal(Vec(0,0,1,1));
      addNormal(Vec(0,0,-1,1));
      addNormal(Vec(0,-1,0,1));
      addNormal(Vec(0,1,0,1));
      addNormal(Vec(1,0,0,1));
      addNormal(Vec(-1,0,0,1));
      
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
      addQuad(0,1,2,3,0,0,0,0,GeomColor(0,100,255,200));//
      addQuad(7,6,5,4,1,1,1,1,GeomColor(0,100,255,200)); // ?
      addQuad(0,3,7,4,2,2,2,2,GeomColor(0,100,255,200));//
      addQuad(5,6,2,1,3,3,3,3,GeomColor(0,100,255,200)); // ?
      addQuad(4,5,1,0,4,4,4,4,GeomColor(0,100,255,200));
      addQuad(3,2,6,7,5,5,5,5,GeomColor(0,100,255,200));
      
    }else if(type == "sphere" || type == "spheroid"){
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
                    geom_blue(200));
          addNormal(normalize3(Vec(cos(alpha)*sin(beta),
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
                addQuad(a,d,c,b,
                        a,d,c,b);
              }else{
                addQuad(d,c,b,a,
                        d,c,b,a);
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
    //int nSkippedVN = 0;
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
            m_vertexColors.push_back(GeomColor(200,200,200,255));
            break;
          case 't': // texture coordinates u,v,[w] (w is optional) (this is skipped)
            ++nSkippedVT;
            break;
          case 'n': // normal for vertex x,y,z (might not be unit!)
            m_normals.push_back(parse<F3>(line.substr(2)).resize<1,4>(1));
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
            std::vector<int> is(n),ns(n);
            for(int i=0;i<n;++i){
              std::vector<int> t = parseVecStr<int>(x[i],"/");
              is[i] = t.at(0)-1;
              if(C == 'C'){
                ns[i] = t.at(2)-1;
              }else if(C == 'D'){
                ns[i] = t.at(1)-1;
              }
            }
            if(n == 3){
              if(C == 'B'){
                addTriangle(is[0],is[1],is[2]);
              }else{
                addTriangle(is[0],is[1],is[2],ns[0],ns[1],ns[2]);
              }
            }else if (n == 4){
              if( C == 'B'){
                addQuad(is[0],is[1],is[2],is[3]);
              }else{
                addQuad(is[0],is[1],is[2],is[3],ns[0],ns[1],ns[2],ns[3]);
              }
            }else{
              if( C == 'B'){
                addPolygon(is);
              }else{
                addPolygon(is,ns);
              }
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
    setVisible(Primitive::line,false);
    setVisible(Primitive::vertex,false);
    setVisible(Primitive::triangle,true);
    setVisible(Primitive::quad,true);
    setVisible(Primitive::polygon,true);

    
    for(unsigned int i=0;i<m_primitives.size();++i){
      const Primitive &p = m_primitives[i];
      for(unsigned int j=0;j<p.normalIndices.size();++j){
        if(p.normalIndices[j] >= (int)m_normals.size()){
          throw ICLException("found normal error in " + str(i) + "th primitive: (normal-index is "
                             + str(p.normalIndices[j]) + " normal count is " + str(m_normals.size()));
        }
      }
      for(unsigned int j=0;j<p.vertexIndices.size();++j){
        if(p.vertexIndices[j] >= (int)m_vertices.size()){
          throw ICLException("found vertex error in " + str(i) + "th primitive: (vertex-index is "
                             + str(p.vertexIndices[j]) + " vertex count is " + str(m_vertices.size()));
        }
      }
    }
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

  const SceneObject *SceneObject::getChild(int index) const{
    return const_cast<SceneObject*>(this)->getChild(index);
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
    
    for(unsigned int i=0;i<m_primitives.size();++i){
      m_primitives[i].detachTextureIfDeepCopied();
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

  
  std::vector<Vec> SceneObject::getTransformedVertices() const{
    std::vector<Vec> ts(m_vertices.size());
    Mat T = getTransformation();
    for(unsigned int i=0;i<ts.size();++i){
      ts[i] = T * m_vertices[i];
    }
    return ts;
  }

  Vec SceneObject::getClosestVertex(const Vec &pWorld, bool relative) throw (ICLException){
    std::vector<Vec> ts = getTransformedVertices();
    if(!ts.size()) throw ICLException("getClosestVertex called on an object that has not vertices");
    std::vector<float> distances(ts.size());
    for(unsigned int i=0;i<ts.size();++i){
      distances[i] = (icl::sqr(pWorld[0]-ts[i][0]) + 
                      icl::sqr(pWorld[1]-ts[i][1]) + 
                      icl::sqr(pWorld[2]-ts[i][2]) ); // no sqrt(..) neccessary since we need to find the max. only
    }
    int idx = (int)(std::min_element(distances.begin(),distances.end()) - distances.begin());
    if(relative) return m_vertices[idx];
    else return ts[idx];
  }
  

  //Input:  a ray R, and a triangle T
  //    Output: *I = intersection point (when it exists)
  //    Return: -1 = triangle is degenerate (a segment or point)
  //             0 = disjoint (no intersect)
  //             1 = intersect in unique point I1
  //             2 = are in the same plane
  enum RayTriangleIntersection{
    noIntersection,
    foundIntersection,
    degenerateTriangle,
    rayIsCollinearWithTriangle
  };
  
  struct Triangle{
    Triangle(const Vec &a, const Vec &b, const Vec &c):a(a),b(b),c(c){}
    Vec a,b,c;
  };
  
  static inline float dot(const Vec &a, const Vec &b){
    return a[0]*b[0] +a[1]*b[1] +a[2]*b[2]; 
  }
  
  // inspired from http://softsurfer.com/Archive/algorithm_0105/algorithm_0105.htm#intersect_RayTriangle()
  RayTriangleIntersection compute_intersection(const ViewRay &r, const Triangle &t, Vec &intersection){
    //Vector    u, v, n;             // triangle vectors
    //Vector    dir, w0, w;          // ray vectors
    //float     r, a, b;             // params to calc ray-plane intersect
    static const float EPSILON = 0.00000001;
    // get triangle edge vectors and plane normal
    Vec u = t.b - t.a;
    Vec v = t.c - t.a;
    Vec n = cross(u,v);  // TEST maybe v,u ??
    if (fabs(n[0]) < EPSILON && fabs(n[1]) < EPSILON && fabs(n[2]) < EPSILON){
      return degenerateTriangle;
    }

    const Vec dir = r.direction;  // dir = R.P1 - R.P0; // points from 0->1
    Vec w0 = r.offset - t.a;      //R.P0 - T.V0;   
    float a = -dot(n,w0);
    float b = dot(n,dir);
    if (fabs(b) < EPSILON) {     // ray is parallel to triangle plane
      return a<EPSILON ? rayIsCollinearWithTriangle : noIntersection;
    }
    
    // get intersect point of ray with triangle plane
    float rr = a / b;
    if (rr < 0) {
      return noIntersection;
    }
    // for a segment, also test if (r > 1.0) => no intersect 
    // a segment meaning a line-segment between a and b

    intersection = r.offset + dir * rr;
    //*I = R.P0 + r * dir;           // intersect point of ray and plane

    // is I inside T?
    //    float    uu, uv, vv, wu, wv, D;
    float uu = dot(u,u);
    float uv = dot(u,v);
    float vv = dot(v,v);
    Vec w = intersection - t.a; //T.V0;
    float wu = dot(w,u);
    float wv = dot(w,v);
    float D = uv * uv - uu * vv;

    // get and test parametric coords
    //float s, t;
    float s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0){
      // I is outside T
      return noIntersection;
    }
    float tt = (uv * wu - uu * wv) / D;
    if (tt < 0.0 || (s + tt) > 1.0){
      // I is outside T
      return noIntersection;
    }

    return foundIntersection; // I is in T
  }
  
  void SceneObject::collect_hits_recursive(SceneObject *obj, 
                                           const ViewRay &v, std::vector<Vec> &hits,
                                           std::vector<SceneObject*> &objects,
                                           bool recursive){
    std::vector<Vec> vs = obj->getTransformedVertices();
    
    if(vs.size()){
      // prepare for aabb (aka 3D-bounding box) check
      // check for intersections with the 3D bounding box-faces first
      Range32f aabb[3];
      for(int i=0;i<3;++i){
        aabb[i] = Range32f::limits();
        std::swap(aabb[i].minVal,aabb[i].maxVal);
      }      
      for(unsigned int i=0;i<vs.size();++i){
        const Vec &v = vs[i];
        if(v[0] < aabb[0].minVal) aabb[0].minVal = v[0];
        if(v[1] < aabb[1].minVal) aabb[1].minVal = v[1];
        if(v[2] < aabb[2].minVal) aabb[2].minVal = v[2];
        if(v[0] > aabb[0].maxVal) aabb[0].maxVal = v[0];
        if(v[1] > aabb[1].maxVal) aabb[1].maxVal = v[1];
        if(v[2] > aabb[2].maxVal) aabb[2].maxVal = v[2];
      }
      
      // 1st: apply check aabb for possible hit
      /**
          0-----1   ---> x
          |4----+5
          ||    ||
          2+----3|
           6-----7
          |
          V
          y
      */
      
      Vec v0(aabb[0].minVal,aabb[1].minVal,aabb[2].minVal);
      Vec v1(aabb[0].maxVal,aabb[1].minVal,aabb[2].minVal);
      Vec v2(aabb[0].minVal,aabb[1].maxVal,aabb[2].minVal);
      Vec v3(aabb[0].maxVal,aabb[1].maxVal,aabb[2].minVal);
      Vec v4(aabb[0].minVal,aabb[1].minVal,aabb[2].maxVal);
      Vec v5(aabb[0].maxVal,aabb[1].minVal,aabb[2].maxVal);
      Vec v6(aabb[0].minVal,aabb[1].maxVal,aabb[2].maxVal);
      Vec v7(aabb[0].maxVal,aabb[1].maxVal,aabb[2].maxVal);
      
      Vec __;
      
      // important optimization check the 3D-bounding box for intersection with the
      // given ray first -> this is in particular very important for e.g. spheres
      // that have a lot of faces ..
      if(compute_intersection(v,Triangle(v0,v1,v2),__) == foundIntersection ||
         compute_intersection(v,Triangle(v1,v3,v2),__) == foundIntersection ||
         compute_intersection(v,Triangle(v4,v5,v6),__) == foundIntersection ||
         compute_intersection(v,Triangle(v5,v6,v7),__) == foundIntersection ||
         compute_intersection(v,Triangle(v0,v1,v4),__) == foundIntersection ||
         compute_intersection(v,Triangle(v1,v4,v5),__) == foundIntersection ||
         compute_intersection(v,Triangle(v2,v3,v6),__) == foundIntersection ||
         compute_intersection(v,Triangle(v3,v6,v7),__) == foundIntersection ||
         compute_intersection(v,Triangle(v0,v4,v2),__) == foundIntersection ||
         compute_intersection(v,Triangle(v2,v4,v6),__) == foundIntersection ||
         compute_intersection(v,Triangle(v1,v5,v3),__) == foundIntersection ||
         compute_intersection(v,Triangle(v3,v5,v7),__) == foundIntersection ){
        
        for(unsigned int i=0;i<obj->m_primitives.size();++i){
          const Primitive &p = obj->m_primitives[i];
          switch(p.type){
            case Primitive::triangle:{
              Triangle t(vs[p.a()],
                         vs[p.b()],
                         vs[p.c()] );
              Vec pos;
              if(compute_intersection(v,t,pos) == foundIntersection){
                hits.push_back(pos);
                objects.push_back(obj);
              }
              break;
            }
            case Primitive::quad:{
              Triangle t1(vs[p.a()],
                          vs[p.b()],
                          vs[p.c()] );
              Triangle t2( vs[p.c()],
                           vs[p.d()],
                           vs[p.a()] );
              
              Vec pos;
              if(compute_intersection(v,t1,pos) == foundIntersection){
                hits.push_back(pos);
                objects.push_back(obj);
              }else if(compute_intersection(v,t2,pos) == foundIntersection){
                hits.push_back(pos);
                objects.push_back(obj);
              }
              break;
            }
            case Primitive::polygon:{
              int n = p.vertexIndices.size();
              // use easy algorithm: choose center and triangularize
              std::vector<Vec> vertices(n);
              Vec mean(0,0,0,0);
              for(int i=0;i<n;++i){
                vertices[i] = vs[p.vertexIndices[i]];
                mean += vertices.back();
              }
              mean *= (1.0/vertices.size());
              
              for(int i=0;i<n-1;++i){
                Triangle t( vs[p.vertexIndices[i]],
                            vs[p.vertexIndices[i+1]],
                            mean );
                Vec pos;
                if(compute_intersection(v,t,pos) == foundIntersection){
                  hits.push_back(pos);
                  objects.push_back(obj);
                  break;
                }
              }
              break;
            }
            default:
              // no checks for other types
              break;
          }
        } // switch
      } // if( aabb.wasHit...)
    } // if(m_vertices.size()

    if(recursive){
      /// recursion step
      for(unsigned int i=0;i<obj->m_children.size();++i){
        collect_hits_recursive(obj->m_children[i].get(),v,hits,objects,recursive);
      }
    }
  }
  
  struct VecAndObject{
    SceneObject *obj;
    Vec *v;
    float dist;
    bool operator<(const VecAndObject &other) const {
      return dist < other.dist;
    }
  };

  SceneObject *SceneObject::hit(const ViewRay &v, Vec *contactPos, bool recursive) {
    std::vector<Vec> hits;
    std::vector<SceneObject*> objects;
    collect_hits_recursive(this,v,hits,objects,recursive);
    if(!hits.size()) return false;

    std::vector<VecAndObject> all(hits.size());
    for(unsigned int i=0;i<all.size();++i){
      all[i].obj = objects[i];
      all[i].v = &hits[i];
      all[i].dist = (v.offset-hits[i]).length();
    }
    std::sort(all.begin(),all.end());

    if(contactPos){
      *contactPos = *all.front().v;
    }
    return all.front().obj;
  }
}
