/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/GUIWidget.cpp                                **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQt/GUIWidget.h>
#include <ICLQt/GUIDefinition.h>
#include <ICLQt/GUI.h>
#include <ICLQt/ProxyLayout.h>

#include <QtGui/QLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <ICLCore/CoreFunctions.h>
#include <ICLQt/GUISyntaxErrorException.h>
#include <QtCore/QString>

#include <ICLQt/IconFactory.h>
#include <ICLUtils/StringUtils.h>

namespace icl{
  
  GUIWidget::GUIWidget(const GUIDefinition &def, 
                       int minParamCount, 
                       int maxParamCount,
                       layoutType lt, 
                       const Size &defMinSize):m_handle(0){
    int minPC = minParamCount;
    int maxPC = maxParamCount == -1 ? minPC : maxParamCount;
    
    int nP = (int)def.numParams();
    if(nP > maxPC || nP < minPC){
      throw GUISyntaxErrorException(def.defString(),"invalid parameter count! found " + str(nP) + " expected [" +str(minPC) + "-"
                                    + str(maxPC) + "] for component type " + def.type());
    } 

    m_poGridLayout = 0;
    m_poOtherLayout = 0;
    Size givenSize = def.size();
    Size givenMinSize = def.minSize();
    Size givenMaxSize = def.maxSize();
        
    m_preferredSize = Size(0,0); // default

    if(givenSize != Size::null){
      m_preferredSize = Size(givenSize.width*GUI::CELLW,givenSize.height*GUI::CELLH);
      // setGeometry(QRect(QPoint(0,0),QSize(givenSize.width*GUI::CELLW,givenSize.height*GUI::CELLH)));
      // setMinimumSize(QSize(givenSize.width*GUI::CELLW,givenSize.height*GUI::CELLH));
      // setMaximumSize(QSize(givenSize.width*GUI::CELLW,givenSize.height*GUI::CELLH));
      //setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred));
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

  QSize GUIWidget::sizeHint () const{
    if(m_preferredSize != Size(0,0)){
      return QSize(m_preferredSize.width,m_preferredSize.height);
    }else{
      return QWidget::sizeHint();
    }
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


