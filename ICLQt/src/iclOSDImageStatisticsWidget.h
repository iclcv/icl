#ifndef OSDIMAGE_STATISTICS_WIDGET_H
#define OSDIMAGE_STATISTICS_WIDGET_H

#include <iclOSDWidget.h>
#include <vector>
#include <QString>

namespace icl{
  
  /// Implementation of a ordinary label widget \ingroup UNCOMMON
  class OSDImageStatisticsWidget : public OSDWidget{
    public:
    OSDImageStatisticsWidget(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent);
    virtual void drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]);
  };
} // namespace

#endif
