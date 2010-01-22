#ifndef ICL_GUI_LABEL_H
#define ICL_GUI_LABEL_H

#include <string>
#include <QString>
#include <QWidget>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/CompabilityLabel.h>


namespace icl{

  /// Class for GUI-Label handling \ingroup HANDLES
  /** The gui label is created inside the GUI class, it can be used
      to make GUI-"label" components show anther text 
      @see GUI */
  class LabelHandle : public GUIHandle<CompabilityLabel>{
    public:
    
    /// Create an empty handle
    LabelHandle(){}
    
    /// Create a new LabelHandle
    LabelHandle(CompabilityLabel *l, GUIWidget *w):GUIHandle<CompabilityLabel>(l,w){}
    
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
    
    /// appends text to the current text
    void operator+=(const std::string &text);
    
    private:
    /// utitlity function
    CompabilityLabel *lab() { return **this; }

    /// utitlity function
    const CompabilityLabel *lab() const { return **this; }
  };

 
  
 
}                               

#endif
