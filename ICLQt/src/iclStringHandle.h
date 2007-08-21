#ifndef ICL_STRING_HANDLE_H
#define ICL_STRING_HANDLE_H

#include <string>
#include <QString>
#include <iclGUIHandle.h>

/**\cond*/
class QLineEdit;
/**\endcond*/

namespace icl{
  
  /// Class for handling "int" textfield components \ingroup HANDLES
  class StringHandle : public GUIHandle<QLineEdit>{
    public:
    /// Create a new Int handle
    StringHandle(QLineEdit *le=0):GUIHandle<QLineEdit>(le){}
    
    /// makes the associated textfield show the given text
    void operator=(const std::string &text);
   
    /// makes the associated textfield show the given text
    void operator=(const QString &text){ (*this)=text.toLatin1().data(); }

    /// makes the associated textfield show the given text
    void operator=(const char *text) {(*this) = std::string(text); }
    
    /// returns the current text
    std::string getValue() const;
  };
}

#endif
