#include <OSD.h>
#include <ICLWidget.h>

using std::string;
namespace icl{
 
  // {{{ static constants

  const int OSD::NAV_ID;
  const int OSD::NAV_MAIN_ID;
  const int OSD::NAV_ADJUST_ID;
  const int OSD::NAV_FITMODE_ID;
  const int OSD::NAV_CHANNELS_ID;
  const int OSD::NAV_CAPTURE_ID;
  const int OSD::NAV_INFO_ID;
  const int OSD::NAV_MENU_ID;
  
  const int OSD::MAIN_ID;
  const int OSD::MAIN_ADJUST_ID;
  const int OSD::MAIN_FITMODE_ID;
  const int OSD::MAIN_CHANNELS_ID;
  const int OSD::MAIN_CAPTURE_ID;
  const int OSD::MAIN_INFO_ID;
  const int OSD::MAIN_MENU_ID;
  
  const int OSD::ADJUST_ID;
  const int OSD::ADJUST_MODE_NONE_ID;
  const int OSD::ADJUST_MODE_MANUAL_ID;
  const int OSD::ADJUST_MODE_AUTO_ID;
  const int OSD::ADJUST_BRIGHTNESS_LABEL_ID;
  const int OSD::ADJUST_BRIGHTNESS_SLIDER_ID;
  const int OSD::ADJUST_CONTRAST_LABEL_ID;
  const int OSD::ADJUST_CONTRAST_SLIDER_ID;
  const int OSD::ADJUST_INTENSITY_LABEL_ID;
  const int OSD::ADJUST_INTENSITY_SLIDER_ID;

  const int OSD::FITMODE_ID;
  const int OSD::FITMODE_NOSCALE_ID;
  const int OSD::FITMODE_HOLDAR_ID;
  const int OSD::FITMODE_FIT_ID;

  
  const int OSD::CHANNELS_ID;
  const int OSD::CAPTURE_ID;
  const int OSD::CAPTURE_LABEL_ID;
  const int OSD::CAPTURE_BUTTON_ID;
  const int OSD::INFO_ID;

  const int OSD::MENU_ID;
  const int OSD::MENU_FILL_RGB_LABEL_ID;
  const int OSD::MENU_FILL_R_SLIDER_ID;
  const int OSD::MENU_FILL_G_SLIDER_ID;
  const int OSD::MENU_FILL_B_SLIDER_ID;
  const int OSD::MENU_BORDER_RGB_LABEL_ID;
  const int OSD::MENU_BORDER_R_SLIDER_ID;
  const int OSD::MENU_BORDER_G_SLIDER_ID;
  const int OSD::MENU_BORDER_B_SLIDER_ID;
  const int OSD::MENU_ALPHA_LABEL_ID;
  const int OSD::MENU_ALPHA_SLIDER_ID;


  const int OSD::BOTTOM_MAIN_ID;
  const int OSD::BOTTOM_EXIT_ID;

  // }}}

  OSD::OSD(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent):
    // {{{ open

