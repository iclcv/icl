#ifndef ICL_SPLITTER_HANDLE_H
#define ICL_SPLITTER_HANDLE_H

#include <QSplitter>
#include <iclGUIHandle.h>

namespace icl{
  
  /// A Handle for SplitterWidget container GUI components  \ingroup HANDLES
  class SplitterHandle : public GUIHandle<QSplitter>{
    public:
    /// create an empty handle
    SplitterHandle(): GUIHandle<QSplitter>(){}
    
    /// create a difined handle
    SplitterHandle(QSplitter *w, GUIWidget *guiw):GUIHandle<QSplitter>(w,guiw){}
    
    /// adds an external compnent to the splitter widget
    void add(QWidget *comp){
      (**this)->addWidget(comp);
    }

    /// inserts a widget at givel location
    void insert(int idx, QWidget *comp){
      (**this)->insertWidget(idx,comp);
    }
  };  
}

#endif
