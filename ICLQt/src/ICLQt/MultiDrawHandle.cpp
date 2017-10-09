/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/MultiDrawHandle.cpp                    **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLQt/MultiDrawHandle.h>
#include <ICLQt/DrawWidget.h>
#include <ICLCore/ImgBase.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace qt{
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
          (**this)->render();
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

    void MultiDrawHandle::render(){
      (**this)->render();
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

  } // namespace qt
}
