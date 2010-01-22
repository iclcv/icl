#ifndef ICL_PROXY_LAYOUT_H
#define ICL_PROXY_LAYOUT_H

#include <ICLQt/GUIWidget.h>


namespace icl{
  /// just a helper class for GUI Layouting \ingroup UNCOMMON
  /** This class shall help to implement GUI container components,
      that do not use a QLayout for layouting e.g. QTabWidgets or
      QSplitters */
  struct ProxyLayout{
    /// defines how to add widges
    virtual void addWidget(GUIWidget *widget)=0;
  };  
}



#endif
