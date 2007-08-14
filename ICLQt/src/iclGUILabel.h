#ifndef ICL_GUI_LABEL_H
#define ICL_GUI_LABEL_H

#include <string>
#include <QString>
#include <QWidget>
#include <iclSimpleMatrix.h>

namespace icl{
  
  class GUILabel{
    public:
    GUILabel():m_poGUILabel(0){}
    GUILabel(const std::string &initialText);
    
    void operator=(const std::string &text);
    void operator=(const QString &text);
    void operator=(const char *text);
    void operator=(int num);
    void operator=(double num);
    
    QWidget *getLabel() const { return m_poGUILabel; }

    private:
    QWidget *m_poGUILabel;
  };

  struct GUILabelAlloc{
    static GUILabel create() { return GUILabel(); }
  };
  typedef SimpleMatrix<GUILabel,GUILabelAlloc>  GUILabelMatrix;
}                               

#endif
