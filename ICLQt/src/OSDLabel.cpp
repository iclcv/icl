#include "OSDLabel.h"

namespace icl{

  OSDLabel::OSDLabel(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent, string sText):
    OSDWidget(id,r,poIW,poParent),m_sText(sText),m_iContainsMultiText(0){
  }
  
  void OSDLabel::drawSelf(GLPaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]){
    (void)x; (void)y;(void)mouseOver;(void)mouseOverChild; (void)downmask;
    drawBG(e,1,1,0,0);
    if(m_iContainsMultiText){
      int n = (int)m_vecMultiText.size();
      int _h = this->h()/n;
      for(int i=0;i<n;i++){
        drawText(e,Rect(this->x(),this->y()+i*_h,this->w(),_h-2),m_vecMultiText[i],0,0);
      }
    }else{
      drawText(e,m_oRect,m_sText,0,0);
    }
  }

  void OSDLabel::setMultiText(std::vector<string> smultitext){
    m_iContainsMultiText=1;
    m_vecMultiText = smultitext;
  }
  
}
