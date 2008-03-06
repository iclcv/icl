#include <iclOSDImageStatisticsWidget.h>
#include <iclWidget.h>
#include <iclStringUtils.h>

using namespace std;

namespace icl{
  
  namespace{
    static const int OSDISW_CHANNEL_NAV_BAR_ID = 100000;
    static const int OSDISW_CHANNEL_NAV_BAR_ID_DEFRGB = OSDISW_CHANNEL_NAV_BAR_ID+1;
    static const int OSDISW_CHANNEL_NAV_BAR_ID_CHANNELS[3] = {
      OSDISW_CHANNEL_NAV_BAR_ID_DEFRGB+1,
      OSDISW_CHANNEL_NAV_BAR_ID_DEFRGB+2,
      OSDISW_CHANNEL_NAV_BAR_ID_DEFRGB+3
    };
    static const int OSDISW_IMAGE_TEXT_INFO_ID = OSDISW_CHANNEL_NAV_BAR_ID_DEFRGB+4;
    static const int OSDISW_HISTO_WIDGET_IDS[3] = {
      OSDISW_IMAGE_TEXT_INFO_ID+1,
      OSDISW_IMAGE_TEXT_INFO_ID+2,
      OSDISW_IMAGE_TEXT_INFO_ID+3
    };
    
    static const int  OSDISW_LOG_BUTTON_ID = OSDISW_IMAGE_TEXT_INFO_ID+4;
    static const int  OSDISW_MEAN_BUTTON_ID = OSDISW_IMAGE_TEXT_INFO_ID+5;
    static const int  OSDISW_MEDIAN_BUTTON_ID = OSDISW_IMAGE_TEXT_INFO_ID+6;
    
    
    static const int GAP = 2;
    static const int BORDER = 2;
    static const float REL_NAVBAR_H = 0.12;
    static const float REL_HISTO_W = 0.7;
    static const int ABS_BUTTON_H = 20;

    Rect get_channel_navbar_rect(const Rect &r){
      // {{{ open

      return Rect(0,1*BORDER,r.width,(int)(REL_NAVBAR_H*(r.height-2*BORDER)));
    }

    // }}}
    
    vector<string> get_channel_navbar_names(int n){
      // {{{ open

      vector<string> v(n+1);
      v[0] = string("RGB");
      for(int i=0;i<n && i < 3;i++){
        v[i+1] = string("Channel ")+str(i);
      }
      return v;
    }

    // }}}

    vector<string> get_channel_navbar_short_names(int n){
      // {{{ open

      vector<string> v(n+1);
      v[0] = string("RGB");
      for(int i=0;i<n && i < 3;i++){
        v[i+1] = string("C-")+str(i);
      }
      return v;
    }

    // }}}
    vector<int> get_channel_navbar_ids(int n){
      // {{{ open

      vector<int> ids(n+1);
      ids[0] = OSDISW_CHANNEL_NAV_BAR_ID_DEFRGB;
      for(int i=0;i<n && i<3;i++){
        ids[i+1] = OSDISW_CHANNEL_NAV_BAR_ID_CHANNELS[i];
      }
      return ids;
    }

    // }}}
    
    Rect get_histo_widget_rect(const Rect &r){
      float useHistoW = r.width > 300 ? REL_HISTO_W : REL_HISTO_W*0.7;
      return Rect(0,
                  0+BORDER+r.height*REL_NAVBAR_H+GAP,
                  r.width*useHistoW,
                  r.height*(1.0-REL_NAVBAR_H)-2*BORDER-GAP);
    } 
    
    /// r is histo widget rect
    Rect get_image_info_rect(const Rect &hwRect, Rect parentRect){
      return Rect(hwRect.x+hwRect.width+2*GAP,
                  hwRect.y,
                  parentRect.width-hwRect.width-GAP-BORDER,
                  hwRect.height-ABS_BUTTON_H-GAP);
    }
    
