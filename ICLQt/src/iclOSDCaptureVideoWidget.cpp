#include <iclOSDCaptureVideoWidget.h>
#include <cmath>
#include <iclTime.h>
#include <iclOSDLabel.h>
#include <iclOSDButton.h>
#include <iclOSDSlider.h>
#include <iclCore.h>
#include <iclWidget.h>

using std::string;
namespace icl{
  namespace{
    
    class OSDStartStopPauseSymbolLable : public OSDWidget{
      ICLWidgetCaptureMode captureMode;
    public:

      void setCaptureMode(ICLWidgetCaptureMode captureMode){
        // {{{ open

        this->captureMode = captureMode;
      }

      // }}}

      OSDStartStopPauseSymbolLable(Rect r, ImageWidget *poIW, OSDWidget *poParent):
        // {{{ open

        OSDWidget(0,r,poIW,poParent){
      
      }

      // }}}
      void drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]){
        // {{{ open

        (void)x; (void)y;
        // save current color
        int color[4],fill[4];
        e->getColor(color);
        e->getFill(fill);
        Rect r = m_oRect.enlarged(-2);
        drawBG(e,1,1,mouseOver && !mouseOverChild, downmask[0]|| downmask[1]|| downmask[2]);
        switch(m_poIW->getCaptureMode()){
          case capturingStarted:{ // recording ...
            r = r.enlarged(-4);
            float alpha = 255 - 100* (1+::sin(Time::now().toMilliSecondsDouble()/250));;
            e->color(255,0,0,alpha);
            e->fill(0,0,0,0);
            e->ellipse(r);
            for(int i=0;i<4;++i){
              e->ellipse(r.enlarged(-2*i));
            }
            r = r.enlarged(4);
            break;
          }
          case capturingPaused:{ // pause ...
            e->color(255,255,255,255);
            r = r.enlarged(-4);
            Rect r1(r.x,r.y,r.width/3,r.height);
            Rect r2(r.x+0.66666*r.width,r.y,r.width/3,r.height);
            for(int i=0;i<4;++i){
              e->rect(r1.enlarged(-2*i));
              e->rect(r2.enlarged(-2*i));
            }
            r = r.enlarged(4);
            break;
          }
          default: // stopped
            r = r.enlarged(-4);
            e->color(255,255,255,255);
            for(int i=0;i<4;++i){
              e->rect(r.enlarged(-2*i));
            }
            r = r.enlarged(4);
            break;
        }
        
        e->color(255,255,255,200);
        //e->rect(r);
        // restore color
        e->color(color[0],color[1],color[2],color[3]);
        e->fill(fill[0],fill[1],fill[2],fill[3]);
        
      }

      // }}}
    };

    class OSDNextCapturingFileNameLabel : public OSDLabel{
    public: 
      OSDNextCapturingFileNameLabel(Rect r,ImageWidget *poIW, OSDWidget *poParent):
      OSDLabel(0,r,poIW,poParent,""){}

      virtual void drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]){
        setText(m_poIW->getNextCapturingFileName());
        OSDLabel::drawSelf(e,x,y,mouseOver,mouseOverChild,downmask);
      }
    };
  }
  
  OSDCaptureVideoWidget::OSDCaptureVideoWidget(int id, int startID, int stopID, int pauseID, int sliderID, Rect r,ImageWidget* poIW , OSDWidget *poParent):
    OSDWidget(id,r,poIW,poParent){

    
    /* LAYOUT

        ################# ################ ################# #########
        ##### START ##### ##### STOP ##### ##### PAUSE ##### #########
        ################# ################ ################# ## SYM ##
                                                             #########
        ## SL-CAP ## #### SLIDER ########################### #########
                                                             #########
        ################### TEXT ########################### #########
        
    */

    static const float SYM_W = 0.15;
    //static const int SYM_MAX_SIZE=50;
    static const int GAP = 2;
    static const float SLIDER_H = 0.3;
    static const float TEXT_H = 0.4;
    static const float SLIDER_CAPTION_W = 0.4;

    float buttonWidth = float(r.width-3*GAP-r.width*SYM_W)/3;
    int buttonHeight = (int)::round(r.height-2*GAP-r.height*SLIDER_H-r.height*TEXT_H);
    
    int sliderWidth = (int)::round(r.width-2*GAP-r.width*SYM_W-r.width*SLIDER_CAPTION_W);
    int sliderHeight = (int)::round(r.height*SLIDER_H);

    int textWidth = 3*buttonWidth+2*GAP;
    int textHeight = (int)::round(r.height*TEXT_H);
    
    int symWidth = (int)::round(r.width*SYM_W);
    int symHeight = r.height;
    
    int sliderCapWidth = (int)::round(r.width*SLIDER_CAPTION_W);
    int sliderCapHeight = sliderHeight;

    addChild(new OSDButton(startID,Rect(0,0,buttonWidth,buttonHeight),m_poIW,this,"record"));
    addChild(new OSDButton(stopID,Rect(buttonWidth+GAP,0,buttonWidth,buttonHeight),m_poIW,this,"stop"));
    addChild(new OSDButton(pauseID,Rect(2*buttonWidth+2*GAP,0,buttonWidth,buttonHeight),m_poIW,this,"pause"));

    addChild(new OSDLabel(0,Rect(0,buttonHeight+GAP,sliderCapWidth,sliderCapHeight),m_poIW,this,"skip frames"));
    addChild(new OSDSlider(sliderID,Rect(0+sliderCapWidth+GAP,buttonHeight+GAP,sliderWidth,sliderHeight),m_poIW,this,0,10,poIW->getCapturingFrameSkip()));
    
    addChild(new OSDStartStopPauseSymbolLable(Rect(3*GAP+3*buttonWidth,0,symWidth,symHeight),m_poIW,this));

    addChild(new OSDNextCapturingFileNameLabel(Rect(0,2*GAP+buttonHeight+sliderHeight,textWidth,textHeight),poIW,this));
  }
}
