#include <iclObject.h>
#include <iclDrawWidget.h>
#include <iclQuick.h>
#include <iclImg.h>
#include <iclImgChannel.h>

namespace icl{
  
  Object::Object():T(Mat::id()),m_bPointsVisible(true),m_bLinesVisible(true),
                   m_bTrianglesVisible(true),m_bQuadsVisible(true){ 
    // {{{ open
  }

  // }}}
  Object::~Object(){
    // {{{ open

  }

  // }}}

  void Object::project(const Mat &CAMMATRIX){
    // {{{ open
    for(unsigned int i=0;i<m_vecPtsTrans.size();++i){ 
      m_vecPtsProj[i] = (CAMMATRIX*m_vecPtsTrans[i]);
      m_vecPtsProj[i].homogenize();
      m_vecPtsProj[i].z() = m_vecPtsTrans[i].z();
    }
  }

  // }}}
  void Object::transform(const Mat &m){
    // {{{ open

    T=T*m;
    for(unsigned int i=0;i<m_vecPtsOrig.size();++i){ 
      m_vecPtsTrans[i] = (T*m_vecPtsOrig[i]);
      //m_vecPtsTrans[i] = (T*m_vecPtsOrig[i]).homogenize(); this is not necessary as long as T is just Rot.Scale.Transl
    }
  }

  // }}}
  void Object::add(const Vec &p, const Vec &color){
    // {{{ open

    m_vecPtsOrig.push_back(p);
    m_vecPtsTrans.push_back(T*p);
    m_vecPtsProj.push_back(Vec(0,0,0,1));
    m_vecPtsColors.push_back(color);
  }

  // }}}
  void Object::add(const Tuple &t, const Vec &color){
    // {{{ open
    m_vecConnections.push_back(t);
    m_vecLineColors.push_back(color);
  }

  // }}}
  void Object::add(const Triple &t, const Vec &color){
    // {{{ open
    m_vecTriangles.push_back(t);
    m_vecTriangleColors.push_back(color);
  }
  // }}}
 
  void Object::add(const Quadruple &q, const Vec &color){
    // {{{ open
    m_vecQuads.push_back(q);
    m_vecQuadColors.push_back(color);
  }

  // }}}
  
  void Object::tintPoint(int i, const Vec &color){
    // {{{ open

    m_vecPtsColors[i] = color;
  }

  // }}}
  void Object::tintLine(int i, const Vec &color){
    // {{{ open

    m_vecLineColors[i] = color;
  }

  // }}}

  void Object::tintTriangle(int i, const Vec &color){

    // {{{ open
    m_vecTriangleColors[i] = color;
  }

  // }}}

  void Object::tintQuad(int i, const Vec &color){
    // {{{ open

    m_vecQuadColors[i] = color;
  }

  // }}}


  namespace{
    struct P3{
      // {{{ open

      inline P3():a(0),b(0),c(0),color(0),z(0){}
      inline P3(const Vec *a,const Vec *b,const Vec *c, const Vec *color):a(a),b(b),c(c),color(color){
        z = (a->z()+b->z()+c->z())/3;
      }
      const Vec *a,*b,*c,*color;
      float z;
      bool operator<(const P3 &p) const{
        return z<p.z;
      }
    };    

    // }}}
    struct P4{
      // {{{ open

      inline P4():a(0),b(0),c(0),d(0),color(0),z(0){}
      inline P4(const Vec *a,const Vec *b,const Vec *c, const Vec *d, const Vec *color):a(a),b(b),c(c),d(d),color(color){
        z = (a->z()+b->z()+c->z()+d->z())/4;
      }
      const Vec *a,*b,*c,*d,*color;
      float z;
      bool operator<(const P4 &p) const{
        return z<p.z;
      }
    };    

