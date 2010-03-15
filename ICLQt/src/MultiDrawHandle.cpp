/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLQt/src/MultiDrawHandle.cpp                          **
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
*********************************************************************/

#include <ICLQt/MultiDrawHandle.h>
#include <ICLQt/DrawWidget.h>
#include <ICLCore/ImgBase.h>

namespace icl{
   MultiDrawHandle::MultiDrawHandle():
    GUIHandle<ICLDrawWidget>(),m_imageBuffer(0),m_tabBar(0),m_bufferAll(false),m_bufferDeeply(true){
  }
  MultiDrawHandle::MultiDrawHandle(ICLDrawWidget *w, QTabBar *t,std::vector<ImgBase*> *imageBuffer, bool bufferAll, bool copyDeep, GUIWidget *guiw):
    GUIHandle<ICLDrawWidget>(w,guiw),m_imageBuffer(imageBuffer),m_tabBar(t),m_bufferAll(bufferAll),m_bufferDeeply(copyDeep){
    
    if(!t) return;
    for(int i=0;i<t->count();++i){
      std::string tabText = t->tabText(i).toLatin1().data();
      if(m_map.find(tabText) != m_map.end()){
        ERROR_LOG("Tab-Text "<<tabText<<" was found twice!");
      }else{
        m_map[tabText] = i;
      }
    }
    if(bufferAll){
      if(!imageBuffer){
        ERROR_LOG("this should not happen!");
        return;
      }
      imageBuffer->resize(t->count(),0);
      QObject::connect(m_tabBar,SIGNAL(currentChanged(int)),this,SLOT(tabChanged(int)));
    }    
  }

  MultiDrawHandle::MultiDrawHandle(const MultiDrawHandle &other):
    GUIHandle<ICLDrawWidget>(const_cast<ICLDrawWidget*>(*other), const_cast<MultiDrawHandle&>(other).getGUIWidget()){

    m_imageBuffer = other.m_imageBuffer;
    m_tabBar = other.m_tabBar;
    m_map = other.m_map;
    m_bufferAll = other.m_bufferAll;
    m_bufferDeeply = other.m_bufferDeeply;
    
    if(m_bufferAll){
      QObject::connect(m_tabBar,SIGNAL(currentChanged(int)),this,SLOT(tabChanged(int)));
    }
  }

  MultiDrawHandle::~MultiDrawHandle(){
    if(m_bufferAll){
      QObject::disconnect(m_tabBar,SIGNAL(currentChanged(int)),this,SLOT(tabChanged(int)));
    }
  }
  
  void MultiDrawHandle::tabChanged(int idx){
    if(m_bufferAll){
      if(!m_imageBuffer){
        ERROR_LOG("this should not happen!");
        return;
      }
      if(idx < (int)m_imageBuffer->size()){
        (**this)->setImage(m_imageBuffer->at(idx));  
        (**this)->updateFromOtherThread();
      }
    }
  }
  
  void MultiDrawHandle::Assigner::setImage(const ImgBase *image){
    /* original version:
        if(!d){
        ERROR_LOG("unable to set image to this index");
        }else if(idx == d->getSelectedIndex()){
        (**d)->setImage(image);
        }
    */
    if(!d){
      ERROR_LOG("unable to set image to this index");
      return;
    }
    if(d->m_bufferAll){
      if(d->m_bufferDeeply){
        if(image){
          image->deepCopy(&d->m_imageBuffer->at(this->idx));
        }else{
          ICL_DELETE(d->m_imageBuffer->at(this->idx));
        }
      }else{
        d->m_imageBuffer->at(this->idx) = const_cast<ImgBase*>(image);        
      }
    }
    
    if(idx == d->getSelectedIndex()){ // this must be performed in each case!
      (**d)->setImage(image);
    }
  }
    
  MultiDrawHandle::Assigner MultiDrawHandle::operator[](int idx){
    MultiDrawHandle::Assigner a;
    a.d = this;
    a.idx = idx;
    if(idx < 0 || idx >= getNumTabs()){
      ERROR_LOG("undefined assigner");
      a.d = 0;
      a.idx = 0;
    }
    return a;
  }
  MultiDrawHandle::Assigner MultiDrawHandle::operator[](const std::string &name){
    std::map<std::string,int>::iterator it = m_map.find(name);
    if(it!=m_map.end()){
      return (*this)[it->second];
    }else{
      ERROR_LOG("Tab " << name << " is not defined");
      Assigner a;
      a.d = 0;
      a.idx = 0;
      return a;
    }
  }
    
  void MultiDrawHandle::update(){
    (**this)->updateFromOtherThread();
  }
  
  int MultiDrawHandle::getSelectedIndex(){
    return m_tabBar->currentIndex();
  }
  int MultiDrawHandle::getNumTabs(){
    return m_tabBar->count();
  }
  std::string MultiDrawHandle::getSelected(){
    return m_tabBar->tabText(m_tabBar->currentIndex()).toLatin1().data();
  }
  bool MultiDrawHandle::isSelected(const std::string &text){
    return getSelected() == text;
  }
  
}
