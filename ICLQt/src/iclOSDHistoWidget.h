#ifndef OSD_HISTO_WIDGET_LABEL_H
#define OSD_HISTO_WIDGET_LABEL_H

#include <iclOSDWidget.h>
#include <vector>
#include <QString>

namespace icl{
  
  /// Implementation of a ordinary label widget \ingroup UNCOMMON
  class OSDHistoWidget : public OSDWidget{
    public:
    OSDHistoWidget(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent):
    OSDWidget(id,r,poIW,poParent),m_bLogMode(false),m_bMeanMode(false),m_bMedianMode(false){}
    
    virtual void drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]);
    
    void setFeatures(bool logMode, bool meanMode, bool medianMode){
      m_bLogMode = logMode;
      m_bMeanMode = meanMode;
      m_bMedianMode = medianMode;
    }
    
    void setColor(int r, int g, int b){
      m_aiColor[0] = r;
      m_aiColor[1] = g;
      m_aiColor[2] = b;
    }
    
    void setHisto(const std::vector<int> &histo){
      m_vecHisto = histo;
    }
    
    private:
    float m_aiColor[3];
    std::vector<int> m_vecHisto;
    bool m_bLogMode;
    bool m_bMeanMode;
    bool m_bMedianMode;
    
  };
} // namespace

#endif
