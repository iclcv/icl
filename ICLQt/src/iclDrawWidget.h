#ifndef ICL_DRAW_WIDGET_H
#define ICL_DRAW_WIDGET_H

#include <iclWidget.h>
#include <QMutex>

namespace icl{
  /** \cond */
  class GLPaintEngine;
  class ImgBase; 
  /** \endcond */

/// Extended Image visualization widget, with a drawing state machine interface \ingroup COMMON
/** The ICLDrawWidget can be used to draw annotation on images in real time.
    It provides the ability for translating draw command given in image coordinations
    with respect to the currently used image scaling type (hold-ar, no-scaling or 
    fit to widget) and to the currently used widget size.

    <h2>Drawing-State machine</h2>
    Like other drawing state machines, like the QPainter or OpenGL, the ICLDrawWidget
    can be used for drawing 2D-primitives step by step into the frame-buffer using 
    OpenGL hardware acceleration. Each implementation of drawing function
    should contain the following steps.
    <pre>

    drawWidget->setImage(..);  /// sets up a new background image 
    
    drawWidget.lock();   /// locks the draw widget against the drawing loop
    drawWidget.reset();  /// deletes all further draw commands
    ... draw commands ...

    drawWidget->unlock();   /// enable the widget to be drawed
 
    </pre>

    <h2>DrawEngine Interface</h2>
    By the introduction of the PaintEngine interface we got an additional abstraction
    layer that generalizes drawing commands like lines, rects, circles and images
    without regarding the underlying drawing mechanism like X11, DirectDraw or OpenGL.
    Yet, Only the GLPaintEngine was hold, because:
    - X11 is not plattform independent
    - Qt is not fast enough for drawing images
*/
 class ICLDrawWidget : public ICLWidget {
    public:
    /// enum used for specification of predefined symbols
    enum Sym {symRect,symCross,symPlus,symTriangle,symCircle};
    
    /// creates a new ICLDrawWidget embedded into the parent component
    ICLDrawWidget(QWidget *parent=0);

    /// destructor2
    ~ICLDrawWidget();

    /// locks the state machine
    /** The state machine collects all draw commands in a command queue internally.
        The access to this command queue must be locked, as push operations on this
        queue are implemented not thread safe. The command queue is also used by the
        internal drawing mechanism, which translates the stored commands into appropriate
        OpenGL commands. Do not forget to call unlock after performing all drawing
        commands.
    */
    void lock(){m_oCommandMutex.lock();}

    /// unlocks the draw command queue ( so it can be drawn by OpenGL )
    void unlock(){m_oCommandMutex.unlock();}

    /// sets up the state machine to treat coordinates in the image pixel coordinate system
    /** the visualization space is x={0..w-1} and y={0..h-1}*/
    void abs();
    
    /// sets up the state machine to receive relative coordinates in range [0,1]
    void rel();
    
    /// draws an image into the given rectangle
    /** The image is copied by the state machine to ensure, that the data is persistent
        when the draw command is passed to the underlying PaintEngine. Otherwise, it would
        not be possible to ensure, that the set image data that is drawn is not changed
        elsewhere 
    */
    void image(ImgBase *image, float x, float y, float w, float h);
    
    /// draws a string into the given rect
    void text(std::string text, float x, float y, float w=-1, float h=-1, int fontsize=15);

    /// draws a point at the given location
    void point(float x, float y); 

    /// draws a set of points
    /** for relative Point coordinates the factors can be set
        point i is drawn at pts[i].x/xfac and pts[i].y/yfac
    **/
    void points(const std::vector<Point> &pts, int xfac=1, int yfac=1);
    
    /// draws a line from point (x1,y1) to point (x2,y2)
    void line(float x1, float y1, float x2, float y2);

    /// draws a rect with given parameters
    void rect(float x, float y, float w, float h);

    /// draws a rect from a icl Rect structure
    void rect(Rect r);

    /// draws a triangle defined by 3 points
    void triangle(float x1, float y1, float x2, float y2, float x3, float y3); 
    
    /// draws a quad with given 4 points 
    void quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4); 

    /// draws an ellipse with given parameters (w==H --> circle)
    void ellipse(float x, float y, float w, float h);

    /// draws a predefined symbol at the given location
    /** The symbols size can be set using symsize */
    void sym(float x, float y, Sym s);
    
    /// sets the size for following "sym" draw commands
    void symsize(float w, float h=-1); // if h==-1, h = w;

    /// sets the draw state machines "edge"-color buffer to a given value
    /** Primitives except images are drawn with the currently set "color"
        and filled with the currently set "fill" 
        alpha values of 0 disables the edge drawing at all
    */    
    void color(int r, int g, int b, int alpha = 255);
    
    /// set the draw state machines "fill"-color buffer to a given value
    /** Primitives except images are drawn with the currently set "color"
        and filled with the currently set "fill" 
        alpha values of 0 disables the edge drawing at all
    */    
    void fill(int r, int g, int b, int alpha = 255);

    /// disables drawing edges
    void nocolor();
    
    /// disables filling primitives
    void nofill();
    
    /// fills the whole image area with the given color
    void clear(int r=0, int g=0, int b=0, int alpha = 255);

    /// clears the drawing command queue 
    /** When drawing in a real-time systems working thread, do not forget to
        call reset before drawing a new frame 
    */
    void reset();
    
    /// if no real image is available
    /** This function will use a black image of size s to be
        drawed in the background */
    void setPseudoImage(Size s);
    
    
    /// this function can be reimplemented in derived classes to perform some custom drawing operations
    virtual void customPaintEvent(PaintEngine *e);

    /// this function can be reimplemented perform some custom initialization before the actual draw call
    virtual void initializeCustomPaintEvent(PaintEngine *e);

    /// this function can be reimplemented perform some custom initialization after the actual draw call
    virtual void finishCustomPaintEvent(PaintEngine *e);

    
    /// forward declaration of an internally used state class
    class State;

    /// forward declaration of the internally used DrawCommandClass
    class DrawCommand;

    protected:    
    /// draw command event queue
    std::vector<DrawCommand*> m_vecCommands;

    /// Data of the "State Machine"
    State *m_poState;

    /// Mutex for a thread save event queue
    QMutex m_oCommandMutex;
  };
}

#endif
