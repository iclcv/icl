#ifndef ICL_GUI_LABEL_H
#define ICL_GUI_LABEL_H

#include <string>
#include <QString>
#include <QWidget>
#include <iclGUIHandle.h>

/** \cond */
class QLabel;
/** \endcond */

namespace icl{
  /// Class for GUI-Label handling \ingroup HANDLES
  /** The gui label is created inside the GUI class, it can be used
      to make GUI-"label" components show anther text 
      @see GUI */
  class LabelHandle : public GUIHandle<QLabel>{
    public:
    /// Create a new LabelHandle
    LabelHandle(QLabel *l=0):GUIHandle<QLabel>(l){}
    
    ///  assign a std::string (makes the underlying label show that string)
    void operator=(const std::string &text);

    ///  assign a QString (makes the underlying label show that string)
    void operator=(const QString &text);

    ///  assign a const char* (makes the underlying label show that string)
    void operator=(const char *text);

    ///  assign an int (makes the underlying label show that integer)
    void operator=(int num);

    ///  assign a double (makes the underlying label show that double)
    void operator=(double num);
    
    private:
    /// utitlity function
    QLabel *lab() { return **this; }

    /// utitlity function
    const QLabel *lab() const { return **this; }
  };

 
  
 
}                               

#endif