    OSDWidget(id,r,poIW,poParent),m_iCurrID(-1){
    
    // NAV-Bar-------------------------------------------------------------
    std::vector<string> vec, vecShort;
    std::vector<int> vecIDs;
    vec.push_back("main");vecShort.push_back("MA");vecIDs.push_back(NAV_MAIN_ID);
    vec.push_back("adjust");vecShort.push_back("A");vecIDs.push_back(NAV_ADJUST_ID);
    vec.push_back("fitmode");vecShort.push_back("F");vecIDs.push_back(NAV_FITMODE_ID);
    vec.push_back("channels");vecShort.push_back("CH");vecIDs.push_back(NAV_CHANNELS_ID);
    vec.push_back("capture");vecShort.push_back("CA");vecIDs.push_back(NAV_CAPTURE_ID);
    vec.push_back("info");vecShort.push_back("I");vecIDs.push_back(NAV_INFO_ID);
    vec.push_back("menu");vecShort.push_back("ME");vecIDs.push_back(NAV_MENU_ID);
    
    m_poNavBar = new OSDNavBar(NAV_ID,getNavRect(),poIW,this,vec,vecShort,vecIDs);
    addChild(m_poNavBar);
    
    // BOTTOM-Bar----------------------------------------------------------
    m_poMain = new OSDButton(BOTTOM_MAIN_ID,getBottomRectMain(),m_poIW,this,"main");
    addChild(m_poMain);
    m_poExit = new OSDButton(BOTTOM_EXIT_ID,getBottomRectExit(),m_poIW,this,"exit");
    addChild(m_poExit);
    
    // MAIN-Menu-----------------------------------------------------------
    OSDWidget *w=0;
    w = new OSDWidget(MAIN_ID,getMainRect(),poIW,this);
    w->addChild(new OSDButton(MAIN_ADJUST_ID,getSubRect(0,5),m_poIW,this,"adjust"));
    w->addChild(new OSDButton(MAIN_FITMODE_ID,getSubRect(1,5),m_poIW,this,"fitmode"));
    w->addChild(new OSDButton(MAIN_CHANNELS_ID,getSubRect(2,5),m_poIW,this,"channels"));
    w->addChild(new OSDButton(MAIN_CAPTURE_ID,getSubRect(3,5),m_poIW,this,"capture"));
    w->addChild(new OSDButton(MAIN_INFO_ID,getSubRect(4,5),m_poIW,this,"info"));
    w->addChild(new OSDButton(MAIN_MENU_ID,getSubRect(5,5),m_poIW,this,"menu"));
    m_mapPanels["main"]=w;
    
    // ADJUST-Menu
    w = new OSDWidget(ADJUST_ID,getMainRect(),poIW,this);
    vec.clear(); vecShort.clear(),vecIDs.clear();
    vec.push_back("none");vecShort.push_back("none");vecIDs.push_back(ADJUST_MODE_NONE_ID);
    vec.push_back("manual");vecShort.push_back("man.");vecIDs.push_back(ADJUST_MODE_MANUAL_ID);
    vec.push_back("auto");vecShort.push_back("auto");vecIDs.push_back(ADJUST_MODE_AUTO_ID);
    
    w->addChild(new OSDNavBar(ADJUST_MODE_ID,getSubRect(1,8),poIW,this,vec,vecShort,vecIDs));
    w->addChild(new OSDLabel(ADJUST_BRIGHTNESS_LABEL_ID,getSubRect(3,8),m_poIW,this,"brightness:"));
    w->addChild(new OSDSlider(ADJUST_BRIGHTNESS_SLIDER_ID,getSubRect(4,8),m_poIW,this,-255,255,0));
    w->addChild(new OSDLabel(ADJUST_CONTRAST_LABEL_ID,getSubRect(5,8),m_poIW,this,"contrast:"));
    w->addChild(new OSDSlider(ADJUST_CONTRAST_SLIDER_ID,getSubRect(6,8),m_poIW,this,-255,255,0));
    w->addChild(new OSDLabel(ADJUST_INTENSITY_LABEL_ID,getSubRect(7,8),m_poIW,this,"intensity:"));
    w->addChild(new OSDSlider(ADJUST_INTENSITY_SLIDER_ID,getSubRect(8,8),m_poIW,this,-255,255,0));
    m_mapPanels["adjust"]=w;
    
    // FITMODE-Menu
    w = new OSDWidget(FITMODE_ID,getMainRect(),poIW,this); vec.clear(); vecShort.clear(),vecIDs.clear();
    vec.push_back("no scale");vecShort.push_back("no.s.");vecIDs.push_back(FITMODE_NOSCALE_ID);
    vec.push_back("hold ar");vecShort.push_back("h.ar");vecIDs.push_back(FITMODE_HOLDAR_ID);
    vec.push_back("fit");vecShort.push_back("fit");vecIDs.push_back(FITMODE_FIT_ID);

    ICLWidget::fitmode fm = poIW->getFitMode();
    int iSelectedIndex = 
      fm==ICLWidget::fmNoScale ? 0:
      fm==ICLWidget::fmHoldAR ? 1: 2;
    w->addChild(new OSDNavBar(FITMODE_ID,getSubRect(1,8),poIW,this,vec,vecShort,vecIDs, iSelectedIndex));
    m_mapPanels["fitmode"]= w;
    
    
    w = new OSDWidget(CHANNELS_ID,getMainRect(),poIW,this);
    w->addChild(new OSDLabel(CHANNELS_LABEL_ID,getSubRect(2,8),m_poIW,this,"select a channel:"));
    w->addChild(new OSDSlider(CHANNELS_SLIDER_ID,getSubRect(3,8),m_poIW,this,-1,4,-1));
    m_mapPanels["channels"]=w;
    
    // CAPTURE-Menu
    w = new OSDWidget(CAPTURE_ID,getMainRect(),poIW,this);  
    w->addChild(new OSDButton(CAPTURE_BUTTON_ID,getSubRect(2,3),m_poIW,this,"write current image"));
    m_mapPanels["capture"]=w;
    
    // INFO-Menu
    m_mapPanels["info"]=new OSDLabel(INFO_ID,getMainRect(),poIW,this,"info");


    // MENU-Menu
    w = new OSDWidget(MENU_ID,getMainRect(),poIW,this);
    Rect r0 = getSubRect(1,7);
    w->addChild(new OSDLabel(MENU_FILL_RGB_LABEL_ID,Rect(r0.x,r0.y,r0.width/2-1,r0.height),m_poIW,this,"fill(R,G,B):"));
    w->addChild(new OSDLabel(MENU_BORDER_RGB_LABEL_ID,Rect(r0.x+r0.width/2+1,r0.y,r0.width/2-1,r0.height),m_poIW,this,"edge(R,G,B):"));
    r0 = getSubRect(2,7);
    w->addChild(new OSDSlider(MENU_FILL_R_SLIDER_ID,Rect(r0.x,r0.y,r0.width/2-1,r0.height),m_poIW,this,0,255,s_iFillR));
    w->addChild(new OSDSlider(MENU_BORDER_R_SLIDER_ID,Rect(r0.x+r0.width/2+1,r0.y,r0.width/2-1,r0.height),m_poIW,this,0,255,s_iBorderR));
    r0 = getSubRect(3,7);
    w->addChild(new OSDSlider(MENU_FILL_G_SLIDER_ID,Rect(r0.x,r0.y,r0.width/2-1,r0.height),m_poIW,this,0,255,s_iFillG));
    w->addChild(new OSDSlider(MENU_BORDER_G_SLIDER_ID,Rect(r0.x+r0.width/2+1,r0.y,r0.width/2-1,r0.height),m_poIW,this,0,255,s_iBorderG));
    r0 = getSubRect(4,7);
    w->addChild(new OSDSlider(MENU_FILL_B_SLIDER_ID,Rect(r0.x,r0.y,r0.width/2-1,r0.height),m_poIW,this,0,255,s_iFillB));
    w->addChild(new OSDSlider(MENU_BORDER_B_SLIDER_ID,Rect(r0.x+r0.width/2+1,r0.y,r0.width/2-1,r0.height),m_poIW,this,0,255,s_iBorderB));

    w->addChild(new OSDLabel(MENU_ALPHA_LABEL_ID,getSubRect(6,7),m_poIW,this,"alpha:"));
    w->addChild(new OSDSlider(MENU_ALPHA_SLIDER_ID,getSubRect(7,7),m_poIW,this,0,255,s_iAlpha));
    m_mapPanels["menu"]= w;


    setActive("main");
  }      

