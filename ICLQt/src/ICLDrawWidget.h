#ifndef ICL_DRAW_WIDGET_H
#define ICL_DRAW_WIDGET_H

#include "ICLWidget.h"
#include <QMutex>

namespace icl{

  class GLPaintEngine;
  class ImgI; 
  
/// Exteded Image visualization widget, with a drawing state machine interface
/** The ICLDrawWidget can be used to draw annotation on images in realtime.
    It provides the abilty for translating draw command given in image coordinations
    with respect to the currently used image scaling type (hold-ar, no-scaling or 
    fit to widget) and to the currently used widget size.

    <h2>Drawing-State machine</h2>
    Like other drawing state machines, like the QPainter or OpenGL, the ICLDrawWidget
    can be used for drawing 2D-primitives step by step into the frambuffer using 
    OpenGL hardware acceleration. Each implementation of drawing function
    should contain the following steps.
    <pre>

    drawWidget->setImage(..);  /// sets up a new background image 
    
    drawWidget.lock();   /// locks the draw widget agains the drawing loop
    drawWidget.reset();  /// deletes all further draw commands
    drawWidget.clear(255,255,255);  /// fills the bachround with color white
    ... draw commands ...

    drawWidget->unlock();   /// enable the widget to be drawed
 
    </pre>
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
    
    void image(ImgI *image, float x, float y, float w, float h);
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
    
    /// if no real image is available
    /** This function will use a black image of size s to be
        drawed in the background */
    void setPseudoImage(Size s);
    
    
    virtual void customPaintEvent(GLPaintEngine *e);
    virtual void initializeCustomPaintEvent(GLPaintEngine *e);
    virtual void finishCustomPaintEvent(GLPaintEngine *e);

    
    class State;
    class DrawCommand;

    protected:    
    std::vector<DrawCommand*> m_vecCommands;
    State *m_poState;
    QMutex m_oCommandMutex;
  };
}

#endif