    Rect get_button_rect(const Rect &r){
      float useHistoW = r.width > 300 ? REL_HISTO_W : REL_HISTO_W*0.7;
      return Rect(r.width*useHistoW+GAP+BORDER,
                  BORDER+r.height*REL_NAVBAR_H+r.height*(1.0-REL_NAVBAR_H)-2*BORDER-ABS_BUTTON_H,
                  r.width * (1.0-useHistoW)-GAP-BORDER,
                  ABS_BUTTON_H);
    }

    void set_up_histo_color(std::vector<OSDHistoWidget *> &v, format f){
      (void)f;
      v[0]->setColor(230,20,50);
      v[1]->setColor(50,230,30);
      v[2]->setColor(30,50,255);
    }
    
    
    void update_navbar_channel_names(OSDNavBar *p, format f, int channels){
      switch(f){
        case formatRGB:
          p->updateName(0,"RGB","RGB");
          p->updateName(1,"R","Red"); 
          p->updateName(2,"G","Green"); 
          p->updateName(3,"B","Blue"); 
          break;
        case formatHLS:
          p->updateName(0,"HLS","HLS");
          p->updateName(1,"H","Hue"); 
          p->updateName(2,"L","Lightness"); 
          p->updateName(3,"S","Saturation"); 
          break;
        case formatLAB:
          p->updateName(0,"LAB","CIE-L*A*B*");
          p->updateName(1,"L*","Lightness"); 
          p->updateName(2,"A*","A*:(G-->M)"); 
          p->updateName(3,"B*","B*:(B-->Y)"); 
          break;
        case formatYUV:
          p->updateName(0,"YUV","YUV(YCbCr)");
          p->updateName(1,"Y","Luma"); 
          p->updateName(2,"U","U:|B-Y|"); 
          p->updateName(3,"V","V:|R-Y|");
          break;
        case formatChroma:
          p->updateName(0,"Chroma","RG-Chromaticity");
          p->updateName(1,"R","R:r/(r+g+b)"); 
          p->updateName(2,"G","G:g/(r+g+b)"); 
          break;
        case formatGray:
          p->updateName(0,"Gray","Intensity");
          p->updateName(1,"C-0","Channel-0");
          break;
        case formatMatrix:
          p->updateName(0,"ALL","ALL");
          for(int i=1;i<channels && i<4;i++){
            p->updateName(i,string("C-")+str(i-1),string("Channel ")+str(i-1));            
          }
          break;
        default:
          break;
          
      }
    }
    vector<string> get_info_string_vec(const ImageStatistics &s, int maxChannels){
      vector<string> v;
      v.push_back(string("Size: ") + translateSize(s.params.getSize()));
      v.push_back(string("Channels: ") + str(s.params.getChannels()));
      v.push_back(string("Depth: ") + translateDepth(s.d));
      v.push_back(string("Format: ") + translateFormat(s.params.getFormat()));
      v.push_back(string("ROI: ") + translateRect(s.params.getROI()));
      
      for(int i=0;i<maxChannels;i++){
        v.push_back(string("C-")+str(i)+":");
        v.push_back(string("[")+str(s.ranges[i].minVal)+","+str(s.ranges[i].maxVal)+"]");
      }
      return v;
    }
  }
  
  OSDImageStatisticsWidget::OSDImageStatisticsWidget(int id, Rect r, ImageWidget* poIW , OSDWidget *poParent):
    // {{{ open

    OSDWidget(id,r,poIW,poParent){

    m_poChannelNavBar = 0;

    for(unsigned int i=0;i<3;i++){
      m_vecHistoWidgets.push_back(new OSDHistoWidget(OSDISW_HISTO_WIDGET_IDS[i],get_histo_widget_rect(r),m_poIW,this));
      addChild(m_vecHistoWidgets[i]);
    }

    Rect br = get_button_rect(r);
    m_poLogButton = new OSDButton(OSDISW_LOG_BUTTON_ID,Rect(br.x,br.y,float(br.width-2*GAP)/3,br.height),poIW,this,"log",true);
    addChild( m_poLogButton );
    m_poMeanButton = new OSDButton(OSDISW_MEAN_BUTTON_ID,Rect(1+GAP+br.x+float(br.width-2*GAP)/3,br.y,float(br.width-2*GAP)/3,br.height),poIW,this,"blur",true);
    addChild( m_poMeanButton );
    m_poMedianButton = new OSDButton(OSDISW_MEDIAN_BUTTON_ID,Rect(1+2*GAP+br.x+2*float(br.width-2*GAP)/3,br.y,float(br.width-2*GAP)/3,br.height),poIW,this,"median",true);
    addChild( m_poMedianButton );
  }