    // }}}

  }
  void Object::render(ICLDrawWidget *widget) const{
    // {{{ open
    
    if(m_bTrianglesVisible && m_vecTriangles.size()){
      vector<P3> v;
      const VecArray &p = m_vecPtsProj;
      for(unsigned int i=0;i<m_vecTriangles.size();i++){
        const Triple &t = m_vecTriangles[i];
        v.push_back(P3(&p[t.a],&p[t.b],&p[t.c],&m_vecTriangleColors[i]));
      }
      std::sort(v.begin(),v.end());
      widget->color(0,0,0,0);
      for(int i=0;i<(int)v.size();i++){
        widget->fill((int)(*(v[i].color))[0],
                     (int)(*(v[i].color))[1],
                     (int)(*(v[i].color))[2],
                     (int)(*(v[i].color))[3]);
        widget->triangle( (*(v[i].a))[0],(*(v[i].a))[1],
                          (*(v[i].b))[0],(*(v[i].b))[1],
                          (*(v[i].c))[0],(*(v[i].c))[1] );
      }    
    }
    
    if(m_bQuadsVisible && m_vecQuads.size()){
      vector<P4> v;
      const VecArray &p = m_vecPtsProj;
      for(unsigned int i=0;i<m_vecQuads.size();i++){
        const Quadruple &t = m_vecQuads[i];
        v.push_back(P4(&p[t.a],&p[t.b],&p[t.c],&p[t.d],&m_vecQuadColors[i]));
      }
      std::sort(v.begin(),v.end());
      widget->color(0,0,0,0);
      for(int i=0;i<(int)v.size();i++){
        widget->fill((int)(*(v[i].color))[0],
                     (int)(*(v[i].color))[1],
                     (int)(*(v[i].color))[2],
                     (int)(*(v[i].color))[3]);
        widget->quad( (*(v[i].a))[0],(*(v[i].a))[1],
                      (*(v[i].b))[0],(*(v[i].b))[1],
                      (*(v[i].c))[0],(*(v[i].c))[1],
                      (*(v[i].d))[0],(*(v[i].d))[1] );
      }    
    }
    
    Size s = widget->getImageSize();
    widget->fill(0,0,0,0);
    if(m_bLinesVisible){
      for(unsigned int i=0;i<m_vecConnections.size();i++){
        const Vec &c = m_vecLineColors[i];
        widget->color((int)c[0],(int)c[1],(int)c[2],(int)c[3]);
        const Vec &a = m_vecPtsProj[m_vecConnections[i].first];
        const Vec &b = m_vecPtsProj[m_vecConnections[i].second];
        widget->line(a.x(),a.y(),b.x(),b.y());
      }
    }
    if(m_bPointsVisible){
      for(unsigned int i=0;i<m_vecPtsProj.size();i++){
        const Vec &c = m_vecPtsColors[i];
        widget->color((int)c[0],(int)c[1],(int)c[2],(int)c[3]);
        const Vec &v = m_vecPtsProj[i];
        static const float r = 0.5;
        widget->ellipse(v.x()-r/2,v.y()-r/2,r,r);
      }
    }
  }

