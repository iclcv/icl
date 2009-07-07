#include "iclGUIWidget.h"
#include "iclGUIDefinition.h"
#include "iclGUI.h"
#include "iclProxyLayout.h"

#include <QLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <iclCore.h>
#include <iclGUISyntaxErrorException.h>
#include <QString>

#include <iclIconFactory.h>

namespace icl{
  
  GUIWidget::GUIWidget(const GUIDefinition &def, 
                       GUIWidget::layoutType lt, 
                       int ensureNumInputs,
                       int ensureNumOutputs,
                       int ensureNumParams,
                       const Size &defMinSize):m_handle(0){
    if(ensureNumInputs > 0 && (int)def.numInputs() != ensureNumInputs){
      throw GUISyntaxErrorException(def.defString(),(QString("input count must be ")+QString::number(ensureNumInputs)+" here").toLatin1().data());
    } 
    if(ensureNumOutputs > 0 && (int)def.numOutputs() != ensureNumOutputs){
      throw GUISyntaxErrorException(def.defString(),(QString("output count must be ")+QString::number(ensureNumOutputs)+" here").toLatin1().data());
    } 
    if(ensureNumParams > 0 && (int)def.numParams() != ensureNumParams){
      throw GUISyntaxErrorException(def.defString(),(QString("param count must be ")+QString::number(ensureNumParams)+" here").toLatin1().data());
    } 

    m_poGridLayout = 0;
    m_poOtherLayout = 0;
    Size givenSize = def.size();
    Size givenMinSize = def.minSize();
    Size givenMaxSize = def.maxSize();
        
    if(givenSize != Size::null){
       setMinimumSize(QSize(givenSize.width*GUI::CELLW,givenSize.height*GUI::CELLH));
       setMaximumSize(QSize(givenSize.width*GUI::CELLW,givenSize.height*GUI::CELLH));
    }else if(givenMinSize != Size::null || givenMaxSize != Size::null){
      if(givenMinSize != Size::null){
        setMinimumSize(QSize(givenMinSize.width*GUI::CELLW,givenMinSize.height*GUI::CELLH));
      } 
      if(givenMaxSize != Size::null){
        setMaximumSize(QSize(givenMaxSize.width*GUI::CELLW,givenMaxSize.height*GUI::CELLH));
      }
    }else if(defMinSize != Size::null){
      //resize(QSize(defMinSize.width*GUI::CELLW,defMinSize.height*GUI::CELLH));
      setMinimumSize(QSize(defMinSize.width*GUI::CELLW,defMinSize.height*GUI::CELLH));
      //setMaximumSize(QSize(defSize.width*GUI::CELLW,defSize.height*GUI::CELLH));
    }
    
    if(def.parentLayout()) def.parentLayout()->addWidget(this);
    if(def.getProxyLayout()) def.getProxyLayout()->addWidget(this);

    switch(lt){
      case noLayout: 
        break;
      case hboxLayout:
        m_poOtherLayout = new QHBoxLayout;
        break;
      case vboxLayout:
        m_poOtherLayout = new QVBoxLayout;
        break;
      case gridLayout:
        m_poGridLayout = new QGridLayout;
        break;
    }
    if(m_poGridLayout){
      m_poGridLayout->setMargin(def.margin());
      m_poGridLayout->setSpacing(def.spacing());
      setLayout(m_poGridLayout);
    }else if(m_poOtherLayout){
      m_poOtherLayout->setMargin(def.margin());
      m_poOtherLayout->setSpacing(def.spacing());
      setLayout(m_poOtherLayout);
    }
    m_poGUI = def.getGUI();

    setWindowIcon(IconFactory::create_icl_window_icon_as_qicon());
    
    if(def.handle() != "") m_handle = new std::string(def.handle());
  }

  GUIWidget::~GUIWidget(){
    if(m_handle)delete m_handle;
  }
  
  void GUIWidget::ioSlot(){
    this->processIO();
    cb();
  }
  
  void GUIWidget::addToGrid(QWidget *widget, int x, int y, int w, int h){
    ICLASSERT_RETURN(m_poGridLayout && layout() == m_poGridLayout);
    m_poGridLayout->addWidget(widget,y,x,h,w);
  }

  void GUIWidget::cb(){
    if(m_handle){
      for(unsigned int i=0;i<m_vecCallbacks.size();++i){
        m_vecCallbacks[i]->exec();
          m_vecCallbacks[i]->exec(*m_handle);
      }
      }else{
      for(unsigned int i=0;i<m_vecCallbacks.size();++i){
        m_vecCallbacks[i]->exec();
      }
    }
  }

}


