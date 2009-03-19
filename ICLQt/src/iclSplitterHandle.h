#ifndef ICL_SPLITTER_HANDLE_H
#define ICL_SPLITTER_HANDLE_H

#include <QSplitter>
#include <iclGUIHandle.h>
#include <iclContainerHandle.h>

namespace icl{
  
  /// A Handle for SplitterWidget container GUI components  \ingroup HANDLES
  class SplitterHandle : public GUIHandle<QSplitter>, public ContainerHandle{
    public:
    /// create an empty handle
    SplitterHandle(): GUIHandle<QSplitter>(){}
    
    /// create a difined handle
    SplitterHandle(QSplitter *w, GUIWidget *guiw):GUIHandle<QSplitter>(w,guiw){}
    
    /// adds an external compnent to the splitter widget
    /** name is ignored */
    virtual void add(QWidget *comp, const QString &name=""){
      (**this)->addWidget(comp);
    }

    /// inserts a widget at givel location
    /** name is ignored */
    virtual void insert(int idx, QWidget *comp, const QString &name=""){
      (**this)->insertWidget(idx,comp);
    }
  };  
}

#endif
