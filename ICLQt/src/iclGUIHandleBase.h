#ifndef ICL_GUI_HANDLE_BASE_H
#define ICL_GUI_HANDLE_BASE_H

#include <iclGUIWidget.h>

namespace icl{
  
  /// Base class for GUIHandles providing functions to register callbacks \ingroup UNCOMMON
  class GUIHandleBase{

    
    protected:
    /// create a new GUIHandleBase
    GUIHandleBase(GUIWidget *w=0):m_poGUIWidget(w){}
    public:

    /** \cond */
    friend class icl::GUI;
    friend class icl::DataStore;
    /** \endcond */
    
    /// returns parent GUIWidget pointer
    GUIWidget *getGUIWidget(){
      return m_poGUIWidget;
    }
    
    /// registers a callback on this gui widget
    void registerCallback(GUI::CallbackPtr cb){
      if(m_poGUIWidget){
        m_poGUIWidget->registerCallback(cb);
      }else{
        ERROR_LOG("unable to register a callback function on a null handle");
      }
    }
    
    void removeCallbacks(){
      if(m_poGUIWidget){
        m_poGUIWidget->removeCallbacks();
      }else{
        ERROR_LOG("unable to remove callbacks from a null handle");
      }
    }
    
    /// envokes all registered callbacks to be called!
    void cb(){
      if(m_poGUIWidget){
        m_poGUIWidget->cb();
      }else{
        ERROR_LOG("unable to call callbacks on a null handle");
      }
    }
    
    private:
    GUIWidget *m_poGUIWidget;
  };
}

#endif
