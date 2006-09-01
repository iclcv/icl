#include "OSDWidget.h"
#include "ICLWidget.h"

namespace icl{
 // {{{ static variables

  int OSDWidget::s_iAlpha = 160;
  int OSDWidget::s_iBorderR = 0;
  int OSDWidget::s_iBorderG = 120;
  int OSDWidget::s_iBorderB = 255;
  int OSDWidget::s_iFillR = 40;
  int OSDWidget::s_iFillG = 170;
  int OSDWidget::s_iFillB = 255;
  int OSDWidget::s_iHoveredAdd = 30;
  int OSDWidget::s_iPressedAdd = 50;

  // }}}

 OSDWidget::OSDWidget(int id, QRect r, ImageWidget *poIW,OSDWidget *poParent):
   // {{{ open

    m_poParent(poParent),m_oRect(r), m_iID(id), m_poIW(poIW){
    if(poParent){
      m_oRect = QRect(this->x()+poParent->x(),this->y()+poParent->y(),w(),h());
    }
  }

  // }}}
 
 void OSDWidget::_keyPressed(int key, int x, int y){
   // {{{ open

    for(wvec::iterator it=m_vecChilds.begin();it!= m_vecChilds.end();++it){
      if((*it)->mouseOver(x,y)){
        (*it)->_keyPressed(key,x,y);
      }
    }
    keyPressed(key,x,y);
  }

  // }}}
 void OSDWidget::_mouseMoved(int x, int y, int downmask[3]){
   // {{{ open

    for(wvec::iterator it=m_vecChilds.begin();it!= m_vecChilds.end();++it){
      if((*it)->mouseOver(x,y)){
        (*it)->_mouseMoved(x,y,downmask);
      }
    }
    mouseMoved(x,y,downmask);
  }

  // }}}
 void OSDWidget::_mousePressed(int x, int y, int button){
   // {{{ open

    for(wvec::iterator it=m_vecChilds.begin();it!= m_vecChilds.end();++it){
      if((*it)->mouseOver(x,y)){
        (*it)->_mousePressed(x,y,button);
      }
    }
    mousePressed(x,y,button);
  }

  // }}}
 void OSDWidget::_mouseReleased(int x, int y, int button){
   // {{{ open

    for(wvec::iterator it=m_vecChilds.begin();it!= m_vecChilds.end();++it){
      //if((*it)->mouseOver(x,y)){
      (*it)->_mouseReleased(x,y,button);
      //}
    }
    mouseReleased(x,y,button);
  }

  // }}}
 void OSDWidget::_drawSelf(QPainter *poPainter,int x, int y, int downmask[3]){
   // {{{ open

    drawSelf(poPainter,x,y,mouseOver(x,y),mouseOverChild(x,y),downmask);    
    for(wvec::iterator it=m_vecChilds.begin();it!= m_vecChilds.end();++it){
      (*it)->_drawSelf(poPainter,x,y,downmask);
    }    
  }

  // }}}
 
 void OSDWidget::removeChild(int id){
   // {{{ open

    for(wvec::iterator it=m_vecChilds.begin();it!= m_vecChilds.end();++it){
      if((*it)->getID() == id){
        m_vecChilds.erase(it);
        return;
      }
    }
  }

  // }}}
 
 void OSDWidget::setCol(QPainter *poPainter, int fill,int border,int over, int pressed){
   // {{{ open
#define __CLIP(X) (X)>255?255:(X)
    int iFillAdd = 0;
    int iBorderAdd = 0;
    if(over){
      if(pressed){
        iFillAdd +=s_iPressedAdd;
        iBorderAdd +=s_iPressedAdd;
      }else{
        iFillAdd +=s_iHoveredAdd;
        iBorderAdd +=s_iHoveredAdd;
      }
    }
    if(border){
      poPainter->setPen(QColor(__CLIP(s_iBorderR+iBorderAdd),__CLIP(s_iBorderG+iBorderAdd),__CLIP(s_iBorderB+iBorderAdd),s_iAlpha));
    }else{
      poPainter->setPen(Qt::NoPen);
    }
    if(fill){
      poPainter->setBrush(QColor(__CLIP(s_iFillR+iFillAdd),__CLIP(s_iFillG+iFillAdd),__CLIP(s_iFillB+iFillAdd),s_iAlpha));
    }else{
      poPainter->setBrush(Qt::NoBrush);
    }
#undef __CLIP
  }

  // }}}
 void OSDWidget::drawBG(QPainter *poPainter,int drawFill,int drawBorder, int hovered,int  pressed){
   // {{{ open

    drawRect(poPainter,m_oRect,drawFill, drawBorder, hovered, pressed);
  }

