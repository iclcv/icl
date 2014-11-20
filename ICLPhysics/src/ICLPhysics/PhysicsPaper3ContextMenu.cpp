/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsPaper3ContextMenu.cpp **
** Module : ICLPhysics                                             **
** Author : Christof Elbrechter, Matthias Esau                     **
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
#include <ICLPhysics/PhysicsPaper3ContextMenu.h>
#include <QtWidgets/QMenu>
#include <ICLUtils/StringUtils.h>

namespace icl{
  
  using namespace utils;

  namespace physics{
    struct PhysicsPaper3ContextMenu::Data{
      PhysicsPaper3ContextMenu::callback cb;
      QMenu men;
      std::map<std::string, QAction*> actions;
      std::map<std::string, QMenu*> menus;
    };
  
  
    PhysicsPaper3ContextMenu::PhysicsPaper3ContextMenu():m_data(new Data){
    
    }
  
    PhysicsPaper3ContextMenu::PhysicsPaper3ContextMenu(const std::string &commaSepEntries){
      addEntries(commaSepEntries);
    }
  
    PhysicsPaper3ContextMenu::~PhysicsPaper3ContextMenu(){
      delete m_data;
    }
    
    void PhysicsPaper3ContextMenu::addEntry(const std::string &entry){
      if(m_data->actions.find(entry) != m_data->actions.end()){
        throw ICLException("PhysicsPaper3ContextMenu::addEntry: given entry does already exist");
      }

      const std::vector<std::string> ts = tok(entry,".");
      if(!ts.size()) throw ICLException("PhysicsPaper3ContextMenu::addEntry: entry is empty!");
      QMenu *men = &m_data->men;
      std::string path;
      for(size_t i=0;i<ts.size()-1;++i){
        path += ts[i] + '.';
        if(m_data->menus.find(path) != m_data->menus.end()){
          men = m_data->menus[path];
        }else{
          m_data->menus[path] = men->addMenu(ts[i].c_str());
          men = m_data->menus[path];
        }
      }
      m_data->actions[entry] = men->addAction(ts.back().c_str());
    }

    void PhysicsPaper3ContextMenu::addEntries(const std::string &commaSepEntryList){
      const std::vector<std::string> entries = tok(commaSepEntryList,",");
      for(size_t i=0;i<entries.size();++i){
        addEntry(entries[i]);
      }
    }
  
    void PhysicsPaper3ContextMenu::setCallback(callback cb){
      m_data->cb = cb;
    }
  
    void PhysicsPaper3ContextMenu::show(const Point &screenPos){
      QAction *a = m_data->men.exec(QPoint(screenPos.x,screenPos.y));
      if(a && m_data->cb){
        for(std::map<std::string, QAction*>::iterator it = m_data->actions.begin();
            it != m_data->actions.end();++it){
          if(it->second == a){
            m_data->cb(it->first);
            return;
          }
        }
      }
    }
  }

}
