#ifndef OSDIMAGE_STATISTICS_WIDGET_H
#define OSDIMAGE_STATISTICS_WIDGET_H

#include "iclOSDWidget.h"
#include "iclOSDNavBar.h"
#include "iclOSDButton.h"
#include "iclOSDHistoWidget.h"
#include <vector>
#include <QString>
#include <iclMutex.h>


namespace icl{
  
  /// Implementation of a ordinary label widget \ingroup UNCOMMON
  class OSDImageStatisticsWidget : public OSDWidget{
    public:
    OSDImageStatisticsWidget(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent);
    virtual void drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]);
    
    private:
    OSDNavBar *m_poChannelNavBar;
    Mutex m_oMutex;
    std::vector<OSDHistoWidget *> m_vecHistoWidgets;
    OSDButton *m_poLogButton;
    OSDButton *m_poMeanButton;    
    OSDButton *m_poMedianButton;
    OSDButton *m_poFillButton;
  };
} // namespace

#endif
