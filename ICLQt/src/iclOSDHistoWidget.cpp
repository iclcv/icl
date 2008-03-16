#include "iclOSDHistoWidget.h"
#include <algorithm>
#include <cmath>

namespace icl{
  
  namespace{
    inline int median_of_3(int a, int b, int c){
      if( a > b){
        if(a > c) return c;
        else return a;
      }else{
        if(b > c) return c;
        else return b;
      }
    }
    inline int median_of_5(int *p){
      int a[5]= {p[0],p[1],p[2],p[3],p[4]};
      std::sort(a,a+5);
      return a[2];
    }

    inline int mean_of_3(int a, int b, int c){
      return (a+b+b+c)/4;
    }
    
    inline int mean_of_5(int *p){
      return (p[0]+3*p[1]+5*p[2]+3*p[3]+p[4])/13;
    }

  }
  
  
  void OSDHistoWidget::drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]){
    if(!m_vecHisto.size()) return;
    static const int BORDER = 2;
    static float GAP = 0;
    Rect r = getRect().enlarged(-BORDER);
    
    int colorSave[4],fillSave[4];
    e->getColor(colorSave);
    e->getFill(fillSave);
    
    std::vector<int> histo = m_vecHisto;
    
    int n = (int)histo.size();

    if(m_bMedianMode){
      histo[1] =  median_of_3(m_vecHisto[0],m_vecHisto[1],m_vecHisto[2]);
      for(int i=2;i<n-2;++i){
        histo[i] = median_of_5(&m_vecHisto[i-2]);
      }
      histo[n-2] =  median_of_3(m_vecHisto[n-3],m_vecHisto[n-2],m_vecHisto[n-1]);
    }else if(m_bMeanMode){
      histo[1] =  mean_of_3(m_vecHisto[0],m_vecHisto[1],m_vecHisto[2]);
      for(int i=2;i<n-2;++i){
        histo[i] = mean_of_5(&m_vecHisto[i-2]);
      }      
      histo[n-2] =  mean_of_3(m_vecHisto[n-3],m_vecHisto[n-2],m_vecHisto[n-1]);
    }
    
    
    
    e->color(m_aiColor[0],m_aiColor[1],m_aiColor[2],255);
    e->fill(m_aiColor[0],m_aiColor[1],m_aiColor[2],255);
    
    float maxElem = *max_element(histo.begin(),histo.end());
    if(m_bLogMode) maxElem = ::log(maxElem);

    if(maxElem){
      float binDistance = float(r.width)/n;
      float binWidth = binDistance-GAP;
      
      int lastX = 0;
      int lastY = 0;
      for(int i=0;i<n;i++){
        float val = histo[i];
        if(m_bLogMode && val) val=(::log(val));
        int h = (float(r.height)/maxElem)*val;
        int y = r.y+r.height-h;
        int x = r.x+(int)(i*binDistance);
        if(m_bDrawFilled){
          e->rect(Rect(x,y,(int)binWidth,h));
        }else{
          e->rect(Rect(x,y,(int)binWidth,2));
          if(i>0){
            e->line(Point(x+binWidth/2,y),Point(lastX+binWidth/2,lastY));
          }
          lastX = x;
          lastY = y;
        }
        
      }
    }

    e->color(colorSave[0],colorSave[1],colorSave[1],colorSave[1]);
    e->fill(fillSave[0],fillSave[1],fillSave[1],fillSave[1]);
    
  }

} 