  // }}}

  void OSD::childChanged(int id, void *val){
    // {{{ open

    switch(id){
      case BOTTOM_MAIN_ID:
        m_poNavBar->selectIndex(0);
      case NAV_MAIN_ID:
        setActive("main");
        break;
      case MAIN_ADJUST_ID:
        m_poNavBar->selectIndex(1);
      case NAV_ADJUST_ID:
        setActive("adjust");
        break;
      case MAIN_FITMODE_ID:
        m_poNavBar->selectIndex(2);
      case NAV_FITMODE_ID:
        setActive("fitmode");
        break;
      case MAIN_CHANNELS_ID:
        m_poNavBar->selectIndex(3);
      case NAV_CHANNELS_ID:
        setActive("channels");
        break;
      case MAIN_CAPTURE_ID:
        m_poNavBar->selectIndex(4);
      case NAV_CAPTURE_ID:
        setActive("capture");
        break;
      case MAIN_INFO_ID:
        m_poNavBar->selectIndex(5);
      case NAV_INFO_ID:
        setActive("info");
        setImageInfo(m_poIW->getImageInfo());
        break; 
      case MAIN_MENU_ID:
        m_poNavBar->selectIndex(6);
      case NAV_MENU_ID:
        setActive("menu");
        break;
      case MENU_FILL_R_SLIDER_ID:
        s_iFillR = *(int*)val;
        break;
      case MENU_FILL_G_SLIDER_ID:
        s_iFillG = *(int*)val;
        break;     
      case MENU_FILL_B_SLIDER_ID:
        s_iFillB = *(int*)val;
        break;
      case MENU_BORDER_R_SLIDER_ID:
        s_iBorderR = *(int*)val;
        break;
      case MENU_BORDER_G_SLIDER_ID:
        s_iBorderG = *(int*)val;
        break;
      case MENU_BORDER_B_SLIDER_ID:
        s_iBorderB = *(int*)val;
        break;
      case MENU_ALPHA_SLIDER_ID:
        s_iAlpha = *(int*)val;
        break;
      default:
        OSDWidget::childChanged(id,val);
    }
  }

