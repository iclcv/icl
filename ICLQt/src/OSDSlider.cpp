#include <OSDSlider.h>
#include <math.h>

#ifdef WIN32
#	include <Mathematics.h>
#endif

using namespace std;

namespace icl{
  const float OSDSlider::s_fTextFrac=0.2;
  const float OSDSlider::s_13=1.0/3;
  const int OSDSlider::s_iGripW=10;
   
  OSDSlider::OSDSlider(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent,int min, int max, int curr):
    OSDWidget(id,r,poIW,poParent),m_iMin(min), m_iMax(max), m_iCurr(curr),m_iSliderHovered(0){
    int rel_x = (int)(s_fTextFrac*w());
    int rel_y = (int)(s_13*h());
    int _w = w()-rel_x;
    int _h = rel_y;
    m_oBar = Rect(rel_x+x(),rel_y+y()+1,_w,_h);
    m_oText = Rect(x(),y()+1,w()-_w,h()-2);
    m_iGripH = _h+4;
    m_iGripY = y()+(h()-m_iGripH)/2;
    m_oGrip = Rect(valToPos(curr)-s_iGripW/2,m_iGripY,s_iGripW,m_iGripH);
    m_oProgress = Rect(m_oBar.x,m_oBar.y,m_oGrip.x-m_oBar.x+m_oGrip.width/2,m_oBar.height-1);
  }
  
  void OSDSlider::drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]){
    (void)x; (void)y;(void)mouseOver;(void)mouseOverChild; (void)downmask;
    
    drawRect(e,m_oBar,1,1,mouseOver && !mouseOverChild,0);
    drawRect(e,m_oText,1,1,mouseOver && !mouseOverChild,0);
    drawText(e,m_oText,QString::number(m_iCurr).toLatin1().data(),mouseOver && !mouseOverChild, downmask[0]|| downmask[1]|| downmask[2]);
    
    drawCircle(e, m_oGrip,1,1,m_oGrip.contains(x,y),m_iSliderHovered);
    
    e->color(0,0,0,0);
    e->fill(255,255,255,100);
    if(m_oProgress.width > 2){
      e->rect(m_oProgress);
    }
    
    e->fill(255,255,255,200);
    for(int d=1;d<2;d++){
      e->ellipse(Rect(m_oGrip.x+d,m_oGrip.y+d,m_oGrip.width-2*d,m_oGrip.height-2*d));
    }
  }
    
  void OSDSlider::mouseMoved(int _x, int _y, int downmask[3]){
    (void)_y;(void)downmask;
    if(m_iSliderHovered && _x >= m_oBar.x){
      m_iCurr = posToVal(_x);
      m_oGrip = Rect(valToPos(m_iCurr)-s_iGripW/2,m_iGripY,s_iGripW,m_iGripH);
      m_oProgress = Rect(m_oBar.x,m_oBar.y,m_oGrip.x-m_oBar.x+m_oGrip.width/2,m_oBar.height-1);
      childChanged(m_iID,&m_iCurr);
    }      
  }

  void OSDSlider::mousePressed(int _x, int _y, int button){
    (void)button;
    if(m_oGrip.contains(_x,_y)){
      m_iSliderHovered = 1;
    }else if(m_oBar.contains(_x,_y)){
      m_iCurr = posToVal(_x);
      m_oGrip = Rect(_x-s_iGripW/2,y()+h()/2-m_iGripH/2,s_iGripW,m_iGripH);
      m_oProgress = Rect(m_oBar.x,m_oBar.y,m_oGrip.x-m_oBar.x+m_oGrip.width/2,m_oBar.height-1);
      childChanged(m_iID,&m_iCurr);
    }
  }
  
  void OSDSlider::mouseReleased(int x, int y, int button){
    (void)x;(void)y;(void)button;
    m_iSliderHovered = 0;
  }
  
  int OSDSlider::mxb(int x, int x_min, int x_max, int y_min, int y_max){
    int dx = x_max-x_min;
    int dy = y_max-y_min;
    float m = (float)dy/(float)dx;
    int b = (int) rint(y_min-m*x_min);
    return (int) (rint(m*x)+b);
  }
  
  int OSDSlider::valToPos(int val){
    return mxb(val,m_iMin,m_iMax,m_oBar.x,m_oBar.x+m_oBar.width);
  }
  int OSDSlider::posToVal(int pos){
    return mxb(pos,m_oBar.x,m_oBar.x+m_oBar.width,m_iMin,m_iMax);
  }
}
