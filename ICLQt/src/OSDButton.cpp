#include "OSDButton.h"

namespace icl{

  OSDButton::OSDButton(int id, QRect r,ImageWidget* poIW , OSDWidget *poParent, QString sText, int iToggable):
    OSDWidget(id,r,poIW,poParent),m_sText(sText),m_iToggable(iToggable),m_iIsToggled(0){ 
  }
  
  
  void OSDButton::drawSelf(QPainter *poPainter,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]){
    (void)x; (void)y;
    drawBG(poPainter,1,1,mouseOver && !mouseOverChild, downmask[0]|| downmask[1]|| downmask[2]);
    if(m_iIsToggled){
      drawBG(poPainter,1,1,mouseOver && !mouseOverChild, downmask[0]|| downmask[1]|| downmask[2]);
    }
    drawText(poPainter,m_oRect,m_sText,mouseOver && !mouseOverChild, downmask[0]|| downmask[1]|| downmask[2],m_iIsToggled);
  }
  
  void OSDButton::mousePressed(int _x, int _y, int button){
    (void)_x; (void)_y; (void)button;
    childChanged(m_iID);
    if(m_iToggable){
      m_iIsToggled = !m_iIsToggled;
    }
  }
  
  void OSDButton::setText(QString sText){
    m_sText = sText;
  }

  void OSDButton::setToggled(int iToggled){
    m_iIsToggled = iToggled;    
  }
}
