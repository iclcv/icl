#include <iclOSDCaptureVideoButton.h>
#include <cmath>
#include <iclTime.h>
using std::string;
namespace icl{
  namespace{
    Rect get_symbol_rect(const Rect &r){
      int MARGIN = r.height > 80 ? (r.height-80)/2 : 4;
      int h = r.height-2*MARGIN;
      int w = h;
      int x = r.right()-1-w-MARGIN;
      int y = r.y+MARGIN;
      return Rect(x,y,w,h);
    }
  }

  OSDCaptureVideoButton::OSDCaptureVideoButton(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent):
    OSDButton(id,r,poIW,poParent,"",true){
  }
  
  
  void OSDCaptureVideoButton::drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]){
    //    std::string filename = m_poIW->getCaptureVideoFileString();
    (void)x; (void)y;
    drawBG(e,1,1,mouseOver && !mouseOverChild, downmask[0]|| downmask[1]|| downmask[2]);

    // save current color
    int color[4],fill[4];
    e->getColor(color);
    e->getFill(fill);
    Rect r = get_symbol_rect(m_oRect);
    drawBG(e,1,1,mouseOver && !mouseOverChild, downmask[0]|| downmask[1]|| downmask[2]);
    if(m_iIsToggled){ // recording ...
      float alpha = 255 - 100* (1+::sin(Time::now().toMilliSecondsDouble()/250));;
      e->color(255,0,0,alpha);
      e->fill(0,0,0,0);
      e->ellipse(r);
      for(int i=0;i<4;++i){
        e->ellipse(r.enlarged(-2*i));
      }
    }else{ // pause ...
      e->color(255,255,255,255);
      e->rect(Rect(r.x,r.y,r.width/3,r.height));
      e->rect(Rect(r.x+0.66666*r.width,r.y,r.width/3,r.height));
    }
    
    e->color(255,255,255,200);
    e->rect(r.enlarged(2));
    // restore color
    e->color(color[0],color[1],color[2],color[3]);
    e->fill(fill[0],fill[1],fill[2],fill[3]);
    
    drawText(e,m_oRect,m_sText,mouseOver && !mouseOverChild, downmask[0]|| downmask[1]|| downmask[2],m_iIsToggled);
  }

}
