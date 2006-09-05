#include "ICLDrawWidget.h"
#include <QPainter>

namespace icl{
  
  /// internally used classes
  struct ICLDrawWidget::State{
    // {{{ open struct

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
    virtual void exec(QPainter *p, State* s){
      (void)p; (void)s; 
      printf("drawCommand :: exec \n");
    }
  };

  // }}}
  
  // {{{ abstract commands (intelligent, 2f, 3f 4f)

  class IntelligentDrawCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open
  protected:
    QPointF tP(float x, float y, ICLDrawWidget::State *s){
      return QPointF(tX(x,s),tY(y,s));
    }  
    QRectF tR(float x, float y, float w, float h, ICLDrawWidget::State *s){
      QRectF r;
      r.setTopLeft(tP(x,y,s));
      r.setBottomRight(tP(x+w,y+h,s));
      return r;
    }
    QSizeF tS(float w, float h, ICLDrawWidget::State *s){
      if(s->rel)
        return QSizeF(w*s->rect.width,h*s->rect.height);
      else
        return QSizeF((w*s->rect.width)/s->imsize.width,(h*s->rect.height)/s->imsize.height);
    }
    float tX(float x, ICLDrawWidget::State *s){
      if(s->rel)
        return tmb(x,s->rect.width,s->rect.x);
      else
        return t(x, s->imsize.width, s->rect.width, 0, s->rect.x);
    }
    float tY(float y, ICLDrawWidget::State *s){
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
    virtual void exec(QPainter *p, ICLDrawWidget::State *s){
      p->drawPoint(tP(m_fA,m_fB,s));
    }
  };

  // }}}

  class LineCommand : public DrawCommand4F{
    // {{{ open

  public:
    LineCommand(float x1, float y1, float x2, float y2):
    DrawCommand4F(x1,y1,x2,y2){
    }
    virtual void exec(QPainter *p, ICLDrawWidget::State *s){
      p->drawLine(tP(m_fA,m_fB,s),tP(m_fC,m_fD,s));
    }
  };

  // }}}

  class RectCommand : public DrawCommand4F{
    // {{{ open
  public:
    RectCommand(float x, float y, float w, float h):
      DrawCommand4F(x,y,w,h){}
    virtual void exec(QPainter *p, ICLDrawWidget::State *s){
      p->drawRect(tR(m_fA,m_fB,m_fC,m_fD,s));
    }
  };

  // }}}

  class EllipseCommand : public DrawCommand4F{
    // {{{ open
  public:
    EllipseCommand(float x, float y, float w, float h):
      DrawCommand4F(x,y,w,h){}
    virtual void exec(QPainter *p, ICLDrawWidget::State *s){
      p->drawEllipse(tR(m_fA,m_fB,m_fC,m_fD,s));
    }
  };

  // }}}

  class SymCommand : public IntelligentDrawCommand{
    // {{{ open
  public:
    SymCommand(float x, float y, ICLDrawWidget::Sym s):
      m_fX(x),m_fY(y),m_eS(s){}
    virtual void exec(QPainter *p, ICLDrawWidget::State *s){
      QRectF r(tP(m_fX,m_fY,s),tS(s->symsize.width(),s->symsize.height(),s));
      r.translate(-r.width()/2,-r.height()/2);
      switch(m_eS){
        case ICLDrawWidget::symRect:
          p->drawRect(r);
          break;
        case ICLDrawWidget::symCircle:
          p->drawEllipse(r);
          break;
        case ICLDrawWidget::symCross:
          p->drawLine(r.topLeft(), r.bottomRight());
          p->drawLine(r.bottomLeft(), r.topRight());
          break;
        case ICLDrawWidget::symPlus:
          p->drawLine( QPointF( (r.x()+r.right())/2, r.y() ),
                               QPointF( (r.x()+r.right())/2, r.bottom() ) );
          p->drawLine( QPointF( r.x(), (r.y()+r.bottom())/2 ),
                               QPointF( r.right(), (r.y()+r.bottom())/2 ) );
          break;
        case ICLDrawWidget::symTriangle:
          p->drawLine( QPointF( (r.x()+r.right())/2, r.y() ), r.bottomLeft() );
          p->drawLine( QPointF( (r.x()+r.right())/2, r.y() ), r.bottomRight() );
          p->drawLine( r.bottomLeft(), r.bottomRight() );
          break;
      }
    }
    float m_fX, m_fY;
    ICLDrawWidget::Sym m_eS;
  };

  // }}}

  // }}}

  // {{{ state commands( (no)edge, (no)fill, abs, rel, clear, setimagesize)

  class EdgeCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open
  public:
    EdgeCommand(int r, int g, int b, int alpha):
      m_oColor(r,g,b,alpha){}
    virtual void exec(QPainter *p, ICLDrawWidget::State *s){
      (void)s;
      p->setPen(m_oColor);
    }
    QColor m_oColor;
  };

  // }}}

  class FillCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open
  public:
    FillCommand(int r, int g, int b, int alpha):
      m_oColor(r,g,b,alpha){}
    virtual void exec(QPainter *p, ICLDrawWidget::State *s){
      (void)s;
      p->setBrush(m_oColor);
    }
    QColor m_oColor;
  };

  // }}}
  
  class NoEdgeCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open
  public:
    virtual void exec(QPainter *p, ICLDrawWidget::State *s){
      (void)s;
      p->setPen(Qt::NoPen);
    }
  };

  // }}}
  
  class NoFillCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open
  public:
    virtual void exec(QPainter *p, ICLDrawWidget::State *s){
      (void)s;
      p->setBrush(Qt::NoBrush);
    }
  };

  // }}}
  
  class AbsCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open

    virtual void exec(QPainter *p, ICLDrawWidget::State *s){
      (void)p;
      s->rel = false;
    }
  };

  // }}}

  class RelCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open

    virtual void exec(QPainter *p, ICLDrawWidget::State *s){
      (void)p;
      s->rel = true;
    }
  };

  // }}}
  
  class ClearCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open
    
  public:
    ClearCommand(int r, int g, int b, int alpha):
      m_oColor(r,g,b,alpha){}
    
    virtual void exec(QPainter *p, ICLDrawWidget::State *s){
      p->save();
      p->setBrush(m_oColor);
      p->drawRect(0,0,s->size.width,s->size.height);
      p->restore();
    }
  protected:
    QColor m_oColor;
  };

  // }}}

  class SetImageSizeCommand : public ICLDrawWidget::DrawCommand{
    // {{{ open
  public:
    SetImageSizeCommand(const Size &s):m_oSize(s){}
    virtual void exec(QPainter *p, ICLDrawWidget::State *s){
      (void)p;
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
    virtual void exec(QPainter *p, ICLDrawWidget::State *s){
      (void)p;
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

  void ICLDrawWidget::initializeCustomPaintEvent(QPainter *poPainter){
    // {{{ open

    (void)poPainter;
  }

    // }}}
  void ICLDrawWidget::finishCustomPaintEvent(QPainter *poPainter){
    // {{{ open

    (void)poPainter;
  }

    // }}}
  void ICLDrawWidget::customPaintEvent(QPainter *poPainter){
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

    initializeCustomPaintEvent(poPainter);
    for(std::vector<DrawCommand*>::iterator it = m_vecCommands.begin();it!= m_vecCommands.end();++it){
      (*it)->exec(poPainter,m_poState);
    }
    finishCustomPaintEvent(poPainter);
    m_oCommandMutex.unlock();
  }

    // }}}

}
