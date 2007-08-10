#include "iclGUIWidget.h"
#include "iclGUIDefinition.h"
#include "iclGUI.h"
#include <QLayout>
#include <QGridLayout>
#include <iclCore.h>

namespace icl{
  
  GUIWidget::GUIWidget(const GUIDefinition &def, bool useGridLayout){
    Size s = def.size();
    Size defSize = getDefaultSize();
    if(s==Size::null) s = defSize;
    if(s!=Size::null){
      setMinimumSize(QSize(s.width*GUI::CELLW,s.height*GUI::CELLH));
      setMaximumSize(QSize(s.width*GUI::CELLW,s.height*GUI::CELLH));
    }
    
    if(def.getParentLayout()) def.getParentLayout()->addWidget(this);

    if(useGridLayout){
      m_poGridLayout = new QGridLayout;
      m_poGridLayout->setMargin(2);
      m_poGridLayout->setSpacing(2);
      setLayout(m_poGridLayout);
    }else{
      m_poGridLayout = 0;
    }
    m_poGUI = def.getGUI();
  }
  GUIWidget::~GUIWidget(){}
  
  void GUIWidget::ioSlot(){
    this->processIO();
  }
  
  void GUIWidget::addToGrid(QWidget *widget, int x, int y, int w, int h){
    ICLASSERT_RETURN(m_poGridLayout && layout() == m_poGridLayout);
    m_poGridLayout->addWidget(widget,y,x,h,w);
  }
}


