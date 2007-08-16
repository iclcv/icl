#ifndef ICL_GUI_LABEL_H
#define ICL_GUI_LABEL_H

#include <string>
#include <QString>
#include <QWidget>
#include <iclSimpleMatrix.h>

namespace icl{
  /// Class for GUI-Label handling \ingroup COMMON  
  /** The gui label is created inside the GUI class, it can be used
      to make GUI-"label" components show anther text 
      @see GUI */
  class GUILabel{
    public:
    /// Create a new GUILabel
    GUILabel():m_poGUILabel(0){}
    
    /// Create a new GUI Label with given initial text (internally creates the label)
    GUILabel(const std::string &initialText);
    
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
    
    ///  retruns the underlying label
    QWidget *getLabel() const { return m_poGUILabel; }

    private:
    ///  Unterlying label
    QWidget *m_poGUILabel;
  };

  /** \cond */
  struct GUILabelAlloc{
    static GUILabel create() { return GUILabel(); }
  };
  /** \endcond */
  
  /// Type definition for handling GUI-"disp" components
  /** @see GUI, SimpleMatrix template class of the ICLUtils package */
  typedef SimpleMatrix<GUILabel,GUILabelAlloc>  GUILabelMatrix;
}                               

#endif
