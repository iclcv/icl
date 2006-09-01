#ifndef OSDWIDGET_H
#define OSDWIDGET_H

#include <QRect>
#include <QPainter>
#include <QColor>


#include <vector>
#include <map>

namespace icl{
  class OSDWidget;
  class ICLWidget;
  typedef std::vector<OSDWidget*> wvec;
  typedef ICLWidget ImageWidget;
  
  class OSDWidget{
    public:
    OSDWidget(int id, QRect r,ImageWidget* poIW , OSDWidget *poParent=0);
    virtual ~OSDWidget();

    virtual void drawSelf(QPainter *poPainter,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]);
    virtual void keyPressed(int key, int  x, int y);
    virtual void mouseMoved(int x, int y, int downmask[3]);
    virtual void mousePressed(int x, int y, int button);
    virtual void mouseReleased(int x, int y, int button);
    
    void _keyPressed(int key, int x, int y);
    void _mouseMoved(int x, int y, int downmask[3]);
    void _mousePressed(int x, int y, int button);
    void _mouseReleased(int x, int y, int button);
    void _drawSelf(QPainter *poPainter, int x, int y, int downmask[3]);
   
    virtual void childChanged(int id, void *val=0);
    
    int getID();
    OSDWidget *getParent();
    const wvec& getChilds();
    const QRect& getRect();
    int getChildCount();
    int hasChilds();

    void addChild(OSDWidget *c);
    void removeChild(int id);

    void setRect(QRect oRect);
    int contains(int x, int y);
    int x();
    int y();
    int w();
    int h();

    int mouseOver(int x, int y);
    int mouseOverChild(int x, int y);
    
    void drawBG(QPainter *poPainter,int drawFill,int drawBorder, int over,int  pressed);
    static void drawRect(QPainter *poPainter, QRect r,int drawFill,int  drawBorder, int over, int pressed);
    static void drawCircle(QPainter *poPainter, QRect r,int drawFill, int drawBorder, int over, int pressed);
    static void drawText(QPainter *poPainter, QRect r,QString sText, int over, int pressed, int highlighted=0);
    static void setCol(QPainter *poPainter, int fill, int border, int over, int pressed);
    static int s_iAlpha,s_iBorderR,s_iBorderG, s_iBorderB,s_iFillR,s_iFillG,s_iFillB,s_iHoveredAdd,s_iPressedAdd;
    
    protected:
    wvec m_vecChilds;
    OSDWidget *m_poParent;
    QRect m_oRect;
    int m_iID;
    ImageWidget* m_poIW;
  };
}// namespace icl
#endif
