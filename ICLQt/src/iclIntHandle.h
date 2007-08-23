#ifndef ICL_INT_HANDLE_H
#define ICL_INT_HANDLE_H

#include <iclGUIHandle.h>

/** \cond */
class QLineEdit;
/** \endcond */

namespace icl{
  
  /// Class for handling "int" textfield components \ingroup HANDLES
  class IntHandle : public GUIHandle<QLineEdit>{
    public:
    /// Create a new Int handle
    IntHandle(QLineEdit *le=0):GUIHandle<QLineEdit>(le){}
    
    /// makes the associated textfield show the given value
    void operator=(int i);
    
    /// returns the current text as int
    int getValue() const;
  };
}

#endif
