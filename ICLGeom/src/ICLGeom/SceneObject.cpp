/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/SceneObject.cpp                    **
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

#include <ICLGeom/SceneObject.h>
#include <fstream>
#include <ICLUtils/File.h>
#include <ICLUtils/StringUtils.h>
#include <ICLGeom/PlaneEquation.h>
#include <ICLGeom/Scene.h>


using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::qt;


namespace icl{
  namespace geom{

    const std::vector<Vec> &SceneObject::getVertices() const {
      return m_vertices;
    }
    std::vector<Vec> &SceneObject::getVertices() {
      return m_vertices;
    }

		const std::vector<Vec> &SceneObject::getNormals() const {
			return m_normals;
		}

    const std::vector<GeomColor> &SceneObject::getVertexColors() const {
      return m_vertexColors;
    }
    std::vector<GeomColor> &SceneObject::getVertexColors() {
      return m_vertexColors;
    }

    const std::vector<Primitive*> &SceneObject::getPrimitives() const {
      return m_primitives;
    }
    std::vector<Primitive*> &SceneObject::getPrimitives() {
      return m_primitives;
    }

    void SceneObject::setVisible(int oredTypes, bool visible, bool recursive) {
      if(visible){
        m_visibleMask |= oredTypes;
      }else{
        m_visibleMask &= ~oredTypes;
      }
      if(recursive){
        for(unsigned int i=0;i<m_children.size();++i){
          m_children[i]->setVisible(oredTypes,visible,recursive);
        }
      }
    }

    void SceneObject::setVisible(const std::string &what, bool recursive){
      int on = 0, off = 0;
      //DEBUG_LOG("what : " << what);
      std::vector<std::string> ts = tok(what,",");
      for(size_t i=0;i<ts.size();++i){
        std::vector<std::string> ab = tok(ts[i],"=");
        if(!ab.size() || ab.size() > 2){
          ERROR_LOG("error in SceneObject::setVisible: invalid syntax: " << what);
        }
        bool enable = ab.size() < 2 || parse<bool>(ab[1]);
        int &target = (enable ? on : off);

#define CHECK_TYPE(x) else if(ab[0] == #x){ target |= Primitive::x; }
        if(ab[0] == "this" || ab[0] == "all"){
          setVisible(enable,recursive);
        }
        CHECK_TYPE(vertex)
        CHECK_TYPE(line)
        CHECK_TYPE(triangle)
        CHECK_TYPE(polygon)
        CHECK_TYPE(texture)
        CHECK_TYPE(text)
        CHECK_TYPE(faces)
        else {
          ERROR_LOG("invalid primitive type name: " << ab[0]);
        }
#undef CHECK_TYPE
      }

      setVisible(off, false, recursive);
      setVisible(on, true, recursive);
    }

    bool SceneObject::isVisible(Primitive::Type t) const {
      return m_visibleMask & t;
    }

    SceneObject::SceneObject():
      m_lineColorsFromVertices(false),
      m_triangleColorsFromVertices(false),
      m_quadColorsFromVertices(false),
      m_polyColorsFromVertices(false),
      m_useCustomRender(false),
      m_pointSize(1),
      m_lineWidth(1),
      m_useSmoothShading(true),
      m_isVisible(true),
      m_transformation(Mat::id()),
      m_hasTransformation(false),
      m_parent(0),
      m_mutex(Mutex::mutexTypeRecursive),
      m_enableLocking(false),
      m_pointSmoothingEnabled(true),
      m_lineSmoothingEnabled(true),
      m_polygonSmoothingEnabled(false),
      m_depthTestEnabled(true),
      m_shininess(128),
      m_specularReflectance(GeomColor(0.5,0.5,0.5,0.5)),
      m_displayListHandle(0),
      m_createDisplayListNextTime(0),
      m_fragmentShader(0),
      m_castShadows(true),
      m_receiveShadows(true),
      m_pointHitMaxDistance(10)
    {

      m_visibleMask = Primitive::all;
    }

    static const float COLOR_FACTOR = 1.0/255.0;

    void SceneObject::clearAllPrimitives(){
      for(unsigned int i=0;i<m_primitives.size();++i){
        delete m_primitives[i];
      }
      m_primitives.clear();
    }

    void SceneObject::clearObject(bool deleteAndRemoveChildren, bool resetTransform){
      clearAllPrimitives();
      m_vertices.clear();
      m_normals.clear();
      m_vertexColors.clear();
      m_sharedTextures.clear();

      if(deleteAndRemoveChildren){
        removeAllChildren();
      }
      if(resetTransform){
        removeTransformation();
      }
    }


