#include <iclBorderedObject.h>
#include <stack>
#include <algorithm>
#include <iclImg.h>
#include <iclImgChannel.h>
#include <iclDrawWidget.h>
#include <vector>
#include <iclQuick.h>

using std::vector;

namespace icl{
  bool BorderedObject::VISUALIZE_BORDERS = true;

  BorderedObject::BorderedObject(){
    // {{{ open

    m_vecBorders.reserve(MAX_BORDERS);
    m_poConvexHullLineStrip = addNewBorder();
  }

  // }}}
  
  BorderedObject::~BorderedObject(){
    /// Borders are objects and might not be deleted!
  }
  
  BorderedObject::LineStrip *BorderedObject::addNewBorder(){
    // {{{ open

    m_vecBorders.push_back(LineStrip());
    return &m_vecBorders[m_vecBorders.size()-1];
  }

  // }}}
  
  void BorderedObject::project(const Mat &m){
    // {{{ open

    Object::project(m);
    updateBorders();
  }

  // }}}
  
  void BorderedObject::updateBorders(){
    // {{{ open
    calculateConvexHull(*m_poConvexHullLineStrip);
  }

  // }}}
  
  void BorderedObject::render(ICLDrawWidget *widget) const{
    // {{{ open

    Object::render(widget);
    
    if (VISUALIZE_BORDERS){
      widget->color(0,255,0,255);
      widget->fill(0,0,0,0);
      const LineStripArray &borders = getBorders();
      for(unsigned int i=0;i<borders.size();++i){
        const LineStrip &b = borders[i];
        for(unsigned int j=0;j<b.size();j++){
          const Vec *v = b[j];
          widget->ellipse(v->x()-1,v->y()-1,1,1);
        }
      }
    }

  }

  // }}}

  void BorderedObject::render(Img32f *image) const{
    // {{{ open

    Object::render(image);
    if (VISUALIZE_BORDERS){
      /// Vis into the image
    }
  }

  // }}}
  
  bool BorderedObject::check() const{
    // {{{ open

    bool checkParent = Object::check();
    
    // Check the lineStrip of the convex border here!
    return checkParent;
  }

  // }}}

  namespace{
    struct PointWithAngle{
      // {{{ open
      static float dxy(const Vec &a,const Vec &b){
        return (float)pow(a.x()-b.x(),2)+pow(a.y()-b.y(),2);
      }
      PointWithAngle():idx(0),angle(0),len(0){}
      PointWithAngle(int idx, float angle):
        idx(idx),angle(angle),len(0){}
      
      PointWithAngle(int idx, const Vec &v, const Vec &p0):
        idx(idx),angle(atan2(v.y()-p0.y(),v.x()-p0.x())),len(dxy(v,p0)){
      }
      
      inline bool operator<(const PointWithAngle &pt) const{
        if(angle == pt.angle){
          return len < pt.len;
        }
        return angle < pt.angle;
      }
      int idx;
      float angle;
      float len;
    }; 

    // }}}
    typedef std::stack<PointWithAngle> Stack;
    
    inline float compute_area(const Vec &a, const Vec &b, const Vec &c){
      // {{{ open

      return (b[0] - a[0]) * (c[1] - a[1]) -
      (c[0] - a[0]) * (b[1] - a[1]);
    }

    // }}}
    inline bool is_left( const Vec &a, const Vec &b, const Vec &c){
      // {{{ open

      /*---------------------------------------------------------------------
          Returns true iff c is strictly to the left of the directed
          line through a to b.
          ---------------------------------------------------------------------*/
      return  compute_area( a, b, c ) > 0;
    }

    // }}}
    inline void sort_by_angle(vector<PointWithAngle> &S){
      // {{{ open

      std::sort(S.begin(),S.end());
    }

    // }}}
    inline int find_upper_left(const VecArray &p,int n){
      // {{{ open

      int iUpper = 0;
      float yUpper = p[0].y();
      float xUpper = p[0].x();
      for(int i=1;i<n;i++){
        float y = p[i].y();
        float x = p[i].x();
        if(y < yUpper || y == yUpper && x < xUpper){
          yUpper = y;
          xUpper = x;
          iUpper = i;
        }
      }
      return iUpper;
    }

    // }}}
    inline void create_angle_list(vector<PointWithAngle> &S, VecArray &p, int n, Vec &p0, int iUpper){
      // {{{ open
      
      for(int i=0;i<n;i++){
        S[i] = PointWithAngle(i,p[i],p0);
      }
      S[iUpper]=PointWithAngle(iUpper,0);
    }     
    // }}}
  }
  
  void BorderedObject::calculateConvexHull(BorderedObject::LineStrip &dst){
    // {{{ open
    
    dst.clear();
    
    VecArray &p = Object::getPointsProj();
    int n = (int)p.size();
    
    vector<PointWithAngle> S(n);
    vector<bool> hullPts(n);
    
    int iUpper = find_upper_left(p,n);
    Vec p0 = p[iUpper];
    
    create_angle_list(S,p,n,p0,iUpper);
    sort_by_angle(S);
    
    
    
    Stack STACK;
    STACK.push(S[0]);
    STACK.push(S[1]);
    
    hullPts[S[0].idx] = hullPts[S[1].idx] = true;
    
    for(int i=2;i<n;){
      if(STACK.empty()){
        printf("stack is empty, this may not occur! \n");
        return;
      }
      PointWithAngle P1 = STACK.top(); 
      STACK.pop();
      if(STACK.empty()){
        printf("stack is empty after calling pop(), this may not occur! at idx %d/%d\n",i,n-1);
        return;
      }
      PointWithAngle P2 = STACK.top();
      PointWithAngle &SI = S[i];

      if(is_left( p[P2.idx], p[P1.idx], p[SI.idx])){
        STACK.push(P1);
        STACK.push(SI);
        i++;
        if((int)hullPts.size() <= SI.idx) printf("err 1!");
        hullPts[SI.idx]=true;
      }else{
        if((int)hullPts.size() <= P1.idx) printf("err 2!");
        hullPts[P1.idx]=false;
      }
    }

    for(int i=0;i<n;i++){
      int idx = S[i].idx;
      if(hullPts[idx]){
        dst.push_back(&p[idx]);
      }
    }
  }

  // }}}

} 
 
