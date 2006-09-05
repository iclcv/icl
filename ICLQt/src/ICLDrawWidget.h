#ifndef ICL_DRAW_WIDGET_H
#define ICL_DRAW_WIDGET_H

#include "ICLWidget.h"
#include <QMutex>

namespace icl{
  /// drawSym class
  /**
  draws absolut or relative into the current image rect
  */
  class ICLDrawWidget : public ICLWidget {
    public:
    
    enum Sym {symRect,symCross,symPlus,symTriangle,symCircle};
    
    ICLDrawWidget(QWidget *poParent);
    virtual ~ICLDrawWidget(){}

    void lock(){m_oCommandMutex.lock();}
    void unlock(){m_oCommandMutex.unlock();}
    void abs();
    void rel();
    

    void point(float x, float y); 
    void line(float x1, float y1, float x2, float y2);
    void rect(float x, float y, float w, float h);
    void ellipse(float x, float y, float w, float h);
    void sym(float x, float y, Sym s);
    void symsize(float w, float h=-1); // if h==-1, h = w;

    void edge(int r, int g, int b, int alpha = 255);
    void fill(int r, int g, int b, int alpha = 255);
    void noedge();
    void nofill();
    void clear(int r=0, int g=0, int b=0, int alpha = 255);
    void reset();
    
    void setPseudoImage(Size s);
    

    virtual void customPaintEvent(QPainter *poPainter);
    virtual void initializeCustomPaintEvent(QPainter *poPainter);
    virtual void finishCustomPaintEvent(QPainter *poPainter);

    
    class State;
    class DrawCommand;

    protected:    
    std::vector<DrawCommand*> m_vecCommands;
    State *m_poState;
    QMutex m_oCommandMutex;
  };
}

#endif
