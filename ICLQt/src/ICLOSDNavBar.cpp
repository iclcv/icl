#include <ICLOSDNavBar.h>

namespace icl{

  const int OSDNavBar::s_iGap;
  const int OSDNavBar::s_iMargin;
  
  OSDNavBar::OSDNavBar(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent,svec vecNames, svec vecShortNames, ivec vecIDs, int selectedIndex):
    OSDWidget(id,r,poIW,poParent),m_iCurrIndex(-1),m_vecNames(vecNames),m_vecShortNames(vecShortNames){
    m_iSmallW = (r.width-r.width/4)/vecNames.size();
    int n = (int)vecNames.size();
    int h = r.height-2*s_iMargin;
    int curr_x = s_iMargin+x();
    for(int i=0;i<n;i++){
      Rect oCurrRect(curr_x,s_iMargin+y(),m_iSmallW-1,h); 
      curr_x += m_iSmallW+s_iGap;
      addChild(new OSDButton(vecIDs[i],oCurrRect,poIW,this,vecShortNames[i],1));
    }
    
    selectIndex(selectedIndex);
  }

  void OSDNavBar::selectIndex(int index){
    if(index == m_iCurrIndex) return;
    m_iCurrIndex = index;
    int n = (int)m_vecChilds.size();
    int w = m_oRect.width-2*s_iMargin;
    int h = m_oRect.height-2*s_iMargin;
    int curr_x = s_iMargin+x();
    for(int i=0;i<n;i++){
      OSDButton *b = static_cast<OSDButton*>(m_vecChilds[i]);
      if(i==index){
        int iWideWidth = w-(n-1)*m_iSmallW-(n-2)*s_iGap;
        b->setRect(Rect(curr_x,s_iMargin+y(),iWideWidth-1,h));
        curr_x += iWideWidth+s_iGap;
        b->setText(m_vecNames[i]);
        b->setToggled(1);
      }else{
        m_vecChilds[i]->setRect(Rect(curr_x,s_iMargin+y(),m_iSmallW-1,h));
        curr_x += m_iSmallW+s_iGap;
        b->setText(m_vecShortNames[i]);
        b->setToggled(0);
      }
    }      
  }

  void OSDNavBar::mousePressed(int _x, int _y, int button){
    (void)button;
    for(uint i = 0; i<m_vecChilds.size(); i++){
      if(m_vecChilds[i]->contains(_x,_y)){
        selectIndex(i);
        return;
      }
    }
  }


}// namespace icl
