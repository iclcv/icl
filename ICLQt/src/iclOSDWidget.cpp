#include <iclOSDWidget.h>
#include <iclWidget.h>

using std::string;

namespace icl{
 // {{{ static variables

#ifndef DO_NOT_USE_OPENGL_ACCELERATION
  int OSDWidget::s_iAlpha = 160;
#else
  int OSDWidget::s_iAlpha = 255;
#endif
  int OSDWidget::s_iBorderR = 0;
  int OSDWidget::s_iBorderG = 120;
  int OSDWidget::s_iBorderB = 255;
  int OSDWidget::s_iFillR = 40;
  int OSDWidget::s_iFillG = 170;
  int OSDWidget::s_iFillB = 255;
  int OSDWidget::s_iHoveredAdd = 30;
  int OSDWidget::s_iPressedAdd = 50;

  // }}}

 OSDWidget::OSDWidget(int id, Rect r, ImageWidget *poIW,OSDWidget *poParent):
   // {{{ open

    m_poParent(poParent),m_oRect(r), m_iID(id), m_poIW(poIW){
    if(poParent){
      m_oRect = Rect(this->x()+poParent->x(),this->y()+poParent->y(),w(),h());
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
 void OSDWidget::_drawSelf(PaintEngine *e,int x, int y, int downmask[3]){
   // {{{ open

    drawSelf(e,x,y,mouseOver(x,y),mouseOverChild(x,y),downmask);    
    for(wvec::iterator it=m_vecChilds.begin();it!= m_vecChilds.end();++it){
      (*it)->_drawSelf(e,x,y,downmask);
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
 
 void OSDWidget::setCol(PaintEngine *e, int fill,int border,int over, int pressed){
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
      e->color(__CLIP(s_iBorderR+iBorderAdd),__CLIP(s_iBorderG+iBorderAdd),__CLIP(s_iBorderB+iBorderAdd),s_iAlpha);
    }else{
      e->color(0,0,0,0);
    }
    if(fill){
      e->fill(__CLIP(s_iFillR+iFillAdd),__CLIP(s_iFillG+iFillAdd),__CLIP(s_iFillB+iFillAdd),s_iAlpha);
    }else{
      e->fill(0,0,0,0);
    }
#undef __CLIP
  }

  // }}}
 void OSDWidget::drawBG(PaintEngine *e,int drawFill,int drawBorder, int hovered,int  pressed){
   // {{{ open

    drawRect(e,m_oRect,drawFill, drawBorder, hovered, pressed);
  }

  // }}}
 void OSDWidget::drawRect(PaintEngine *e, Rect r,int drawFill,int  drawBorder, int hovered, int pressed){
   // {{{ open

    setCol(e,drawFill, drawBorder, hovered, pressed);
    e->rect(r);    
    e->color(0,0,0,0); /// ????
    int d = 2;
    e->rect(Rect(r.x+d,r.y+d, r.width-2*d, r.height-2*d));
  }     

  // }}}
 void OSDWidget::drawCircle(PaintEngine *e, Rect r,int drawFill, int drawBorder, int hovered, int pressed){
   // {{{ open
    setCol(e,drawFill, drawBorder, hovered, pressed);
    e->ellipse(r);
  }

  // }}}
  void OSDWidget::drawText(PaintEngine *e, Rect r,string sText, int hovered, int pressed,int highlighted, bool centered){
   // {{{ open
    (void)hovered; (void)pressed;
    
    PaintEngine::AlignMode m = centered ? PaintEngine::Centered :  PaintEngine::NoAlign;

    r.x--;
    if(highlighted){
      e->color(255,255,255,100);
      
      int x=r.x;
      int y=r.y;
      int w=r.width;
      int h=r.height;

      
      e->text(Rect(x-1,y-1,w,h),sText,m);
      e->text(Rect(x,y-1,w,h),sText,m);
      e->text(Rect(x+1,y-1,w,h),sText,m);
      e->text(Rect(x-1,y,w,h),sText,m);
      e->text(Rect(x+1,y,w,h),sText,m);
      e->text(Rect(x-1,y+1,w,h),sText,m);
      e->text(Rect(x,y+1,w,h),sText,m);
      e->text(Rect(x+1,y+1,w,h),sText,m);
    }
    e->color(255,255,255);
    e->text(r,sText,m);
  }

  // }}}
 
 void OSDWidget::drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]){
   // {{{ open

    (void)x; (void)y; (void)downmask;
    if(!hasChilds()){
      drawBG(e,0,1,mouseOver && !mouseOverChild,downmask[0]||downmask[1]||downmask[3]);
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
   for(wvec::iterator it=m_vecChilds.begin();it!= m_vecChilds.end();++it){
     delete *it;
   }
 }
    // }}}
 
 void OSDWidget::keyPressed(int key, int  x, int y){(void)x;(void)y;(void)key;}
  void OSDWidget::mouseMoved(int x, int y, int downmask[3]){(void)x;(void)y;(void)downmask;}
  void OSDWidget::mousePressed(int x, int y, int button){(void)x;(void)y;(void)button;}
  void OSDWidget::mouseReleased(int x, int y, int button){(void)x;(void)y;(void)button;}
  
  int OSDWidget::getID(){ return m_iID; }
  OSDWidget *OSDWidget::getParent(){ return m_poParent; }
  const wvec& OSDWidget::getChilds(){ return m_vecChilds; }
  const Rect& OSDWidget::getRect(){ return m_oRect; }
  int OSDWidget::getChildCount(){ return (int)m_vecChilds.size(); }
  int OSDWidget::hasChilds(){ return getChildCount()>0; }
  int OSDWidget::contains(int x, int y){ return QRect(m_oRect.x,m_oRect.y,m_oRect.width,m_oRect.height).contains(x,y); }
  int OSDWidget::x(){ return m_oRect.x; }
  int OSDWidget::y(){ return m_oRect.y; }
  int OSDWidget::w(){ return m_oRect.width; }
  int OSDWidget::h(){ return m_oRect.height; }
   
  void OSDWidget::addChild(OSDWidget *c){ m_vecChilds.push_back(c); }
  void OSDWidget::setRect(Rect oRect){ m_oRect = oRect; }

  
}// namespace icl