    void SceneObject::addVertex(const Vec &p, const GeomColor &color){
      m_vertices.push_back(p);
      m_vertexColors.push_back(color*COLOR_FACTOR);
    }
    /// adds a new normal to this object
    void SceneObject::addNormal(const Vec &n){
      m_normals.push_back(n);
    }

    void SceneObject::addLine(int a, int b, const GeomColor &color){
      m_primitives.push_back(new LinePrimitive(a,b,color*COLOR_FACTOR));
    }

    void SceneObject::addTriangle(int a, int b, int c, int na, int nb, int nc, const GeomColor &color){
      m_primitives.push_back(new TrianglePrimitive(a,b,c,color*COLOR_FACTOR,na,nb,nc));
    }

    void SceneObject::addQuad(int a, int b, int c, int d, int na, int nb, int nc, int nd, const GeomColor &color){
      m_primitives.push_back(new QuadPrimitive(a,b,c,d,color*COLOR_FACTOR,na,nb,nc,nd));
    }

    void SceneObject::addPolygon(int nPoints,const int *vertexIndices, const GeomColor &color,
                                 const int *normalIndices){
      ICLASSERT_RETURN(vertexIndices);
      m_primitives.push_back(new PolygonPrimitive(nPoints,vertexIndices,
                                                  color*COLOR_FACTOR,normalIndices));
    }

    void SceneObject::addSharedTexture(SmartPtr<GLImg> gli){
      m_sharedTextures.push_back(gli);
    }

    void SceneObject::addSharedTexture(const ImgBase *image, scalemode sm){
      m_sharedTextures.push_back(new GLImg(image,sm));
    }

    void SceneObject::addTexture(int a, int b, int c, int d,const ImgBase *texture,
                                 int na, int nb, int nc, int nd, bool createTextureOnce, scalemode sm){
      m_primitives.push_back(new TexturePrimitive(a,b,c,d,texture,createTextureOnce,na,nb,nc,nd,sm));
    }

    void SceneObject::addTexture(int a, int b, int c, int d,
                                 int sharedTextureIndex,
                                 int na, int nb, int nc, int nd){
      m_primitives.push_back(new SharedTexturePrimitive(a,b,c,d,sharedTextureIndex,na,nb,nc,nd));
    }

    void SceneObject::addTexture(const ImgBase *image, int numPoints, const int *vertexIndices,
                                 const Point32f *texCoords, const int *normalIndices,
                                 bool createTextureOnce){
      m_primitives.push_back(new GenericTexturePrimitive(image, numPoints, vertexIndices, texCoords, normalIndices, createTextureOnce));
    }

    void SceneObject::addTextureGrid(int w, int h, const ImgBase *image,
                                     const icl32f *px, const icl32f *py, const icl32f *pz,
                                     const icl32f *pnx, const icl32f *pny, const icl32f *pnz,
                                     int stride,bool createTextureOnce,scalemode sm){
      m_primitives.push_back(new TextureGridPrimitive(w,h,image,px,py,pz,pnx, pny,pnz,
                                                      stride,createTextureOnce,sm));
    }

    void SceneObject::addTwoSidedTextureGrid(int w, int h, const ImgBase *front, const ImgBase *back,
                                             const icl32f *px, const icl32f *py, const icl32f *pz,
                                             const icl32f *pnx, const icl32f *pny, const icl32f *pnz,
                                             int stride,bool createFrontOnce, bool createBackOnce, scalemode sm){
      m_primitives.push_back(new TwoSidedTextureGridPrimitive(w,h,front,back,px,py,pz,pnx,pny,pnz,
                                                              stride,createFrontOnce, createBackOnce, sm));
    }

    void SceneObject::addTwoSidedTGrid(int w, int h, const Vec *vertices, const Vec *normals,
                                       const GeomColor &frontColor,
                                       const GeomColor &backColor,
                                       const GeomColor &lineColor,
                                       bool drawLines, bool drawQuads){
      m_primitives.push_back(new TwoSidedGridPrimitive(w,h,vertices, normals, frontColor, backColor,
                                                       lineColor, drawLines, drawQuads));
    }



    void SceneObject::addTextTexture(int a, int b, int c, int d, const std::string &text,
                                     const GeomColor &color,
                                     int na, int nb, int nc, int nd,
                                     int textSize, scalemode sm){
      m_primitives.push_back(new TextPrimitive(a,b,c,d,text,textSize,color,na,nb,nc,nd,-1, sm));
    }

    void SceneObject::addText(int a, const std::string &text, float billboardHeight,
                              const GeomColor &color, int textRenderSize, scalemode sm){
      m_primitives.push_back(new TextPrimitive(a,0,0,0,text,textRenderSize,color,-1,-1,-1,-1,billboardHeight, sm));
    }

