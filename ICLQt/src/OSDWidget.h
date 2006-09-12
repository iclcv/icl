#ifndef OSDWIDGET_H
#define OSDWIDGET_H

#include <QRect>
#include <QPainter>
#include <QColor>

#include <vector>
#include <map>

#include "GLPaintEngine.h"

namespace icl{
  class OSDWidget;
  class ICLWidget;
  typedef std::vector<OSDWidget*> wvec;
  typedef ICLWidget ImageWidget;
  
  class OSDWidget{
    public:
    OSDWidget(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent=0);
    virtual ~OSDWidget();

    virtual void drawSelf(GLPaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]);
    virtual void keyPressed(int key, int  x, int y);
    virtual void mouseMoved(int x, int y, int downmask[3]);
    virtual void mousePressed(int x, int y, int button);
    virtual void mouseReleased(int x, int y, int button);
    
    void _keyPressed(int key, int x, int y);
    void _mouseMoved(int x, int y, int downmask[3]);
    void _mousePressed(int x, int y, int button);
    void _mouseReleased(int x, int y, int button);
    void _drawSelf(GLPaintEngine *e, int x, int y, int downmask[3]);
   
    virtual void childChanged(int id, void *val=0);
    
    int getID();
    OSDWidget *getParent();
    const wvec& getChilds();
    const Rect& getRect();
    int getChildCount();
    int hasChilds();

    void addChild(OSDWidget *c);
    void removeChild(int id);

    void setRect(Rect oRect);
    int contains(int x, int y);
    int x();
    int y();
    int w();
    int h();

    int mouseOver(int x, int y);
    int mouseOverChild(int x, int y);
    
    void drawBG(GLPaintEngine *e,int drawFill,int drawBorder, int over,int  pressed);
    static void drawRect(GLPaintEngine *e, Rect r,int drawFill,int  drawBorder, int over, int pressed);
    static void drawCircle(GLPaintEngine *e, Rect r,int drawFill, int drawBorder, int over, int pressed);
    static void drawText(GLPaintEngine *e, Rect r,string sText, int over, int pressed, int highlighted=0);
    static void setCol(GLPaintEngine *e, int fill, int border, int over, int pressed);
    static int s_iAlpha,s_iBorderR,s_iBorderG, s_iBorderB,s_iFillR,s_iFillG,s_iFillB,s_iHoveredAdd,s_iPressedAdd;
    
    protected:
    wvec m_vecChilds;
    OSDWidget *m_poParent;
    Rect m_oRect;
    int m_iID;
    ImageWidget* m_poIW;
  };
}// namespace icl
#endif
