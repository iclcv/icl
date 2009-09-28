#include <iclDrawWidget.h>
#include <iclPaintEngine.h>
#include <iclImgBase.h>
#include <iclGLTextureMapBaseImage.h>

using std::string;
namespace icl{
  
  
  /// internally used classes
  struct ICLDrawWidget::State{
    // {{{  struct
    State(){}
    bool aa;             // antializing on
    bool rel;            // relative or absolut coords
    Rect rect;           // current image rect
    Size size;           // current drawing widget size
    Size imsize;         // current image size
    unsigned char bg[4]; // background color
    QSizeF symsize;    
  };

  // }}}

  struct ICLDrawWidget::DrawCommand{
    // {{{ open struct

    virtual ~DrawCommand(){}
    virtual void exec(PaintEngine *e, State* s){
      (void)e; (void)s; 
      printf("drawCommand :: exec \n");
    }
  };

  // }}}
  
  // {{{ abstract commands (intelligent, 2f, 3f 4f)

  class IntelligentDrawCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open
  protected:
    Point tP(float x, float y, ICLDrawWidget::State *s){
      return Point(tX(x,s),tY(y,s));
    } 
    Point tP(const Point32f &p, ICLDrawWidget::State *s){
      return Point(tX(p.x,s),tY(p.y,s));
    }  
    Rect tR(float x, float y, float w, float h, ICLDrawWidget::State *s){
      Point a = tP(x,y,s);
      Point b = tP(x+w,y+h,s);
      return Rect(a,Size(b.x-a.x,b.y-a.y));
    }
    Size tS(float w, float h, ICLDrawWidget::State *s){
      if(s->rel)
        return Size((int)(w*s->rect.width),(int)(h*s->rect.height));
      else
        return Size((int)((w*s->rect.width)/s->imsize.width),(int)((h*s->rect.height)/s->imsize.height));
    }
    int tX(float x, ICLDrawWidget::State *s){
      return (int)tXF(x,s);
    }
    int tY(float x, ICLDrawWidget::State *s){
      return (int)tYF(x,s);
    }
    
