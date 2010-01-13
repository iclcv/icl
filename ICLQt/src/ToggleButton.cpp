#include <ICLQt/ToggleButton.h>

namespace icl{

  ToggleButton::ToggleButton(const std::string &untoggledText, 
                             const std::string &toggledText,
                             QWidget *parent,
                             bool *stateRef):
    QPushButton(untoggledText.c_str(),parent),m_stateRef(stateRef){
    
    setCheckable(true);
    
    m_text[0] = untoggledText;
    m_text[1] = toggledText;
    
    connect(this,SIGNAL(toggled(bool)),this,SLOT(toggleStateChanged(bool)));
  }
  
  ToggleButton::~ToggleButton(){
    disconnect(this,SIGNAL(toggled(bool)),this,SLOT(toggleStateChanged(bool)));
  }
    
  void ToggleButton::toggleStateChanged(bool toggled){
    if(toggled) toggled = 1; // I'm not shure if a boolean is always 0x1
    setText(m_text[toggled].c_str());
    if(m_stateRef) *m_stateRef = toggled;
  }  
}
