#ifndef ICL_BOX_HANDLE_H
#define ICL_BOX_HANDLE_H

#include <QWidget>
#include <QLayout>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/ContainerHandle.h>

namespace icl{
  
  /// A Handle for container GUI components (hbox and vbox) \ingroup HANDLES
  class BoxHandle : public GUIHandle<QWidget>, public ContainerHandle{
    public:
    /// create an empty handle
    BoxHandle(): GUIHandle<QWidget>(){}
    
    /// create a difined handle
    BoxHandle(QWidget *w, GUIWidget *guiw):GUIHandle<QWidget>(w,guiw){}
    
    /// returns the associated layout
    QLayout *getLayout() { return (**this)->layout(); }
    
    /// adds an external compnent to the underlying widgets layout
    /** name is ignored */
    virtual void add(QWidget *comp, const QString &name=""){ getLayout()->addWidget(comp); }

    /// this does not work here (calls add, idx and name is ignored)
    virtual void insert(int idx, QWidget *comp, const QString &name=""){add(comp,name); }
  };  
}

#endif
