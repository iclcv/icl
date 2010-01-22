#ifndef ICL_GUI_HANDLE_BASE_H
#define ICL_GUI_HANDLE_BASE_H

#include <ICLQt/GUIWidget.h>

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
    friend class icl::MultiTypeMap;
    /** \endcond */
    
    /// returns parent GUIWidget pointer
    GUIWidget *getGUIWidget(){
      return m_poGUIWidget;
    }
    
    /// registers a callback on this gui widget
    /** This function can be re-implemented to bypass event propagation. By default,
        events are passed to the parent GUIWidget which, then again passes the events
        implementation-dependently to the actual widget.

        @param cb callback functor to call
        @param events comma separated list of event types to register on.
               This list is handled internally by each special GUIHandlerBase implementation
               currently only the ImageHandle DrawHandle and DrawHandle3D re-implement this 
               functionality. At default, events is not regarded at all 
    */
    virtual void registerCallback(GUI::CallbackPtr cb, const std::string &events="all"){
      (void)events;
      if(m_poGUIWidget){
        m_poGUIWidget->registerCallback(cb);
      }else{
        ERROR_LOG("unable to register a callback function on a null handle");
      }
    }
    
    /// removes all callbacks from parent GUIWidget component
    virtual void removeCallbacks(){
      if(m_poGUIWidget){
        m_poGUIWidget->removeCallbacks();
      }else{
        ERROR_LOG("unable to remove callbacks from a null handle");
      }
    }
    
    /// envokes all registered callbacks
    virtual void cb(){
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
