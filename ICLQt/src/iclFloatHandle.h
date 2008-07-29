#ifndef ICL_FLOAT_HANDLE_H
#define ICL_FLOAT_HANDLE_H

#include <iclGUIHandle.h>
/** \cond */
class QLineEdit;
/** \endcond */

namespace icl{
  
  /// Class for handling "float" textfield components \ingroup HANDLES
  class FloatHandle : public GUIHandle<QLineEdit> {
    public:
    
    /// Create an empty handle
    FloatHandle(){}
    
    /// Create a new FloatHandel
    FloatHandle(QLineEdit *le, GUIWidget *w):GUIHandle<QLineEdit>(le,w){}
    
    /// make the associated text field show a float
    void operator=(float f);
    
    /// returns the current text as float
    float getValue() const;
  };
}

#endif