  // }}}
 void OSDWidget::drawRect(QPainter *poPainter, QRect r,int drawFill,int  drawBorder, int hovered, int pressed){
   // {{{ open

    setCol(poPainter,drawFill, drawBorder, hovered, pressed);
    poPainter->drawRect(r);    
    poPainter->setPen(Qt::NoPen);
    int d = 2;
    poPainter->drawRect(r.x()+d,r.y()+2, r.width()-2*d, r.height()-2*d);
  }     

  // }}}
 void OSDWidget::drawCircle(QPainter *poPainter, QRect r,int drawFill, int drawBorder, int hovered, int pressed){
   // {{{ open

    setCol(poPainter,drawFill, drawBorder, hovered, pressed);
    poPainter->drawEllipse(r);
  }

  // }}}
 void OSDWidget::drawText(QPainter *poPainter, QRect r,QString sText, int hovered, int pressed,int highlighted){
   // {{{ open
    (void)hovered; (void)pressed;
    if(highlighted){
      poPainter->setPen(QColor(255,255,255,100));
      
      int x=r.x();
      int y=r.y();
      int w=r.width();
      int h=r.height();
    
      poPainter->drawText(QRect(x-1,y-1,w,h),Qt::AlignCenter,sText);
      poPainter->drawText(QRect(x,y-1,w,h),Qt::AlignCenter,sText);
      poPainter->drawText(QRect(x+1,y-1,w,h),Qt::AlignCenter,sText);
      poPainter->drawText(QRect(x-1,y,w,h),Qt::AlignCenter,sText);
      poPainter->drawText(QRect(x+1,y,w,h),Qt::AlignCenter,sText);
      poPainter->drawText(QRect(x-1,y+1,w,h),Qt::AlignCenter,sText);
      poPainter->drawText(QRect(x,y+1,w,h),Qt::AlignCenter,sText);
      poPainter->drawText(QRect(x+1,y+1,w,h),Qt::AlignCenter,sText);
    }
    poPainter->setPen(QColor(255,255,255,255));
    poPainter->drawText(r,Qt::AlignCenter,sText);
  }

  // }}}
 
 void OSDWidget::drawSelf(QPainter *poPainter,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]){
   // {{{ open

    (void)x; (void)y; (void)downmask;
    if(!hasChilds()){
      drawBG(poPainter,0,1,mouseOver && !mouseOverChild,downmask[0]||downmask[1]||downmask[3]);
    }
  }

  // }}}
 int OSDWidget::mouseOver(int x, int y){
   // {{{ open

    return m_oRect.contains(x,y);
  }

  // }}}
 int OSDWidget::mouseOverChild(int x, int y){
   // {{{ open

    for(wvec::iterator it=m_vecChilds.begin();it!= m_vecChilds.end();++it){
      if((*it)->mouseOver(x,y)) return 1;
    }
    return 0;
  }

  // }}}
 void OSDWidget::childChanged(int id, void *val){
   // {{{ open
    if(m_poParent) m_poParent->childChanged(id,val);
    else m_poIW->childChanged(id, val);
  }

  // }}}
 
 OSDWidget::~OSDWidget(){
   // {{{ open

  }
    // }}}
 
 void OSDWidget::keyPressed(int key, int  x, int y){(void)x;(void)y;(void)key;}
  void OSDWidget::mouseMoved(int x, int y, int downmask[3]){(void)x;(void)y;(void)downmask;}
  void OSDWidget::mousePressed(int x, int y, int button){(void)x;(void)y;(void)button;}
  void OSDWidget::mouseReleased(int x, int y, int button){(void)x;(void)y;(void)button;}
  
  int OSDWidget::getID(){ return m_iID; }
  OSDWidget *OSDWidget::getParent(){ return m_poParent; }
  const wvec& OSDWidget::getChilds(){ return m_vecChilds; }
  const QRect& OSDWidget::getRect(){ return m_oRect; }
  int OSDWidget::getChildCount(){ return (int)m_vecChilds.size(); }
  int OSDWidget::hasChilds(){ return getChildCount()>0; }
  int OSDWidget::contains(int x, int y){ return m_oRect.contains(x,y); }
  int OSDWidget::x(){ return m_oRect.x(); }
  int OSDWidget::y(){ return m_oRect.y(); }
  int OSDWidget::w(){ return m_oRect.width(); }
  int OSDWidget::h(){ return m_oRect.height(); }
   
  void OSDWidget::addChild(OSDWidget *c){ m_vecChilds.push_back(c); }
  void OSDWidget::setRect(QRect oRect){ m_oRect = oRect; }

  
}// namespace icl
