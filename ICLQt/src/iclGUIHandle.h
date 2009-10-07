#ifndef ICL_GUI_HANDLE_H
#define ICL_GUI_HANDLE_H

#include <iclGUIHandleBase.h>

namespace icl{
  /// Abstract base class for Handle classes \ingroup HANDLES
  template <class T>
  class GUIHandle : public GUIHandleBase{

    protected:
    /// as GUIHandle is just an interface, its base constructor is protected
    GUIHandle():m_poContent(0){}

    /// as GUIHandle is just an interface, its base constructor is protected
    GUIHandle(T *t, GUIWidget *w):GUIHandleBase(w),m_poContent(t){}
    
    public:
    
    /// use the *-oprator to get the wrapped component (const)
    const T *operator*() const{
      return m_poContent;
    }

    /// use the *-oprator to get the wrapped component (unconst)
    T *&operator*(){ return m_poContent; }
    
    /// this can be used for direct access to wrapped type
    T* operator->(){ return m_poContent; }

    /// this can be used for direct access to wrapped type
    const T* operator->() const{ return m_poContent; }
    
    private:
    /// wrapped component
    T *m_poContent;
  };
}

#endif
