#include <iclMultiDrawHandle.h>
#include <iclDrawWidget.h>
#include <iclImgBase.h>

namespace icl{
  
  MultiDrawHandle::MultiDrawHandle(ICLDrawWidget *w, QTabBar *t):
    GUIHandle<ICLDrawWidget>(w),m_tabBar(t){
  
    if(!t) return;
    for(int i=0;i<t->count();++i){
      std::string tabText = t->tabText(i).toLatin1().data();
      if(m_map.find(tabText) != m_map.end()){
        ERROR_LOG("Tab-Text "<<tabText<<" was found twice!");
      }else{
        m_map[tabText] = i;
      }
    }
    
  }
  
  void MultiDrawHandle::Assigner::setImage(const ImgBase *image){
    if(!d){
      ERROR_LOG("unable to set image to this index");
    }else if(idx == d->getSelectedIndex()){
      (**d)->setImage(image);
    }
  }
    
  MultiDrawHandle::Assigner MultiDrawHandle::operator[](int idx){
    MultiDrawHandle::Assigner a;
    a.d = this;
    a.idx = idx;
    if(idx < 0 || idx >= getNumTabs()){
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