  // }}}

  void OSDImageStatisticsWidget::drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]){
    // {{{ open

    Mutex::Locker l(m_oMutex);
    ImageStatistics s = m_poIW->getImageStatistics();

    Rect r = getRect();
   
    if(s.isNull){
      drawRect(e,r,true,true,false,false);
      drawText(e,r,"Image is NULL",false,false,true);
      return;
    }

    // Channel count
    int cc = s.params.getFormat() == formatMatrix ? (int)s.ranges.size() : getChannelsOfFormat(s.params.getFormat());
    
    // updating the channel navbar
    if(!m_poChannelNavBar || m_poChannelNavBar->getItemCount()-1 != cc){
      if(m_poChannelNavBar){
        removeChild(OSDISW_CHANNEL_NAV_BAR_ID);
        delete m_poChannelNavBar;
      }
      m_poChannelNavBar = new OSDNavBar(OSDISW_CHANNEL_NAV_BAR_ID,
                                        get_channel_navbar_rect(r),
                                        m_poIW,this,
                                        get_channel_navbar_names(iclMin(3,cc)),
                                        get_channel_navbar_short_names(iclMin(3,cc)),
                                        get_channel_navbar_ids(iclMin(3,cc)));
      addChild(m_poChannelNavBar);
      
    }
    update_navbar_channel_names(m_poChannelNavBar,s.params.getFormat(),cc);
    
    if(m_poChannelNavBar->getSelectedIndex() == 0){
      for(int i=0;i<cc;i++){
        m_vecHistoWidgets[i]->setHisto(s.histos[i]);
      }
      for(int i=cc;i<3;i++){
        m_vecHistoWidgets[i]->setHisto(std::vector<int>());
      }
    }else{
      int idx = m_poChannelNavBar->getSelectedIndex()-1;
      for(int i=0;i<3;i++){
        m_vecHistoWidgets[i]->setHisto(i==idx ? s.histos[i] : std::vector<int>());
      }
    }

    for(int i=0;i<cc;++i){
      m_vecHistoWidgets[i]->setFeatures(m_poLogButton->isToggled(),
                                        m_poMeanButton->isToggled(),
                                        m_poMedianButton->isToggled());
    }
    
    set_up_histo_color(m_vecHistoWidgets,s.params.getFormat());

    drawRect(e,m_vecHistoWidgets[0]->getRect(),true,true,mouseOver,false);
    e->color(0,0,0,0);
    e->fill(0,0,50,200);
    e->rect(m_vecHistoWidgets[0]->getRect().enlarged(-BORDER));

    Rect iiR = get_image_info_rect(m_vecHistoWidgets[0]->getRect(),getRect());
    drawRect(e,iiR,true,true,false,false);
    
    std::vector<string> infovec = get_info_string_vec(s,cc);

    e->color(255,255,255,255);   
    e->fill(255,255,255,255);
    
    int fontSizeSave = e->getFontSize();
    e->fontsize(0.7*fontSizeSave);
    for(unsigned int i=0;i<infovec.size();i++){
      Rect currR(iiR.x+BORDER,iiR.y+i* float(iiR.height-2*BORDER)/infovec.size(), iiR.width-2*BORDER, float(iiR.height-2*BORDER)/infovec.size());
      e->text(currR,infovec[i],PaintEngine::NoAlign);
    }
    e->fontsize(fontSizeSave);
  }

  // }}}

 
  
}
