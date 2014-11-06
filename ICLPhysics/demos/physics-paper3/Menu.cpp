#include <Menu.h>
#include <QtWidgets/QMenu>
#include <ICLUtils/StringUtils.h>

namespace icl{
  
  using namespace utils;

  struct Menu::Data{
    Menu::callback cb;
    QMenu men;
    std::map<std::string, QAction*> actions;
    std::map<std::string, QMenu*> menus;
  };
  
  
  Menu::Menu():m_data(new Data){
    
  }
  
  Menu::Menu(const std::string &commaSepEntries){
    addEntries(commaSepEntries);
  }
  
  Menu::~Menu(){
    delete m_data;
  }
    
  void Menu::addEntry(const std::string &entry){
    if(m_data->actions.find(entry) != m_data->actions.end()){
      throw ICLException("Menu::addEntry: given entry does already exist");
    }

    const std::vector<std::string> ts = tok(entry,".");
    if(!ts.size()) throw ICLException("Menu::addEntry: entry is empty!");
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

  void Menu::addEntries(const std::string &commaSepEntryList){
    const std::vector<std::string> entries = tok(commaSepEntryList,",");
    for(size_t i=0;i<entries.size();++i){
      addEntry(entries[i]);
    }
  }
  
  void Menu::setCallback(callback cb){
    m_data->cb = cb;
  }
  
  void Menu::show(const Point &screenPos){
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
