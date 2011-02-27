#include <ICLQt/HistogrammWidget.h>
#include <ICLUtils/Rect32f.h>

#include <QtGui/QPainter>

namespace icl{
  static inline int median_of_3(int a, int b, int c){
    if( a > b){
      if(a > c) return c;
      else return a;
    }else{
      if(b > c) return c;
      else return b;
    }
  }
  static inline int median_of_5(int *p){
    int a[5]= {p[0],p[1],p[2],p[3],p[4]};
    std::sort(a,a+5);
    return a[2];
  }
  
  static inline int mean_of_3(int a, int b, int c){
    return (a+b+b+c)/4;
  }
  
  static inline int mean_of_5(int *p){
    return (p[0]+3*p[1]+5*p[2]+3*p[3]+p[4])/13;
  }
  
  HistogrammWidget::HistogrammWidget(QWidget *parent):
    ThreadedUpdatableWidget(parent),logOn(false),meanOn(false),medianOn(false),
    fillOn(false),accuMode(false),selChannel(-1){
  }
    
  void HistogrammWidget::setFeatures(bool logOn, bool meanOn, bool medianOn, bool fillOn, int selChannel, bool accuMode){
    this->logOn = logOn;
    this->meanOn = meanOn;
    this->medianOn = medianOn;
    this->fillOn = fillOn;
    this->selChannel = selChannel;
    this->accuMode = accuMode;
  }
  void HistogrammWidget::fillColor(int i,float color[3]){
    switch(i){
      case 0: color[0]=255;color[1]=0;color[2]=0; break;
      case 1: color[0]=0;color[1]=255;color[2]=0; break;
      case 2: color[0]=0;color[1]=0;color[2]=255; break;
      default: color[0]=255;color[1]=255;color[2]=255; break;
    }
  }
  void HistogrammWidget::update(){
    QWidget::update();
  }
  void HistogrammWidget::update(const ImageStatistics &s){
    Mutex::Locker l(mutex);
    if(s.isNull){
      entries.clear();
      return;
    }
    
    // xxx todo process somewhere else 
    /** ImgParams params;
        depth d;
        std::vector<Range64f> ranges;
        ***/
    entries.resize(s.histos.size());
    for(unsigned int i=0;i<s.histos.size();++i){
      entries[i].histo = s.histos[i];
      fillColor(i,entries[i].color);
    }
    updateFromOtherThread();
  }
  
  void HistogrammWidget::paintEvent(QPaintEvent *e){
    Mutex::Locker l(mutex);
    QWidget::paintEvent(e);
    QPainter p(this);
    
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::NoBrush);
    p.setPen(QColor(50,50,50));
    p.drawRect(QRectF(0,0,width(),height()));
    
    
    static const int BORDER = 2;
    static float GAP = 0;
    Rect32f r = Rect32f(0,0,width(),height()).enlarged(-BORDER);
    
    std::vector<QPolygonF> polys(entries.size());
    if(fillOn){
      for(unsigned int e=0;e<entries.size();++e){
        polys[e] << QPointF(width(),height());
        polys[e] << QPointF(0,height());
      }
    }
    // TODO fix accuMode
    // TODO use overAll max elem to be able to compare bins
    float maxElem = 0;
    if(accuMode){
      for(unsigned int e=1;e<entries.size();++e){
        ICLASSERT_RETURN(entries[0].histo.size() == entries[e].histo.size());
      }
      for(unsigned int i=0;i<entries[0].histo.size();++i){
        int accu = 0;
        for(unsigned int e=1;e<entries.size();++e){
          accu += entries[e].histo[i];
        }
        if(accu > maxElem) maxElem = accu;
      }
    }

    std::vector<std::vector<int> > oldHistos(entries.size());
    for(unsigned int e=0;e<entries.size();++e){
      if(!(selChannel == -1 || selChannel == (int)e)) continue;
      std::vector<int> histo = entries[e].histo;
      if(!histo.size()) continue;
      int n = (int)histo.size();
        
      if(medianOn){
        histo[1] =  median_of_3(entries[e].histo[0],entries[e].histo[1],entries[e].histo[2]);
        for(int i=2;i<n-2;++i){
          histo[i] = median_of_5(&entries[e].histo[i-2]);
        }
        histo[n-2] =  median_of_3(entries[e].histo[n-3],entries[e].histo[n-2],entries[e].histo[n-1]);
      }else if(meanOn){
        histo[1] =  mean_of_3(entries[e].histo[0],entries[e].histo[1],entries[e].histo[2]);
        for(int i=2;i<n-2;++i){
          histo[i] = mean_of_5(&entries[e].histo[i-2]);
        }      
        histo[n-2] =  mean_of_3(entries[e].histo[n-3],entries[e].histo[n-2],entries[e].histo[n-1]);
      }
      
      p.setPen(QColor(entries[e].color[0],entries[e].color[1],entries[e].color[2]));

        
      if(!accuMode){
        maxElem = *max_element(histo.begin(),histo.end());
      }
      if(logOn) maxElem = ::log(maxElem);
        
      if(maxElem){
        float binDistance = r.width/n;
        float binWidth = binDistance-GAP;
          
        float lastX = 0,lastY=0;
        for(int i=0;i<n;i++){
          float val = histo[i];
          float valUnder = 0;
          if(accuMode){
            for(unsigned int j=0;j<e;++j){
              valUnder += oldHistos[j][i];
            }
            val += valUnder;
          }
            
          if(logOn && val) val=(::log(val));
            
          float h = (r.height/maxElem)*val;
          if(accuMode){
            h -= (r.height/maxElem)*valUnder;
          }
          float y = r.y+r.height-h;
          float x = r.x+(int)(i*binDistance);
          if(fillOn){
            // old xxx p.drawRect(QRectF(x,y,binWidth,h));
            polys[e] << QPointF(lastX+binWidth/2,lastY);
            if(i==n-1){
              polys[e] << QPointF(x+binWidth/2,y);
            }
          }else if(i>0){
            p.drawLine(QPointF(x+binWidth/2,y),QPointF(lastX+binWidth/2,lastY));
          }
          lastX = x;
          lastY = y;
        }
      }
      if(fillOn){
        p.setPen(QColor(entries[e].color[0],entries[e].color[1],entries[e].color[2],255));
        p.setBrush(QColor(entries[e].color[0],entries[e].color[1],entries[e].color[2],100));
        p.drawPolygon(polys[e]);
      }
      oldHistos[e]=histo;
    }
  }
}
