#ifndef ICL_TOGGLE_BUTTON_H
#define ICL_TOGGLE_BUTTON_H

#include <QPushButton>

namespace icl{

  class ToggleButton : public QPushButton{
    Q_OBJECT;
    public:
    ToggleButton(const std::string &untoggledText, 
                 const std::string &toggledText,
                 QWidget *parent = 0,
                 bool *stateRef=0);
    ~ToggleButton();

    private slots:
    void toggleStateChanged(bool toggled);

    private:
    std::string m_text[2];
    bool *m_stateRef;
  };  
}

#endif
