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
