#ifndef ICL_DRAW_WIDGET_H
#define ICL_DRAW_WIDGET_H

#include <ICLQt/Widget.h>
#include <QMutex>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Rect32f.h>

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
    \code

    drawWidget->setImage(..);  /// sets up a new background image 
    
    drawWidget.lock();   /// locks the draw widget against the drawing loop
    drawWidget.reset();  /// deletes all further draw commands
    ... draw commands ...

    drawWidget->unlock();   /// enable the widget to be drawed
 
    \endcode

    \section DRAWING_EXAMPLE Sample Application for Image Segmentation
    
    <TABLE border=0><TR><TD>
    \code
\#include <ICLQuick/Common.h>
\#include <ICLQuick/QuickRegions.h>

GUI gui;
std::vector<double> c(3,255); // ref color

void click(const MouseEvent &e){
  if(e.isLeft() && !gui["vis"].as<int>()){
    c = e.getColor();
  }
}

void init(){
  gui << "draw[@handle=draw@minsize=32x24]"
      << ( GUI("hbox[@maxsize=100x3]")
           << "combo(image,colormap,levelmap)[@out=_@handle=vis]"
           << "slider(2,10,5)[@out=levels@label=levels]" );
  gui.show();
  gui["draw"].install(new MouseHandler(click));
}

void run(){
  static GenericGrabber g(FROM_PROGARG("-input"));
  g.setDesiredSize(Size::VGA);
  
  // extract "draw" component as DrawHandle from "gui" the
  // DrawHandle provides direct access to the underlying 
  // ICLDrawWidget by the 'operator->' i.e., it behaves like
  // an ICLDrawWidget-pointer 
  gui_DrawHandle(draw);
  
  // do some image processing (pretty slow here)
  ImgQ im = cvt(g.grab());
  
  // create a color distance map to current ref-color
  ImgQ cm = colormap(im,c[0],c[1],c[2]);
  
  // re-quantize colormap to reduce levels
  ImgQ lm = levels(cm,gui["levels"].as<int>());
  

  // detect connected components
  vector<vector<Point> > pxs = pixels(lm,0,1<<20,255);
  vector<vector<Point> > bds = boundaries(lm,0,1<<20,255);

  // visualize selected image 
  ImgQ *ims[3] = { &im, &cm, &lm};
  draw = ims[gui["vis"].as<int>()];
  
  
  // begin drawing by locking draw-command queue
  draw->lock();
  
  // removing all former draw commands from the queue 
  // important: don't forget this step!
  draw->reset();

  // use drawing state-machine to post draw commands
  draw->pointsize(2);  
  
  draw->color(255,0,0,60);
  for(unsigned int i=0;i<pxs.size();++i){
    draw->points(pxs[i]);
  }

  draw->color(0,100,255,150);
  for(unsigned int i=0;i<bds.size();++i){
    draw->linestrip(bds[i]);
  }
  
  // unlock draw-command queue to grant access to
  // asynchroneous Qt-Event loop
  draw->unlock();

  // post redraw event (note: dont use draw->update(), because
  // 'draw->' refers to the underlying ICLDrawWidget, while
  // 'draw.' refers to the DrawHandle i.e., draw.update() is
  // equivalent to draw->updateFromOtherThread()
  draw.update();
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input(2)",init,run).exec();
}

    \endcode

</TD><TD valign="top">
    \image html drawing-example.png "Screenshot of sample application"
</TD></TR></TABLE>
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

    /// draws a set of points
    void points(const std::vector<Point32f> &pts);
    
    /// draws a set of connected points
    /** for relative Point coordinates the factors can be set
        point i is drawn at pts[i].x/xfac and pts[i].y/yfac
    **/
    void linestrip(const std::vector<Point> &pts, bool closeLoop=true, int xfac=1, int yfac=1);

    /// draws a set of connected points
    void linestrip(const std::vector<Point32f> &pts, bool closeLoop=true);
    
    
    /// draws a line from point (x1,y1) to point (x2,y2)
    void line(float x1, float y1, float x2, float y2);
    
    /// convenience function for drawing lines between two points
    void line(const Point32f &a, const Point32f &b);

    /// draws an arrow from a to b (arrow cap is at b)
    void arrow(float ax, float ay, float bx, float by, float capsize=10);
    
    /// draws an arrow from a to b (arrow cap is at b)
    void arrow(const Point32f &a, const Point32f &b, float capsize=10);
    
    /// draws a rect with given parameters
    void rect(float x, float y, float w, float h);

    /// convenience function for drawing float rects
    void rect(const Rect32f &r);

    /// draws a rect from a icl Rect structure
    void rect(const Rect &r);

    /// draws a triangle defined by 3 points
    void triangle(float x1, float y1, float x2, float y2, float x3, float y3); 
    
    /// draws a triangle defined by 3 points
    void triangle(const Point32f &a, const Point32f &b, const Point32f &c);
    
    /// draws a quad with given 4 points 
    void quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4); 

    /// draws a quad with given 4 points 
    void quad(const Point32f &a, const Point32f &b, const Point32f &c, const Point32f &d);

    /// draws an ellipse with given parameters (w==H --> circle)
    void ellipse(float x, float y, float w, float h);
    
    /// draws an ellipse into given rectangle
    void ellipse(const Rect &r);
    
    /// draws an ellipse into given rectangle
    void ellipse(const Rect32f &r);
    
    /// draws a circle with given center and radius
    void circle(float cx, float cy, float r);
    
    /// draws a circle with given center and radius
    void circle(const Point32f &center, float radius);
    
    /// draws a convex polygon
    void polygon(const std::vector<Point32f> &ps);

    /// draws a regular grid between given points
    void grid(const Point32f *points, int nx, int ny, bool rowMajor=true);

    /// draws a predefined symbol at the given location
    /** The symbols size can be set using symsize */
    void sym(float x, float y, Sym s);
    
    /// sets the size for following "sym" draw commands
    void symsize(float w, float h=-1); // if h==-1, h = w;

    /// sets current linewidth (default is 1);
    void linewidth(float w);

    /// sets current pointsize (default is 1)
    void pointsize(float s);
    
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