  // }}}
  void Object::render(Img32f *image) const{
    // {{{ open


    if(m_bTrianglesVisible && m_vecTriangles.size()){
      vector<P3> v;
      const VecArray &p = m_vecPtsProj;
      for(unsigned int i=0;i<m_vecTriangles.size();i++){
        const Triple &t = m_vecTriangles[i];
        v.push_back(P3(&p[t.a],&p[t.b],&p[t.c],&m_vecTriangleColors[i]));
      }
      std::sort(v.begin(),v.end());
      color(0,0,0,0);
      for(int i=0;i<(int)v.size();i++){
        fill((int)(*(v[i].color))[0],
             (int)(*(v[i].color))[1],
             (int)(*(v[i].color))[2],
             (int)(*(v[i].color))[3]);
        triangle( *image, (int)(*(v[i].a))[0],(int)(*(v[i].a))[1],
                  (int)(*(v[i].b))[0],(int)(*(v[i].b))[1],
                  (int)(*(v[i].c))[0],(int)(*(v[i].c))[1] );
      }    
    }
    
    if(m_bQuadsVisible && m_vecQuads.size()){
      vector<P4> v;
      const VecArray &p = m_vecPtsProj;
      for(unsigned int i=0;i<m_vecQuads.size();i++){
        const Quadruple &t = m_vecQuads[i];
        v.push_back(P4(&p[t.a],&p[t.b],&p[t.c],&p[t.d],&m_vecQuadColors[i]));
      }
      std::sort(v.begin(),v.end());
      color(0,0,0,0);
      for(int i=0;i<(int)v.size();i++){
        fill((int)(*(v[i].color))[0],
             (int)(*(v[i].color))[1],
             (int)(*(v[i].color))[2],
             (int)(*(v[i].color))[3]);
        triangle(*image,(int)(*(v[i].a))[0],(int)(*(v[i].a))[1],
                 (int)(*(v[i].b))[0],(int)(*(v[i].b))[1],
                 (int)(*(v[i].c))[0],(int)(*(v[i].c))[1] );
        triangle(*image,(int)(*(v[i].c))[0],(int)(*(v[i].c))[1],
                 (int)(*(v[i].d))[0],(int)(*(v[i].d))[1],
                 (int)(*(v[i].a))[0],(int)(*(v[i].a))[1] );
      }    
    }
  

    if(m_bLinesVisible){
      for(unsigned int i=0;i<m_vecConnections.size();i++){
        color(200,200,200);
        const Vec &a = m_vecPtsProj[m_vecConnections[i].first];
        const Vec &b = m_vecPtsProj[m_vecConnections[i].second];
        line(*image,
             (int)(a.x()),
             (int)(a.y()),
             (int)(b.x()),
             (int)(b.y()) );
      }
    }
    if(m_bPointsVisible){
      for(unsigned int i=0;i<m_vecPtsProj.size();i++){
        color(255,0,0);
        const Vec &v = m_vecPtsProj[i];
        pix(*image,(int)(v.x()),(int)(v.y()));
        /*
            circle(*image,
            (int)(v.x()),
            (int)(v.y()),
            2);
        */
        //      printf("drawed a point at %d %d \n",(int)v.x(),(int)v.y());
      }    
    }
  }

  // }}}
 


  
  bool Object::check() const{
    // {{{ open
#define TEST_VAL(X,MESSAGE) if(!(X)) { printf("Error: %s \n",MESSAGE); ok=false; }
#define TEST_VAL_IDX(X,MESSAGE,IDX) if(!(X)) { printf("Error: %s (index:%d)\n",MESSAGE,IDX); ok=false; }
    const VecArray &ps = m_vecPtsOrig;
    const std::vector<Tuple> &ls = m_vecConnections;
    const std::vector<Triple> &ts = m_vecTriangles;
    const std::vector<Quadruple> qs = m_vecQuads;

    const VecArray &pc = m_vecPtsColors;    
    const VecArray &lc = m_vecLineColors;
    const VecArray &tc = m_vecTriangleColors;    
    const VecArray &qc = m_vecQuadColors;    

    bool ok = true;
    
    TEST_VAL(pc.size()==ps.size(),"invalid count of point colors");
    TEST_VAL(lc.size()==ls.size(),"invalid count of line colors");
    TEST_VAL(tc.size()==ts.size(),"invalid count of triangle colors");
    TEST_VAL(qc.size()==qs.size(),"invalid count of quad colors");
    
    int np = (int)ps.size();
    int nl = (int)ls.size();
    int nt = (int)ts.size();
    int nq = (int)qs.size();
    
    for(int i=0;i<nl;i++){
      TEST_VAL_IDX(ls[i].first<np,"invalid point reference in lines first point",i);
      TEST_VAL_IDX(ls[i].second<np,"invalid point reference in lines second point",i);
    }
    for(int i=0;i<nt;i++){
      TEST_VAL_IDX(ts[i].a<np,"invalid point reference in triangles point a",i);
      TEST_VAL_IDX(ts[i].b<np,"invalid point reference in triangles point b",i);
      TEST_VAL_IDX(ts[i].c<np,"invalid point reference in triangles point b",i);
    }
    for(int i=0;i<nq;i++){
      TEST_VAL_IDX(qs[i].a<np,"invalid point reference in quads point a",i);
      TEST_VAL_IDX(qs[i].b<np,"invalid point reference in quads point b",i);
      TEST_VAL_IDX(qs[i].c<np,"invalid point reference in quads point c",i);
      TEST_VAL_IDX(qs[i].d<np,"invalid point reference in quads point d",i);
    }
    return ok;
  }

  // }}}
}