    SceneObject *SceneObject::copy() const{
      return new SceneObject(*this);
    }

    void SceneObject::setColor(Primitive::Type t,const GeomColor &color, bool recursive){

      GeomColor colorScaled = color * COLOR_FACTOR;

      if(t == Primitive::vertex || t == Primitive::all){
        std::fill(m_vertexColors.begin(),m_vertexColors.end(),color);
      }
      if(t != Primitive::vertex){
        for(unsigned int i=0;i<m_primitives.size();++i){
          if(t == Primitive::all || m_primitives[i]->type == t){
            m_primitives[i]->color = colorScaled;
          }
        }
      }
      if(recursive){
        for(unsigned int i=0;i<m_children.size();++i){
          m_children[i]->setColor(t,color);
        }
      }
    }

    static inline float cos_sq(float n, float e){
      const float cn = cos(n);
      return (cn<0?-1:1)*pow(fabs(cn),e);
    }
    static inline float sin_sq(float n, float e){
      const float sn = sin(n);
      return (sn<0?-1:1)*pow(fabs(sn),e);
    }


    SceneObject::SceneObject(const std::string &type,const float *params):
      m_lineColorsFromVertices(false),
      m_triangleColorsFromVertices(false),
      m_quadColorsFromVertices(false),
      m_polyColorsFromVertices(false),
      m_useCustomRender(false),
      m_pointSize(1),
      m_lineWidth(1),
      m_useSmoothShading(true),
      m_isVisible(true),
      m_transformation(Mat::id()),
      m_hasTransformation(false),
      m_parent(0),
      m_mutex(Mutex::mutexTypeRecursive),
      m_enableLocking(false),
      m_pointSmoothingEnabled(true),
      m_lineSmoothingEnabled(true),
      m_polygonSmoothingEnabled(false),
      m_depthTestEnabled(true),
      m_shininess(128),
      m_specularReflectance(GeomColor(0.5,0.5,0.5,0.5)),
      m_displayListHandle(0),
      m_createDisplayListNextTime(0),
      m_fragmentShader(0),
      m_castShadows(true),
      m_receiveShadows(true),
      m_pointHitMaxDistance(10)
    {
      m_visibleMask = Primitive::all;

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

        /** cube scheme ...

        +z__
         |\      0
           \     |
            +----+----> +x
            | 3----0
            | |7---+4
          0-+ ||   ||
            | ||   ||
            | 2+---1|
            |  6----5
            V
            +y
        */

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
      }else if(type == "superquadric"){
        float x = *params++;
        float y = *params++;
        float z = *params++;
        float rotx = *params++;
        float roty = *params++;
        float rotz = *params++;
        float dx = *params++;
        float dy = *params++;
        float dz = *params++;

        float e1 = *params++;
        float e2 = *params++;

        int na = *params++;
        int nb = *params++;

        const float dAlpha = M_PI/(na-1);
        const float dBeta = 2*M_PI/nb;
        Mat T = create_hom_4x4<float>(rotx,roty,rotz,x,y,z);
        Mat R = create_hom_4x4<float>(rotx,roty,rotz);

        for(int i=0;i<na;++i){
          for(int j=0;j<nb;++j){
            float eta = i*dAlpha - (M_PI/2);
            float omega = j*dBeta - (M_PI);

            float X = cos_sq(eta,e1)*cos_sq(omega,e2);
            float Y = cos_sq(eta,e1)*sin_sq(omega,e2);
            float Z = sin_sq(eta,e1);
            addVertex(T * Vec(dx*X,dy*Y,dz*Z,1), geom_blue(200));
            addNormal(R * normalize3(-Vec(X,Y,Z)));

            if(i){
              if( i != (na-1)){
                addLine(j+nb*i, j ? (j+nb*i-1) : (nb-1)+nb*i);
              }

              int a = j+nb*i;
              int b = j ? (j+nb*i-1) : (nb-1)+nb*i;
              int c = j ? (j+nb*(i-1)-1) : (nb-1)+nb*(i-1);
              int d = j+nb*(i-1);
              if(i == na-1){
                addQuad(a,d,c,b,
                        a,d,c,b);
              }else{
                addQuad(d,c,b,a,
                        d,c,b,a);
              }
              addLine(j+nb*i, j+nb*(i-1));
            }

          }
        }
      }else if(type == "cone"){
        // args: x,y,z, dx, dy, dz, steps
        float x = *params++;
        float y = *params++;
        float z = *params++;
        float rx = *params++/2;
        float ry = *params++/2;
        float h = *params++/2;
        int steps = *params;

        float da = (2*M_PI)/steps;

        addVertex(Vec(x,y,z+h,1));
        addNormal(Vec(0,0,1,1));
        std::vector<int> bottom;
        for(int i=0;i<steps;++i){
          float a = i*da;
          float ca = cos(a), sa = sin(a);
          addVertex(Vec(x+rx*ca, y+ry*sa, z-h, 1));
          addLine(0,i+1);
          if(i){
            addLine(i,i+1);
            addTriangle(i,i+1,0);
          }else{
            addLine(steps-1,1);
            addTriangle(0,steps,1);
          }

          bottom.push_back(i+1);
        }
        addNormal(Vec(0,0,-1,1));
        addPolygon(steps,bottom.data(),geom_blue(),std::vector<int>(steps,m_normals.size()-1).data());
      }else if(type == "cylinder"){
        // args: x,y,z, dx, dy, dz, steps
        float x = *params++;
        float y = *params++;
        float z = *params++;
        float rx = *params++/2;
        float ry = *params++/2;
        float h = *params++/2;
        int steps = *params;

        float da = (2*M_PI)/steps;
        addNormal(Vec(0,0,1,1));

        std::vector<int> bottom,top;
        for(int i=0;i<steps;++i){
          float a = i*da;
          float cx = x+rx*cos(a), cy=y+ry*sin(a);
          addVertex(Vec(cx,cy,z-h,1));
          addVertex(Vec(cx,cy,z+h,1));

          addLine(2*i, 2*i+1);
          if(i){
            addLine(2*i, 2*(i-1));
            addLine(2*i+1, 2*(i-1)+1);
            addQuad(2*(i-1), 2*i, 2*i+1, 2*(i-1)+1);
          }else{
            addLine(0, 2*(steps-1));
            addLine(1, 2*(steps-1)+1);
            //            addQuad(0, 2*(steps-1), 2*(steps-1)+1, 1);
            addQuad(1, 2*(steps-1)+1, 2*(steps-1), 0);
          }
          bottom.push_back(2*i);
          top.push_back(2*i+1);
        }
        addNormal(Vec(0,0,-1,1));
        addNormal(Vec(0,0,1,1));
        addPolygon(steps,top.data(),geom_blue(),std::vector<int>(top.size(),m_normals.size()-1).data());
        addPolygon(steps,bottom.data(),geom_blue(),std::vector<int>(top.size(),m_normals.size()-2).data());
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

    SceneObject::~SceneObject(){
      for(unsigned int i=0;i<m_primitives.size();++i){
        delete m_primitives[i];
      }
      if(m_displayListHandle){
        Scene::freeDisplayList(m_displayListHandle);
        m_displayListHandle = 0;
      }
      ICL_DELETE(m_fragmentShader);
    }

    SceneObject::SceneObject(const std::string &objFileName) throw (ICLException):
      m_lineColorsFromVertices(false),
      m_triangleColorsFromVertices(false),
      m_quadColorsFromVertices(false),
      m_polyColorsFromVertices(false),
      m_useCustomRender(false),
      m_pointSize(1),
      m_lineWidth(1),
      m_useSmoothShading(true),
      m_isVisible(true),
      m_transformation(Mat::id()),
      m_hasTransformation(false),
      m_parent(0),
      m_mutex(Mutex::mutexTypeRecursive),
      m_enableLocking(false),
      m_pointSmoothingEnabled(true),
      m_lineSmoothingEnabled(true),
      m_polygonSmoothingEnabled(false),
      m_depthTestEnabled(true),
      m_shininess(128),
      m_specularReflectance(GeomColor(0.5,0.5,0.5,0.5)),
      m_displayListHandle(0),
      m_createDisplayListNextTime(0),
      m_fragmentShader(0),
      m_castShadows(true),
      m_receiveShadows(true),
      m_pointHitMaxDistance(0)
    {
      File file(objFileName,File::readText);
      if(!file.exists()) throw ICLException("Error in SceneObject(objFilename): unable to open file " + objFileName);

      setSmoothShading(true);

      typedef FixedColVector<float,3> F3;

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
        }else if(line[0] == 'l'){ // line
          // f v1 v2 v3 v4 ... -> line strip
          std::vector<int> linestrip = parseVecStr<int>(line.substr(2)," ");
          for(unsigned int l=1;l<linestrip.size();++l){
            addLine(linestrip[l-1]-1,linestrip[l]-1);
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
                addPolygon(xx.size(),xx.data());
              }
              break;
            case 'B': // for now, this is simple, we simply dont use the 2nd and 3rd token;
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
                  addPolygon(is.size(),is.data());
                }else{
                  addPolygon(is.size(),is.data(),GeomColor(0,100,255,255),ns.data());
                }
              }
            }
          }
        }else if(line[0] == '#') {
          if(line.substr(1,4) == "!icl"){
            std::string rest = line.substr(5);
            std::vector<std::string> ts = tok(rest," ");
            if(ts.size() < 2){
              WARNING_LOG("parsing object file " << objFileName << " (line: "
                          << line  << "): #!icl - line does not contain enough tokens!");
            }else if(ts[0] == "transformation"){
              setTransformation(parse<Mat>(cat(std::vector<std::string>(ts.begin()+1,ts.end())," ")));
            }else{
              WARNING_LOG("parsing object file " << objFileName << " (line: "
                          << line  << "): #!icl - line cannot be parsed!");
            }
          }
          continue;
        }else if(line[0] == ' ') {
          continue;
        }
        else if(line[0] == 's') {
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

    void SceneObject::rotate(float rx, float ry, float rz,  AXES axes){
      transform(create_hom_4x4<float>(rx,ry,rz, 0,0,0, 0,0,0, axes));
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
    void SceneObject::addChild(SmartPtr<SceneObject> child){
      m_children.push_back(child);
      child->m_parent = this;
    }

    bool SceneObject::hasChild(const SceneObject *o) const{
      for(size_t i=0;i<m_children.size();++i){
        if(m_children[i].get() == o) return true;
      }
      return false;
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

    /// returns a shared pointer to the child at given index
    SmartPtr<SceneObject> SceneObject::getChildPtr(int index){
      if(index < 0 || index >= (int)m_children.size()) return SmartPtr<SceneObject>();
      return m_children[index];
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

    void SceneObject::setUseCustomRender(bool use, bool recursive){
      m_useCustomRender = use;
      if(recursive){
        for(unsigned int i=0;i<m_children.size();++i){
          m_children[i]->setUseCustomRender(use);
        }
      }
    }

    SceneObject &SceneObject::operator=(const SceneObject &other){
      if(this == &other) return *this;
  #define DEEP_COPY(X) X = other.X
  #define DEEP_COPY_2(X,Y) DEEP_COPY(X); DEEP_COPY(Y)
  #define DEEP_COPY_4(X,Y,A,B) DEEP_COPY_2(X,Y); DEEP_COPY_2(A,B)
      DEEP_COPY_2(m_vertices,m_vertexColors);
      DEEP_COPY(m_normals);
      DEEP_COPY_4(m_primitives,m_lineColorsFromVertices,m_triangleColorsFromVertices,m_quadColorsFromVertices);
      DEEP_COPY_4(m_polyColorsFromVertices,m_pointSize,m_lineWidth,m_useSmoothShading);
      DEEP_COPY_2(m_transformation,m_hasTransformation);
  #undef DEEP_COPY
  #undef DEEP_COPY_2
  #undef DEEP_COPY_4

      m_pointSmoothingEnabled = other.m_pointSmoothingEnabled;
      m_lineSmoothingEnabled  = other.m_lineSmoothingEnabled;
      m_polygonSmoothingEnabled = other.m_polygonSmoothingEnabled;
      m_depthTestEnabled = other.m_depthTestEnabled;
      m_shininess = other.m_shininess;
      m_specularReflectance = other.m_specularReflectance;
      m_pointHitMaxDistance = other.m_pointHitMaxDistance;
      m_useCustomRender = other.m_useCustomRender;

      setLockingEnabled(other.getLockingEnabled());
      m_visibleMask = other.m_visibleMask;
      m_children.clear();
      m_children.resize(other.m_children.size());
      for(unsigned int i=0;i<other.m_children.size();++i){
        m_children[i] = SmartPtr<SceneObject>(other.m_children[i]->copy());
      }

      for(unsigned int i=0;i<m_primitives.size();++i){
        m_primitives[i] = m_primitives[i]->copy();
      }
      m_sharedTextures = other.m_sharedTextures;
      for(unsigned int i=0;i<m_sharedTextures.size();++i){
        m_sharedTextures[i] = new GLImg(m_sharedTextures[i]->extractImage(),
                                        m_sharedTextures[i]->getScaleMode());
      }
      if(m_displayListHandle){
        Scene::freeDisplayList(m_displayListHandle);
        m_displayListHandle = 0;
      }
      if(other.getFragmentShader()){
        setFragmentShader(other.getFragmentShader()->copy());
      }else{
        setFragmentShader(0);
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
      float params[] = {x,y,z,rx,ry,rz,(float)rzSteps,(float)xySlices};
      SceneObject *o = new SceneObject("spheroid",params);
      addChild(o);
      return o;
    }

    SceneObject *SceneObject::addCylinder(float x, float y, float z, float rx, float ry, float h, int steps){
      float params[] = {x,y,z,rx,ry,h,(float)steps};
      SceneObject *o = new SceneObject("cylinder",params);
      addChild(o);
      return o;
    }

    SceneObject *SceneObject::addCone(float x, float y, float z, float rx, float ry, float h, int steps){
      float params[] = {x,y,z,rx,ry,h,(float)steps};
      SceneObject *o = new SceneObject("cone",params);
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
        distances[i] = (utils::sqr(pWorld[0]-ts[i][0]) +
                        utils::sqr(pWorld[1]-ts[i][1]) +
                        utils::sqr(pWorld[2]-ts[i][2]) ); // no sqrt(..) neccessary since we need to find the max. only
      }
      int idx = (int)(std::min_element(distances.begin(),distances.end()) - distances.begin());
      if(relative) return m_vertices[idx];
      else return ts[idx];
    }



    struct Triangle{
      Triangle(const Vec &a, const Vec &b, const Vec &c):a(a),b(b),c(c){}
      Vec a,b,c;
    };

    ViewRay::TriangleIntersection compute_intersection(const ViewRay &r, const Triangle &t, Vec &intersection){
      return r.getIntersectionWithTriangle(t.a,t.b,t.c,&intersection);
    }

#if 0
    enum RayTriangleIntersection{
      noIntersection,
      foundIntersection,
      wrongDirection,
      degenerateTriangle,
      rayIsCollinearWithTriangle
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
      Vec n = cross(v,u);  // TEST maybe v,u ??
      if (fabs(n[0]) < EPSILON && fabs(n[1]) < EPSILON && fabs(n[2]) < EPSILON){
        return degenerateTriangle;
      }

      const Vec dir = r.direction;
      Vec w0 =  r.offset - t.a;

      float a = -dot(n,w0);
      float b = dot(n,dir);
      if (fabs(b) < EPSILON) {     // ray is parallel to triangle plane
        return a<EPSILON ? rayIsCollinearWithTriangle : noIntersection;
      }

      // get intersect point of ray with triangle plane
      float rr = a / b;
      if (rr < 0) {
        return wrongDirection;
      }

      intersection = r.offset + dir * rr;

      // is I inside T?
      float uu = dot(u,u);
      float uv = dot(u,v);
      float vv = dot(v,v);
      Vec w = intersection - t.a;
      float wu = dot(w,u);
      float wv = dot(w,v);
      float D = uv * uv - uu * vv;

      // get and test parametric coords
      float s = (uv * wv - vv * wu) / D;
      if (s < 0.0 || s > 1.0){
        return noIntersection;
      }
      float tt = (uv * wu - uu * wv) / D;
      if (tt < 0.0 || (s + tt) > 1.0){
        return noIntersection;
      }
      intersection[3] = 1;
      return foundIntersection;
    }
#endif

    static float l3(const Vec &a, const Vec &b){
      float l = sqrt( sqr(a[0]-b[0]) + sqr(a[1]-b[1]) + sqr(a[2]-b[2]) );
      //    DEBUG_LOG("a:" << a.transp() << " b:" << b.transp() << " |a-b|:" << l);
      return l;
    }

    void SceneObject::collect_hits_recursive(SceneObject *obj, const ViewRay &v,
                                             std::vector<Hit> &hits, bool recursive){
      std::vector<Vec> vs = obj->getTransformedVertices();

      int nFaces = 0;
      for(unsigned int i=0;i<obj->m_primitives.size();++i){
        const Primitive &p = *obj->m_primitives[i];
        switch(p.type){
          case Primitive::triangle:
            nFaces++; break;
          case Primitive::quad:
            nFaces+=2; break;
          case Primitive::texture:{
            const TextureGridPrimitive *tg = dynamic_cast<const TextureGridPrimitive*>(&p);
            if(tg){
              nFaces += tg->w * tg->h * 2;
            }else{
              nFaces+=2;
            }
            break;
          }
          case Primitive::polygon:
            nFaces+=dynamic_cast<const PolygonPrimitive&>(p).getNumPoints()-2; break;
          default:
            break;
        }

      }
      if(vs.size()){
        bool aabbCheckOK = false;
        if(nFaces < 20){
          aabbCheckOK = true;
        }else{
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
          aabbCheckOK = (compute_intersection(v,Triangle(v0,v1,v2),__) == ViewRay::foundIntersection ||
                         compute_intersection(v,Triangle(v1,v3,v2),__) == ViewRay::foundIntersection ||
                         compute_intersection(v,Triangle(v4,v5,v6),__) == ViewRay::foundIntersection ||
                         compute_intersection(v,Triangle(v5,v6,v7),__) == ViewRay::foundIntersection ||
                         compute_intersection(v,Triangle(v0,v1,v4),__) == ViewRay::foundIntersection ||
                         compute_intersection(v,Triangle(v1,v4,v5),__) == ViewRay::foundIntersection ||
                         compute_intersection(v,Triangle(v2,v3,v6),__) == ViewRay::foundIntersection ||
                         compute_intersection(v,Triangle(v3,v6,v7),__) == ViewRay::foundIntersection ||
                         compute_intersection(v,Triangle(v0,v4,v2),__) == ViewRay::foundIntersection ||
                         compute_intersection(v,Triangle(v2,v4,v6),__) == ViewRay::foundIntersection ||
                         compute_intersection(v,Triangle(v1,v5,v3),__) == ViewRay::foundIntersection ||
                         compute_intersection(v,Triangle(v3,v5,v7),__) == ViewRay::foundIntersection );

          // ok, TextureGridPrimitives are not supported so far
          // However as long as their xs, ys, and zs, pointers point directly
          // into the vertex list of the object, it _is_ supported implicitly
        }
        if(aabbCheckOK){
          //std::cout << "?? checking " << obj->m_vertices.size() << " vertices" << std::endl;
          float maxD = sqr(obj->getPointHitMaxDistance());

          int n = 0;
          for(size_t i=0;i<vs.size();++i){
            float d_squared = v.closestSqrDistanceTo(vs[i]);
            if(d_squared < maxD){
              ++n;
              Hit h;
              h.pos = vs[i];
              h.obj = obj;
              h.dist = l3(v.offset, vs[i]) + 2*sqrt(d_squared);
              if (h.dist > 0.001){ // ohh nasty one here, but we need to remove the ones that are mapped into the
                                  // camera center here
                hits.push_back(h);
              }
            }
          }// use just the points here!
          //std::cout << "added " << n << " hits\n" << std::endl;

          for(unsigned int i=0;i<obj->m_primitives.size();++i){
            const Primitive *p = obj->m_primitives[i];
            switch(p->type){
              case Primitive::triangle:{
                Hit h;
                  const TrianglePrimitive *tp = reinterpret_cast<const TrianglePrimitive*>(p);
                  if(compute_intersection(v,Triangle(vs[tp->i(0)],vs[tp->i(1)],vs[tp->i(1)] ),h.pos) == ViewRay::foundIntersection){
                    h.obj = obj;
                    h.dist = l3(v.offset,h.pos);
                    hits.push_back(h);
                  }
                  break;
                }
                case Primitive::texture:
                case Primitive::quad:{
                  const TextureGridPrimitive *t = dynamic_cast<const TextureGridPrimitive*>(p);
                  const TwoSidedGridPrimitive *tg = dynamic_cast<const TwoSidedGridPrimitive*>(p);

                  Hit h;
                  if(t){
                    for(int x=1;x<t->w;++x){
                      for(int y=1;y<t->h;++y){
                        Vec a = t->getPos(x-1,y-1);
                        Vec b = t->getPos(x,y-1);
                        Vec c = t->getPos(x,y);
                        Vec d = t->getPos(x-1,y);

                        if(compute_intersection(v, Triangle(a,d,b), h.pos) == ViewRay::foundIntersection){
                          h.obj = obj;
                          h.dist = l3(v.offset,h.pos);
                          hits.push_back(h);
                        }else if(compute_intersection(v, Triangle(c,b,d), h.pos) == ViewRay::foundIntersection){
                          h.obj = obj;
                          h.dist = l3(v.offset,h.pos);
                          hits.push_back(h);
                        }
                      }
                    }
                  }else if(tg){
                    for(int x=1;x<tg->w;++x){
                      for(int y=1;y<tg->h;++y){
                        Vec a = tg->getPos(x-1,y-1);
                        Vec b = tg->getPos(x,y-1);
                        Vec c = tg->getPos(x,y);
                        Vec d = tg->getPos(x-1,y);

                        if(compute_intersection(v, Triangle(a,d,b), h.pos) == ViewRay::foundIntersection){
                          h.obj = obj;
                          h.dist = l3(v.offset,h.pos);
                          hits.push_back(h);
                        }else if(compute_intersection(v, Triangle(c,b,d), h.pos) == ViewRay::foundIntersection){
                          h.obj = obj;
                          h.dist = l3(v.offset,h.pos);
                          hits.push_back(h);
                        }
                      }
                    }
                  }else{

                    /** a--b xxx
                        |  |
                        d--c
                        */

                    const QuadPrimitive *qp = reinterpret_cast<const QuadPrimitive*>(p);
                    if(compute_intersection(v, Triangle(vs[qp->i(0)],vs[qp->i(1)],vs[qp->i(2)] ),h.pos) == ViewRay::foundIntersection){
                      h.obj = obj;
                      h.dist = l3(v.offset,h.pos);
                      hits.push_back(h);
                    }else if(compute_intersection(v,Triangle(vs[qp->i(0)],vs[qp->i(2)],vs[qp->i(3)]),h.pos) == ViewRay::foundIntersection){
                      h.obj = obj;
                      h.dist = l3(v.offset,h.pos);
                      hits.push_back(h);
                    }
                  }
                  break;
                }
              case Primitive::polygon:{
                const PolygonPrimitive *pp = reinterpret_cast<const PolygonPrimitive*>(p);
                int n = pp->getNumPoints();
                // use easy algorithm: choose center and triangularize
                std::vector<Vec> vertices(n);
                Vec mean(0,0,0,0);
                for(int i=0;i<n;++i){
                  vertices[i] = vs[pp->getVertexIndex(i)];
                  mean += vertices.back();
                }
                mean *= (1.0/vertices.size());

                for(int i=0;i<n-1;++i){
                  Hit h;
                  Triangle t(vs[pp->getVertexIndex(i)],vs[pp->getVertexIndex(i+1)],mean);
                  if(compute_intersection(v,t,h.pos) == ViewRay::foundIntersection){
                    h.obj = obj;
                    h.dist = l3(v.offset,h.pos);
                    hits.push_back(h);
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
          collect_hits_recursive(obj->m_children[i].get(),v,hits,true);
        }
      }
    }

    Hit SceneObject::hit(const ViewRay &v, bool recursive) {
      std::vector<Hit> hits;
      collect_hits_recursive(this,v,hits,recursive);
      return hits.size() ? *std::min_element(hits.begin(),hits.end()) : Hit();
    }

    std::vector<Hit> SceneObject::hits(const ViewRay &v, bool recursive){
      std::vector<Hit> hits;
      collect_hits_recursive(this,v,hits,recursive);
      std::sort(hits.begin(),hits.end());
      return hits;
    }

    void SceneObject::setVisible(bool visible, bool recursive){
      m_isVisible = visible;
      if(recursive){
        for(unsigned int i=0;i<m_children.size();++i){
          m_children[i]->setVisible(visible,true);
        }
      }
    }

    void SceneObject::createDisplayList(){
      m_createDisplayListNextTime = 1;
    }

    void SceneObject::freeDisplayList(){
      m_createDisplayListNextTime = 2;
    }

    void SceneObject::setFragmentShader(GLFragmentShader *shader){
      ICL_DELETE(m_fragmentShader);
      m_fragmentShader = shader;
    }

    void SceneObject::createAutoNormals(bool smooth){
      if(smooth){
        /// list of faces that use each vertex
        std::vector<std::vector<Vec> > graph(m_vertices.size());

        for(size_t i=0;i<m_primitives.size();++i){
          Primitive *p = m_primitives[i];
          switch(p->type){
            case Primitive::triangle:{
              TrianglePrimitive &t = (TrianglePrimitive&)*p;
              Vec n = t.computeNormal(m_vertices);
              for(int i=0;i<3;++i){
                graph[t[i]].push_back(n);
                t[i+3] = t[i];
              }
              break;

            }
            case Primitive::quad:
            case Primitive::texture:{
              if(dynamic_cast<QuadPrimitive*>(p)){
                QuadPrimitive &q = (QuadPrimitive&)*p;
                Vec n = q.computeNormal(m_vertices);
                for(int i=0;i<4;++i){
                  graph[q[i]].push_back(n);
                  q[i+4] = q[i];
                }
              }
              break;
            }
            default:
              break;
          }
        }
        m_normals.resize(m_vertices.size());
        for(size_t i=0;i<m_vertices.size();++i){
          int n = graph[i].size();
          if(!n) continue;
          m_normals[i] = std::accumulate(graph[i].begin(),graph[i].end(), Vec(0,0,0,0)) * (1./n);
          m_normals[i][3] = 1;
        }
      }else{
        m_normals.resize(m_primitives.size());

        for(size_t i=0;i<m_primitives.size();++i){
          Primitive *p = m_primitives[i];
          switch(p->type){
            case Primitive::triangle:{
              TrianglePrimitive &t = (TrianglePrimitive&)*p;
              m_normals[i] = t.computeNormal(m_vertices);
              for(int j=0;j<3;++j){
                t[j+3] = i;
              }
              break;
            }
            case Primitive::quad:
            case Primitive::texture:{
              if(dynamic_cast<QuadPrimitive*>(p)){
                QuadPrimitive &q = (QuadPrimitive&)*p;
                m_normals[i] = q.computeNormal(m_vertices);
                for(int j=0;j<4;++j){
                  q[j+4] = i;
                }
              }
              break;
            }
            default:
              break;
          }
        }
      }
    }
  } // namespace geom
}
