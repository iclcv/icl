#ifndef ICL_BOX_HANDLE_H
#define ICL_BOX_HANDLE_H

#include <QWidget>
#include <QLayout>
#include <iclGUIHandle.h>

namespace icl{
  
  /// A Handle for container GUI components (hbox and vbox) \ingroup HANDLES
  class BoxHandle : public GUIHandle<QWidget>{
    public:
    /// create an empty handle
    BoxHandle(): GUIHandle<QWidget>(0){}
    
    /// create a difined handle
    BoxHandle(QWidget *w):GUIHandle<QWidget>(w){}
    
    /// returns the associated layout
    QLayout *getLayout() { return (**this)->layout(); }
    
    /// adds an external compnent to the underlying widgets layout
    void add(QWidget *comp){ getLayout()->addWidget(comp); }
  };  
}

#endif
