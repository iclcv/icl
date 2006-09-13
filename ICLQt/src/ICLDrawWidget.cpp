#include "ICLDrawWidget.h"
#include "GLPaintEngine.h"
#include "ImgI.h"

namespace icl{
  
  /// internally used classes
  struct ICLDrawWidget::State{
    // {{{  struct

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
    virtual void exec(GLPaintEngine *e, State* s){
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

  // }}}
  
  // {{{ geometric commands ( point, line, rect, ellipse )

  class PointCommand : public DrawCommand2F{
    // {{{ open

    public:
    PointCommand(float x, float y):DrawCommand2F(x,y){};
    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
      e->point(tP(m_fA,m_fB,s));
    }
  };

  // }}}

  class LineCommand : public DrawCommand4F{
    // {{{ open

  public:
    LineCommand(float x1, float y1, float x2, float y2):
    DrawCommand4F(x1,y1,x2,y2){
    }
    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
      e->line(tP(m_fA,m_fB,s),tP(m_fC,m_fD,s));
    }
  };

  // }}}

  class RectCommand : public DrawCommand4F{
    // {{{ open
  public:
    RectCommand(float x, float y, float w, float h):
      DrawCommand4F(x,y,w,h){}
    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
      e->rect(tR(m_fA,m_fB,m_fC,m_fD,s));
    }
  };

  // }}}

  class EllipseCommand : public DrawCommand4F{
    // {{{ open
  public:
    EllipseCommand(float x, float y, float w, float h):
      DrawCommand4F(x,y,w,h){}
    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
      e->ellipse(tR(m_fA,m_fB,m_fC,m_fD,s));
    }
  };
 // }}}

  class SymCommand : public IntelligentDrawCommand{
    // {{{ open
  public:
    SymCommand(float x, float y, ICLDrawWidget::Sym s):
      m_fX(x),m_fY(y),m_eS(s){}
    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
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

  class ImageCommand : public DrawCommand4F{
    // {{{ open

  public:
    ImageCommand(ImgI *image, float x, float y, float w, float h):
      DrawCommand4F(x,y,w,h), m_poImage(0){
      ensureCompatible(&m_poImage,image);
      image->deepCopy(m_poImage);
    }
    virtual ~ImageCommand(){
      if(m_poImage)delete m_poImage;
    }
    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
      e->image(Rect(tP(m_fA,m_fB,s),tS(m_fC, m_fD,s)),m_poImage,GLPaintEngine::Justify);
    }
    ImgI *m_poImage;
  };

  // }}}

  // }}}

  // {{{ state commands( (no)edge, (no)fill, abs, rel, clear, setimagesize)

 

  class EdgeCommand : public DrawCommand4F{
    // {{{ open
  public:
    EdgeCommand(int r, int g, int b, int alpha):
      DrawCommand4F(r,g,b,alpha){}
    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
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
    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
      (void)s;
      e->fill((int)m_fA,(int)m_fB,(int)m_fC,(int)m_fD);
    }
  };

  // }}}
  
  class NoEdgeCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open
  public:
    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
      (void)s;
      e->color(0,0,0,0);
    }
  };

  // }}}
  
  class NoFillCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open
  public:
    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
      (void)s;
      e->fill(0,0,0,0);
    }
  };

  // }}}
  
  class AbsCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open

    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
      (void)e;
      s->rel = false;
    }
  };

  // }}}

  class RelCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open

    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
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
    
    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
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
    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
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
    virtual void exec(GLPaintEngine *e, ICLDrawWidget::State *s){
      (void)e;
      s->symsize = QSizeF(m_fW,m_fH);
    }
  protected:
    float m_fW, m_fH;
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
    m_poState->size = Size(w(),h());
    m_poState->imsize = getImageSize();
    m_poState->symsize = QSizeF(5,5);
    
    memset(m_poState->bg,0,4*sizeof(unsigned char));
  }

    // }}}
  
  // {{{ commands: line, sym, rel, ...
  void ICLDrawWidget::image(ImgI *image,float x, float y, float w, float h){
    m_vecCommands.push_back(new ImageCommand(image,x,y,w,h));
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

  void ICLDrawWidget::point(float x, float y){
    m_vecCommands.push_back(new PointCommand(x,y));
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
  void ICLDrawWidget::ellipse(float x, float y, float w, float h){
    m_vecCommands.push_back(new EllipseCommand(x,y,w,h));
  }
  void ICLDrawWidget::edge(int r, int g, int b, int alpha){
    m_vecCommands.push_back(new EdgeCommand(r,g,b,alpha));
  }
  void ICLDrawWidget::fill(int r, int g, int b, int alpha){
    m_vecCommands.push_back(new FillCommand(r,g,b,alpha));
  }
  void ICLDrawWidget::noedge(){
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

  void ICLDrawWidget::initializeCustomPaintEvent(GLPaintEngine *e){
    // {{{ open
    (void)e;
  }

    // }}}
  void ICLDrawWidget::finishCustomPaintEvent(GLPaintEngine *e){
    // {{{ open
    (void)e;
  }

    // }}}
  void ICLDrawWidget::customPaintEvent(GLPaintEngine *e){
    // {{{ open
    m_oCommandMutex.lock();
    Rect r = getImageRect();
    m_poState->aa = false;
    m_poState->rel = false;
    m_poState->rect = getImageRect();
    m_poState->size = Size(w(),h());
    m_poState->imsize = getImageSize();
    m_poState->symsize = QSizeF(5,5);
    memset(m_poState->bg,0,4*sizeof(unsigned char));

    initializeCustomPaintEvent(e);
    for(std::vector<DrawCommand*>::iterator it = m_vecCommands.begin();it!= m_vecCommands.end();++it){
      (*it)->exec(e,m_poState);
    }
    finishCustomPaintEvent(e);
    m_oCommandMutex.unlock();
  }

    // }}}

}
