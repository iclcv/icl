#include <iclObject.h>
#include <iclDrawWidget.h>
#include <iclQuick.h>
#include <iclImg.h>
#include <iclImgChannel.h>

namespace icl{
  
  Object::Object():T(Mat::id()){ 
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
  void Object::render(ICLDrawWidget *widget) const{
    // {{{ open

    //    const_cast<Object*>(this)->tintConvexHull();
    
    Size s = widget->getImageSize();
    widget->fill(0,0,0,0);
    for(unsigned int i=0;i<m_vecConnections.size();i++){
      const Vec &c = m_vecLineColors[i];
      widget->color((int)c[0],(int)c[1],(int)c[2],(int)c[3]);
      const Vec &a = m_vecPtsProj[m_vecConnections[i].first];
      const Vec &b = m_vecPtsProj[m_vecConnections[i].second];
      widget->line(a.x(),a.y(),b.x(),b.y());
    }
    for(unsigned int i=0;i<m_vecPtsProj.size();i++){
      const Vec &c = m_vecPtsColors[i];
      widget->color((int)c[0],(int)c[1],(int)c[2],(int)c[3]);
      const Vec &v = m_vecPtsProj[i];
      static const float r = 0.5;
      widget->ellipse(v.x()-r/2,v.y()-r/2,r,r);
    }
  }

  // }}}
  void Object::render(Img32f *image) const{
    // {{{ open

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
    for(unsigned int i=0;i<m_vecPtsProj.size();i++){
      color(255,0,0);
      const Vec &v = m_vecPtsProj[i];
      circle(*image,
             (int)(v.x()),
             (int)(v.y()),
             2);
      //      printf("drawed a point at %d %d \n",(int)v.x(),(int)v.y());
    }    
  }

  // }}}
 
}
