#include <iclOSDImageStatisticsWidget.h>
#include <iclWidget.h>
#include <iclStringUtils.h>

using namespace std;

namespace icl{
  
  OSDImageStatisticsWidget::OSDImageStatisticsWidget(int id, Rect r, ImageWidget* poIW , OSDWidget *poParent):
    OSDWidget(id,r,poIW,poParent){
  }
  void OSDImageStatisticsWidget::drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]){
    drawBG(e,true,true,mouseOver,downmask[0] || downmask[1] || downmask[2]);
    
    ImageStatistics s = m_poIW->getImageStatistics();
    
    Rect r = getRect();
    
    std::vector<string> v;
    v.push_back(string("Size  :")+translateSize(s.params.getSize()));
    v.push_back(string("Depth :")+translateDepth(s.d));
    v.push_back(string("Format:")+translateFormat(s.params.getFormat()));
    v.push_back(string("ROI   :")+translateRect(s.params.getROI()));

    int n = (int)v.size();
    int _h = this->h()/n;
    for(int i=0;i<n;i++){
      drawText(e,Rect(this->x(),this->y()+i*_h,this->w(),_h-2),v[i],0,0);
    }
    
  }
  
}
