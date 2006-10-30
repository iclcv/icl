#include "OSDButton.h"

using std::string;
namespace icl{

  OSDButton::OSDButton(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent, string sText, int iToggable):
    OSDWidget(id,r,poIW,poParent),m_sText(sText),m_iToggable(iToggable),m_iIsToggled(0){ 
  }
  
  
  void OSDButton::drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]){
    (void)x; (void)y;
    drawBG(e,1,1,mouseOver && !mouseOverChild, downmask[0]|| downmask[1]|| downmask[2]);
    if(m_iIsToggled){
      drawBG(e,1,1,mouseOver && !mouseOverChild, downmask[0]|| downmask[1]|| downmask[2]);
    }
    drawText(e,m_oRect,m_sText,mouseOver && !mouseOverChild, downmask[0]|| downmask[1]|| downmask[2],m_iIsToggled);
  }
  
  void OSDButton::mousePressed(int _x, int _y, int button){
    (void)_x; (void)_y; (void)button;
    childChanged(m_iID);
    if(m_iToggable){
      m_iIsToggled = !m_iIsToggled;
    }
  }
  
  void OSDButton::setText(string sText){
    m_sText = sText;
  }

  void OSDButton::setToggled(int iToggled){
    m_iIsToggled = iToggled;    
  }
}
