#ifndef OSDWIDGET_H
#define OSDWIDGET_H

#include <QRect>
#include <QPainter>
#include <QColor>

#include <vector>
#include <map>

#include "PaintEngine.h"

namespace icl{
  
  /// forward declarations
  class OSDWidget;
  class ICLWidget;
  
  /// vector of OSDWidget pointers
  typedef std::vector<OSDWidget*> wvec;

  /// typedef for according image widget class
  typedef ICLWidget ImageWidget;
  
  /// base class for On Screen Display (OSD) Widgets
  /** Like QWidgets, the OSDWidget class provides an interface for
      arbitrary OSD-components like sliders, buttons and labels.
      Each widget may contain other wigets, an passes all givent events
      to its childs by default. All event handling functinos like
      drawSelf, keyPressed, ... are virtual, and may be specialized 
      in the specific widget classes, derived from OSDWidget.
  

      <h2>Event-Handling</h2>
      To simlpify event handling, each Widget gets a constant and 
      <b>unique</b> id, which may be used to estimate event sources 
      in parent classes.
      In addition to this, the event handling mechanism is simplified
      to provide the ability of processing all child-events in the parent
      class, or to pass child events to the parent class.
    
      All events, that are captured by Qts event loop are passed to the
      root Widget, which distributes the event to each child widget. All 
      of these widgets may have a custom event handling functionality and/or
      forwarding the received events to its own childs.
      It is strongly recommented not to build <b>very</b> complex GUI structures,
      a one needs to considere, that all events are passed down to all
      widgets that are contained directly and indirectly by the root widget.
  
      In addition to this top down event handlich approach, an bottom up
      mechanism is implemented, which allows the programmer for
      grouped handling of the child widgets events. Each widget may
      call the <em>childChanded(id, value)</em> function to inform the
      parent widget, that somthing has been changed, e.g. a slider will
      send up its new value using this mechanism. (For more detail take
      a look at the implementation of the common OSDWidget-classes like
      OSDButton or OSDSlider)
  
      <h2>Drawing</h2>
      The drawing mechanism is derived from the eventhandling except
      the bottom up functionalities. Like QWidgets, all OSDWidgets
      may implement the drawSelf-method. To achive a related look
      of all gui componets the base class provides abilities for drawing
      primitives like text, boxes or ellipses.    
  */
  class OSDWidget{
    public:

    /// Constructor 
    /**Creates a OSWWidget with the given parameters*/
    OSDWidget(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent=0);
    
    /// Destructor
    virtual ~OSDWidget();

    //@{ @name custom event processing of this widget
    /// drawing functions
    virtual void drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]);

    /// key event handling with given key and given mouse coords
    virtual void keyPressed(int key, int  x, int y);

    /// mouse motion handling
    virtual void mouseMoved(int x, int y, int downmask[3]);
    
    /// mouse pressed handling
    virtual void mousePressed(int x, int y, int button);
    
    /// mouse released handling
    virtual void mouseReleased(int x, int y, int button);
    
    // @}@{ @name top down event handling functions
    void _keyPressed(int key, int x, int y);
    void _mouseMoved(int x, int y, int downmask[3]);
    void _mousePressed(int x, int y, int button);
    void _mouseReleased(int x, int y, int button);
    void _drawSelf(PaintEngine *e, int x, int y, int downmask[3]);
    
    // @}@{ @name bottom up event handling
    virtual void childChanged(int id, void *val=0);
    
    // @}@{ @name utility functions
    
    /// returns the widgets unique id
    int getID();
    
    /// returns the widgets parent widgets
    OSDWidget *getParent();
    
    /// returns a reference of the child vector
    const wvec& getChilds();
    
    /// returns the wigets rect
    /** All widgets have their own "Rect", that specifies
        the widgest absolut window coordinates and size     
    */
    const Rect& getRect();

    /// returns the child count of this widget
    int getChildCount();
    
    /// returns if the child count is > 0
    int hasChilds();

    /// adds a new child to the widgets
    void addChild(OSDWidget *c);
    
    /// removes the child with the given id
    void removeChild(int id);
    
    /// sets up the widget geometry
    void setRect(Rect oRect);
    
    /// retuns if the widget contains a given point
    int contains(int x, int y);
    
    /// x-offset of the widget
    int x();
    
    /// y-offset of the widget
    int y();
    
    /// width of the widget
    int w();
    
    /// height of the widget
    int h();

    /// alias for contains(x,y)
    int mouseOver(int x,int y);
    
    /// returns if a child of the widgets was hit
    int mouseOverChild(int x, int y);
    // @}@{ @name utility function for drawing primitives 
    
    /// draws the background of the widgets with givent parameters
    void drawBG(PaintEngine *e,int drawFill,int drawBorder, int over,int  pressed);
    
    /// draws a rect with given parameters
    static void drawRect(PaintEngine *e, Rect r,int drawFill,int  drawBorder, int over, int pressed);

    /// draws a ellipse with given parameters
    static void drawCircle(PaintEngine *e, Rect r,int drawFill, int drawBorder, int over, int pressed);

    /// draws a string with given parameters
    static void drawText(PaintEngine *e, Rect r,std::string sText, int over, int pressed, int highlighted=0);

    /// sets up the currently used color ot the widget 
    static void setCol(PaintEngine *e, int fill, int border, int over, int pressed);
    
    /// static parameters that specify appearance or the widgest
    static int s_iAlpha,s_iBorderR,s_iBorderG, s_iBorderB,s_iFillR,s_iFillG,s_iFillB,s_iHoveredAdd,s_iPressedAdd;
    // @}
   
    protected:
    wvec m_vecChilds;
    OSDWidget *m_poParent;
    Rect m_oRect;
    int m_iID;
    ImageWidget* m_poIW;
  };
}// namespace icl
#endif