    float tXF(float x, ICLDrawWidget::State *s){
      if(s->rel)
        return tmb(x,s->rect.width,s->rect.x);
      else
        return t(x, s->imsize.width, s->rect.width, 0, s->rect.x);
    }
    float tYF(float y, ICLDrawWidget::State *s){
      if(s->rel)
        return tmb(y,s->rect.height,s->rect.y);
      else
        return t(y, s->imsize.height, s->rect.height, 0, s->rect.y);
    }
  private:
    inline float tmb(float x, float m, float b){
      return m * x + b;
    }
    inline float t(float x, float dx, float dy, float xmin, float ymin){
      return (dy / dx) * (x - xmin) + ymin;
    }
  };
  // }}}
  
  class DrawCommand2F : public IntelligentDrawCommand{
    // {{{ open

  public:
    DrawCommand2F(float a, float b):m_fA(a),m_fB(b){}
  protected:
    float m_fA, m_fB;
  };

  // }}}
  
  class DrawCommand3F : public DrawCommand2F{
    // {{{ open
  public:
    DrawCommand3F(float a, float b, float c): 
      DrawCommand2F(a,b),m_fC(c){}
  protected:
    float m_fC;
  };

  // }}}

  class DrawCommand4F : public DrawCommand3F{
    // {{{ open

  public:
    DrawCommand4F(float a, float b, float c, float d):
      DrawCommand3F(a,b,c),m_fD(d){}
  protected:
    float m_fD;
  };

  // }}}
 
  class DrawCommand6F : public DrawCommand4F{
    // {{{ open
    
  public:
    DrawCommand6F(float a, float b, float c, float d,float e,float f):
      DrawCommand4F(a,b,c,d),m_fE(e),m_fF(f){}
  protected:
    float m_fE;
    float m_fF;
  };

  // }}}

  class DrawCommand8F : public DrawCommand6F{
    // {{{ open
    
  public:
    DrawCommand8F(float a, float b, float c, float d,float e,float f,float g, float h):
      DrawCommand6F(a,b,c,d,e,f),m_fG(g),m_fH(h){}
  protected:
    float m_fG;
    float m_fH;
  };

  // }}}

  // }}}
  
  // {{{ geometric commands ( point, line, rect, ellipse )

  class PointCommand : public DrawCommand2F{
    // {{{ open

    public:
    PointCommand(float x, float y):DrawCommand2F(x,y){};
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      e->point(tP(m_fA,m_fB,s));
    }
  };

  // }}}

  class PointsCommand : public IntelligentDrawCommand{
    // {{{ open

    public:
    PointsCommand(const std::vector<Point> &pts, int xfac, int yfac, bool connectPoints, bool closeLoop):
      pts(pts),xfac(xfac),yfac(yfac),connectPoints(connectPoints),closeLoop(closeLoop){}
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      if(!pts.size()) return;
      if(connectPoints){
        if(pts.size() == 2){
          e->line(tP(pts[0].x/xfac,pts[0].y/yfac,s),tP(pts[1].x/xfac,pts[1].y/yfac,s));
          e->point(tP(pts[0].x/xfac,pts[0].y/yfac,s));
          e->point(tP(pts[1].x/xfac,pts[1].y/yfac,s));
          return;
        }
        ///  x----------x---------x------------x-------------x
        ///  0          1         2            3             4  n=5
        if(xfac==1 && yfac==1){
          for(unsigned int i=1;i<pts.size();i++){
            e->line(tP(pts[i-1].x,pts[i-1].y,s),tP(pts[i].x,pts[i].y,s));
            e->point(tP(pts[i].x,pts[i].y,s));
          }
          if(closeLoop){
            e->line(tP(pts[pts.size()-1].x,pts[pts.size()-1].y,s),tP(pts[0].x,pts[0].y,s));
          }
        }else{
          for(unsigned int i=1;i<pts.size();i++){
            e->line(tP(float(pts[i-1].x)/xfac,float(pts[i-1].y)/yfac,s),tP(float(pts[i].x)/xfac,float(pts[i].y)/yfac,s));
            e->point(tP(float(pts[i].x)/xfac,float(pts[i].y)/yfac,s));
          }
          if(connectPoints){
            e->line(tP(float(pts[pts.size()-1].x)/xfac,float(pts[pts.size()-1].y)/yfac,s),tP(float(pts[0].x)/xfac,float(pts[0].y)/yfac,s));
          }
        }
      }else{
        if(xfac==1 && yfac==1){
          for(unsigned int i=0;i<pts.size();i++){
            e->point(tP(pts[i].x,pts[i].y,s));
          }
        }else{
          for(unsigned int i=0;i<pts.size();i++){
            e->point(tP(float(pts[i].x)/xfac,float(pts[i].y)/yfac,s));
          }
        }
      }
    }
    std::vector<Point> pts;
    int xfac;
    int yfac;
    bool connectPoints;
    bool closeLoop;
  };

  // }}}


  class PolygonCommand : public IntelligentDrawCommand{
    // {{{ open

    public:
    PolygonCommand(const std::vector<Point32f> &pts):pts(pts),center(0,0){
      if(pts.size()){
        for(unsigned int i=0;i<pts.size();++i){
          center += pts[i];
        }
        center *= 1.0/pts.size();
      }
    }
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      if(pts.size()< 3) return;
      
      int colorSave[4];
      e->getColor(colorSave);
      e->color(0,0,0,0);
      
      Point c = tP(center.x,center.y,s);
      
      const unsigned int n = pts.size();
      for(unsigned int i=0; i<n;++i){
        unsigned int next =i+1; 
        if(next == n) next = 0;
        const Point32f &a = pts[i];
        const Point32f &b = pts[next];
        Point A = tP(a.x,a.y,s);
        Point B = tP(b.x,b.y,s);
        e->color(0,0,0,0);
        e->triangle(A,B,c);
        e->color(colorSave[0],colorSave[1],colorSave[2],colorSave[3]);
        e->line(A,B);
      }

      
    }
    std::vector<Point32f> pts;
    Point32f center;
  };

  // }}}


 class Points32fCommand : public IntelligentDrawCommand{
    // {{{ open

    public:
    Points32fCommand(const std::vector<Point32f> &pts, bool connectPoints, bool closeLoop):
      pts(pts),connectPoints(connectPoints),closeLoop(closeLoop){}
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      if(!pts.size()) return;
      if(connectPoints){
        if(pts.size() == 2){
          e->line(tP(pts[0].x,pts[0].y,s),tP(pts[1].x,pts[1].y,s));
          e->point(tP(pts[0].x,pts[0].y,s));
          e->point(tP(pts[1].x,pts[1].y,s));
          return;
        }
        ///  x----------x---------x------------x-------------x
        ///  0          1         2            3             4  n=5
        
        for(unsigned int i=1;i<pts.size();i++){
          e->line(tP(pts[i-1].x,pts[i-1].y,s),tP(pts[i].x,pts[i].y,s));
          e->point(tP(pts[i].x,pts[i].y,s));
        }
        if(closeLoop){
          e->line(tP(pts[pts.size()-1].x,pts[pts.size()-1].y,s),tP(pts[0].x,pts[0].y,s));
        }
      }else{
        for(unsigned int i=0;i<pts.size();i++){
          e->point(tP(pts[i].x,pts[i].y,s));
        }
      }
    }
    std::vector<Point32f> pts;
    int xfac;
    int yfac;
    bool connectPoints;
    bool closeLoop;
  };

  // }}}


  
  class LineCommand : public DrawCommand4F{
    // {{{ open

  public:
    LineCommand(float x1, float y1, float x2, float y2):
    DrawCommand4F(x1,y1,x2,y2){
    }
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      e->line(tP(m_fA,m_fB,s),tP(m_fC,m_fD,s));
    }
  };

  // }}}

  class RectCommand : public DrawCommand4F{
    // {{{ open
  public:
    RectCommand(float x, float y, float w, float h):
      DrawCommand4F(x,y,w,h){}
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      e->rect(tR(m_fA,m_fB,m_fC,m_fD,s));
    }
  };

  // }}}

  class TriangleCommand : public DrawCommand6F{
    // {{{ open
  public:
    TriangleCommand(float x1, float y1, float x2, float y2,float x3,float y3):
      DrawCommand6F(x1,y1,x2,y2,x3,y3){}
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      e->triangle(tP(m_fA,m_fB,s),tP(m_fC,m_fD,s),tP(m_fE,m_fF,s));
    }
  };
  
  // }}}

  class QuadCommand : public DrawCommand8F{
    // {{{ open
  public:
    QuadCommand(float x1, float y1, float x2, float y2,float x3,float y3,float x4,float y4):
      DrawCommand8F(x1,y1,x2,y2,x3,y3,x4,y4){}
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      e->quad(tP(m_fA,m_fB,s),tP(m_fC,m_fD,s),tP(m_fE,m_fF,s),tP(m_fG,m_fH,s));
    }
  };
  
  // }}}

  
  class EllipseCommand : public DrawCommand4F{
    // {{{ open
  public:
    EllipseCommand(float x, float y, float w, float h):
      DrawCommand4F(x,y,w,h){}
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      e->ellipse(tR(m_fA,m_fB,m_fC,m_fD,s));
    }
  };
 // }}}

  class GridCommand : public IntelligentDrawCommand{
    // {{{ open
    std::vector<Point32f> pts;
    int nx,ny;
    bool rowMajor;
  public:
    GridCommand(const Point32f *points, int nx, int ny, bool rowMajor):
      pts(points,points+nx*ny),nx(nx),ny(ny),rowMajor(rowMajor){
    }
    
    
    struct AnyData{
      std::vector<Point32f> &p;
      int wh;
      AnyData(std::vector<Point32f> & p, int wh):p(p),wh(wh){}
    };
    struct RowMajorData : public AnyData{
      RowMajorData(std::vector<Point32f> &p, int w):AnyData(p,w){}
      const Point32f &operator()(int x, int y) const { return p[x+wh*y]; }
    };
    struct ColMajorData : public AnyData{
      ColMajorData(std::vector<Point32f> &p, int h):AnyData(p,h){}
      const Point32f &operator()(int x, int y) const { return p[y+wh*x]; }
    };

    
    template<class Data>
    void exec_t(PaintEngine *e, ICLDrawWidget::State *s,const Data &data){
      for(int x=0;x<nx-1;++x){
        for(int y=0;y<ny-1;++y){
          Point a = tP(data(x,y),s);
          Point b = tP(data(x+1,y),s);
          Point c = tP(data(x,y+1),s);
          
          e->line(a,b);
          e->line(a,c);
        }
      }
      for(int x=0;x<nx-1;++x){
        Point a = tP(data(x,ny-1),s);
        Point b = tP(data(x+1,ny-1),s);
        e->line(a,b);
      }
      for(int y=0;y<ny-1;++y){
        Point a = tP(data(nx-1,y),s);
        Point b = tP(data(nx-1,y+1),s);
        e->line(a,b);
      }
    }
    
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      if(rowMajor){
        exec_t(e,s,RowMajorData(pts,nx));
      }else{
        exec_t(e,s,ColMajorData(pts,ny));
      }
    }
  };
  // }}}

  class SymCommand : public IntelligentDrawCommand{
    // {{{ open
  public:
    SymCommand(float x, float y, ICLDrawWidget::Sym s):
      m_fX(x),m_fY(y),m_eS(s){}
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      Rect r(tP(m_fX,m_fY,s),tS(s->symsize.width(),s->symsize.height(),s));
      r.x-=r.width/2;
      r.y-=r.height/2;
      switch(m_eS){
        case ICLDrawWidget::symRect:
          e->rect(r);
          break;
        case ICLDrawWidget::symCircle:
          e->ellipse(r);
          break;
        case ICLDrawWidget::symCross:
          e->line(r.ul(), r.lr());
          e->line(r.ll(), r.ur());
          break;
        case ICLDrawWidget::symPlus:
          e->line( Point( (r.x+r.right())/2, r.y ),
                   Point( (r.x+r.right())/2, r.bottom() ) );
          e->line( Point( r.x, (r.y+r.bottom())/2 ),
                   Point( r.right(), (r.y+r.bottom())/2 ) );
          break;
        case ICLDrawWidget::symTriangle:
          e->line( Point( (r.x+r.right())/2, r.y ), r.ul() );
          e->line( Point( (r.x+r.right())/2, r.y ), r.lr() );
          e->line( r.ll(), r.lr() );
          break;
      }
    }
    float m_fX, m_fY;
    ICLDrawWidget::Sym m_eS;
  };

  // }}}

  /** old 
  class ImageCommand : public DrawCommand4F{
    // {{{ open

  public:
    ImageCommand(ImgBase *image, float x, float y, float w, float h):
      DrawCommand4F(x,y,w,h), m_poImage(0){
      image->deepCopy(&m_poImage);
    }
    virtual ~ImageCommand(){
      if(m_poImage)delete m_poImage;
    }
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      e->image(Rect(tP(m_fA,m_fB,s),tS(m_fC, m_fD,s)),m_poImage,PaintEngine::Justify);
    }
    ImgBase *m_poImage;
  };

  // }}}
  **/
  class ImageCommand : public DrawCommand4F{
    // {{{ open

  public:
    ImageCommand(ImgBase *image, float x, float y, float w, float h):
      DrawCommand4F(x,y,w,h){
      m_poImage = new GLTextureMapBaseImage(image,false);
    }
    virtual ~ImageCommand(){
      if(m_poImage)delete m_poImage;
    }
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      (void)e;
      Rect r(tP(m_fA,m_fB,s),tS(m_fC, m_fD,s));
      m_poImage->drawTo(Rect(tP(m_fA,m_fB,s),tS(m_fC, m_fD,s)),s->size);
    }
    GLTextureMapBaseImage *m_poImage;
  };

  // }}}


  class TextCommand : public DrawCommand4F{
    // {{{ open

  public:
    TextCommand(std::string text, float x, float y, float w, float h, int fontsize):
      DrawCommand4F(x,y,w,h),text(text),fontsize(fontsize){
    }
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      int oldFontSize = e->getFontSize();

      e->fontsize(fontsize);
      if(m_fC == -1 || m_fD == -1){
        e->text(Rect(tP(m_fA,m_fB,s),tS(m_fC, m_fD,s)),text,PaintEngine::NoAlign);
      }else{
        e->text(Rect(tP(m_fA,m_fB,s),tS(m_fC, m_fD,s)),text,PaintEngine::Justify);
      }
      e->fontsize(oldFontSize);
    }
  protected:
    string text;
    int fontsize;
  };

  // }}}
  // }}}

  // {{{ state commands( (no)edge, (no)fill, abs, rel, clear, setimagesize)

 

  class EdgeCommand : public DrawCommand4F{
    // {{{ open
  public:
    EdgeCommand(int r, int g, int b, int alpha):
      DrawCommand4F(r,g,b,alpha){}
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      (void)s;
      e->color((int)m_fA,(int)m_fB,(int)m_fC,(int)m_fD);
    }
  };

  // }}}

  class FillCommand : public DrawCommand4F{
    // {{{ open
  public:
    FillCommand(int r, int g, int b, int alpha):
      DrawCommand4F(r,g,b,alpha){}
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      (void)s;
      e->fill((int)m_fA,(int)m_fB,(int)m_fC,(int)m_fD);
    }
  };

  // }}}
  
  class NoEdgeCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open
  public:
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      (void)s;
      e->color(0,0,0,0);
    }
  };

  // }}}
  
  class NoFillCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open
  public:
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      (void)s;
      e->fill(0,0,0,0);
    }
  };

  // }}}
  
  class AbsCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open

    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      (void)e;
      s->rel = false;
    }
  };

  // }}}

  class RelCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open

    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      (void)e;
      s->rel = true;
    }
  };

  // }}}
  
  class ClearCommand : public DrawCommand4F{
    // {{{ open
    
  public:
    ClearCommand(int r, int g, int b, int alpha):
      DrawCommand4F(r,g,b,alpha){}
    
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      int aiFill[4],aiLine[4];
      e->getFill(aiFill);
      e->getColor(aiLine);
      e->fill((int)m_fA,(int)m_fB,(int)m_fC,(int)m_fD);
      e->color(0,0,0,0);
      e->rect(Rect(0,0,s->size.width,s->size.height));
      e->fill(aiFill[0],aiFill[1],aiFill[2],aiFill[3]);
      e->color(aiLine[0],aiLine[1],aiLine[2],aiLine[3]);
    }
  };

  // }}}

  class SetImageSizeCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open
  public:
    SetImageSizeCommand(const Size &s):m_oSize(s){}
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      (void)e;
      s->imsize = m_oSize;
    }
  protected:
    Size m_oSize;
  };

  // }}}

  class SymSizeCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open

  public:
    SymSizeCommand(float w, float h) : m_fW(w), m_fH(h){}
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      (void)e;
      s->symsize = QSizeF(m_fW,m_fH);
    }
  protected:
    float m_fW, m_fH;
  };

    // }}}


  class LineWidthCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open

  public:
    LineWidthCommand(float w):m_w(w){}
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      e->linewidth(m_w);
    }
  protected:
    float m_w;
  };

    // }}}

  class PointSizeCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open
    
  public:
    PointSizeCommand(float s):m_s(s){}
    virtual void exec(PaintEngine *e, ICLDrawWidget::State *s){
      e->pointsize(m_s);
    }
  protected:
    float m_s;
  };

  // }}}

  
  // }}}
 
  ICLDrawWidget::ICLDrawWidget(QWidget *poParent):
    // {{{ open

    ICLWidget(poParent){
    m_poState = new State();
    m_poState->aa = false;
    m_poState->rel = false;
    m_poState->rect = getImageRect();
    m_poState->size = Size(width(),height());
    m_poState->imsize = getImageSize();
    m_poState->symsize = QSizeF(5,5);
    
    memset(m_poState->bg,0,4*sizeof(unsigned char));

    setShowNoImageWarnings(false);
  }

    // }}}
  ICLDrawWidget::~ICLDrawWidget(){
    lock();
    reset();
    if(m_poState)delete m_poState;
    unlock();
  }

  // {{{ commands: line, sym, rel, ...
  void ICLDrawWidget::image(ImgBase *image,float x, float y, float w, float h){
    m_vecCommands.push_back(new ImageCommand(image,x,y,w,h));
  }
 
  void ICLDrawWidget::text(string text, float x, float y, float w, float h, int fontsize){
    m_vecCommands.push_back(new TextCommand(text,x,y,w,h,fontsize));
  } 

  void ICLDrawWidget::line(float x1, float y1, float x2, float y2){
    m_vecCommands.push_back(new LineCommand(x1,y1,x2,y2));
  }

  void ICLDrawWidget::sym(float x, float y, Sym s){
    m_vecCommands.push_back(new SymCommand(x,y,s));
  }
  void ICLDrawWidget::symsize(float w, float h){
    m_vecCommands.push_back(new SymSizeCommand(w,h==-1? w : h));
  }

  void ICLDrawWidget::linewidth(float w){
    m_vecCommands.push_back(new LineWidthCommand(w));
  }

  void ICLDrawWidget::pointsize(float s){
    m_vecCommands.push_back(new PointSizeCommand(s));
  }


  void ICLDrawWidget::point(float x, float y){
    m_vecCommands.push_back(new PointCommand(x,y));
  }
  void ICLDrawWidget::points(const std::vector<Point> &pts, int xfac, int yfac){
    m_vecCommands.push_back(new PointsCommand(pts,xfac,yfac,false,false));
  }
  void ICLDrawWidget::points(const std::vector<Point32f> &pts){
    m_vecCommands.push_back(new Points32fCommand(pts,false,false));
  }

  void  ICLDrawWidget::linestrip(const std::vector<Point> &pts, bool closeLoop, int xfac, int yfac){
    m_vecCommands.push_back(new PointsCommand(pts,xfac,yfac,true,closeLoop));
  }
  void  ICLDrawWidget::linestrip(const std::vector<Point32f> &pts, bool closeLoop){
    m_vecCommands.push_back(new Points32fCommand(pts,true,closeLoop));
  }

  void ICLDrawWidget::polygon(const std::vector<Point32f> &ps){
    m_vecCommands.push_back(new PolygonCommand(ps));
  }
  
  void ICLDrawWidget::grid(const Point32f *points, int nx, int ny, bool rowMajor){
    m_vecCommands.push_back(new GridCommand(points,nx,ny,rowMajor));
  }

  void ICLDrawWidget::abs(){
    m_vecCommands.push_back(new AbsCommand());
  }
  void ICLDrawWidget::rel(){
    m_vecCommands.push_back(new RelCommand());
  }
  void ICLDrawWidget::rect(float x, float y, float w, float h){
    m_vecCommands.push_back(new RectCommand(x,y,w,h));
  }
  void ICLDrawWidget::rect(Rect r){
    m_vecCommands.push_back(new RectCommand(r.x,r.y,r.width,r.height));
  }
  void ICLDrawWidget::triangle(float x1, float y1, float x2, float y2, float x3, float y3){
    m_vecCommands.push_back(new TriangleCommand(x1,y1,x2,y2,x3,y3));
  }
  void ICLDrawWidget::quad(float x1, float y1, float x2, float y2, float x3, float y3,float x4,float y4){
    m_vecCommands.push_back(new QuadCommand(x1,y1,x2,y2,x3,y3,x4,y4));
  }
  void ICLDrawWidget::ellipse(float x, float y, float w, float h){
    m_vecCommands.push_back(new EllipseCommand(x,y,w,h));
  }
  void ICLDrawWidget::color(int r, int g, int b, int alpha){
    m_vecCommands.push_back(new EdgeCommand(r,g,b,alpha));
  }
  void ICLDrawWidget::fill(int r, int g, int b, int alpha){
    m_vecCommands.push_back(new FillCommand(r,g,b,alpha));
  }
  void ICLDrawWidget::nocolor(){
    m_vecCommands.push_back(new NoEdgeCommand());
  }
  void ICLDrawWidget::nofill(){
    m_vecCommands.push_back(new NoFillCommand());
  }
  void ICLDrawWidget::setPseudoImage(Size s){
    m_vecCommands.push_back(new SetImageSizeCommand(s));
  }
  void ICLDrawWidget::clear(int r, int g, int b, int alpha){
    m_vecCommands.push_back(new ClearCommand(r,g,b,alpha));
  }
  void ICLDrawWidget::reset(){
    for(std::vector<DrawCommand*>::iterator it = m_vecCommands.begin();it!= m_vecCommands.end();++it){
      delete (*it);
    }
    m_vecCommands.clear();
  }

    // }}}

  void ICLDrawWidget::initializeCustomPaintEvent(PaintEngine *e){
    // {{{ open
    (void)e;
  }

    // }}}
  void ICLDrawWidget::finishCustomPaintEvent(PaintEngine *e){
    // {{{ open
    (void)e;
  }

    // }}}
  void ICLDrawWidget::customPaintEvent(PaintEngine *e){
    // {{{ open
    m_oCommandMutex.lock();
    //Rect r = getImageRect();
    m_poState->aa = false;
    m_poState->rel = false;
    m_poState->rect = getImageRect(true);
    m_poState->size = Size(width(),height());
    m_poState->imsize = getImageSize(true);
    m_poState->symsize = QSizeF(5,5);
    memset(m_poState->bg,0,4*sizeof(unsigned char));
    e->font("Arial",8,PaintEngine::DemiBold);
    e->color(255,255,255);
    e->fill(0,0,0);
    initializeCustomPaintEvent(e);
    for(std::vector<DrawCommand*>::iterator it = m_vecCommands.begin();it!= m_vecCommands.end();++it){
      (*it)->exec(e,m_poState);
    }
    finishCustomPaintEvent(e);
    m_oCommandMutex.unlock();
  }

    // }}}

}