  // }}}
  int OSD::getCurrID(){
    return m_iCurrID;
  }
  void OSD::setCurrID(int id){
    for(pmap::iterator it = m_mapPanels.begin();it!=m_mapPanels.end();++it){
      if((*it).second->getID() == id){
        if(m_iCurrID >= 0) removeChild(m_iCurrID);
        addChild((*it).second);
        m_iCurrID = id;
        return;
      }
    }
  }

  void OSD::setActive(string sName){
    // {{{ open

    if(m_iCurrID >= 0) removeChild(m_iCurrID);
    OSDWidget *w = m_mapPanels[sName]; 
    addChild(w);
    m_iCurrID = w->getID();
  }

  // }}}

  Rect OSD::getNavRect(){ 
    // {{{ open

    return Rect(x()+MARGIN,y()+MARGIN,w()-2*MARGIN,NAV_H); 
  }

  // }}}

  Rect OSD::getMainRect(){
    // {{{ open

    return Rect(x()+MARGIN,y()+MARGIN+GAP+NAV_H,w()-2*MARGIN,h()-2*MARGIN-2*GAP-NAV_H-BOTTOM_H); 
  }

  // }}}

  Rect OSD::getSubRect(int i, int iMax){
    // {{{ open

    Rect m = getMainRect();
    int h = rint((float)m.height/(float)(iMax+1));
    return Rect(m.x,m.y+i*h,m.width,h-2);
  } 

  // }}}

  Rect OSD::getBottomRectMain(){
    // {{{ open

    return Rect(x()+w()-MARGIN-GAP-2*BOTTOM_W,y()+h()-MARGIN-BOTTOM_H,BOTTOM_W,BOTTOM_H);
  }

  // }}}
  
  Rect OSD::getBottomRectExit(){
    // {{{ open

    return Rect(x()+w()-MARGIN-BOTTOM_W,y()+h()-MARGIN-BOTTOM_H,BOTTOM_W,BOTTOM_H); 
  }

  // }}}

  void OSD::setImageInfo(std::vector<string> info){
    // {{{ open
    if(m_iCurrID == INFO_ID){
      OSDLabel *w = (OSDLabel*)(m_mapPanels["info"]);
      w->setMultiText(info);      
    }
  }

  // }}}
}
