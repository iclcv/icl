#ifndef ICL_GUI_HANDLE_H
#define ICL_GUI_HANDLE_H

namespace icl{
  /// Abstract base class for Handle classes \ingroup HANDLES
  template <class T>
  class GUIHandle{
    protected:
    /// as GUIHandle is just an interface, its base constructor is protected
    GUIHandle(T *t=0):m_poContent(t){}
    
    /// use the *-oprator to get the wrapped component (const)
    const T *operator*() const{
      return m_poContent;
    }

    /// use the *-oprator to get the wrapped component (unconst)
    T *&operator*(){ return m_poContent; }

    private:
    /// wrapped component
    T *m_poContent;
  };
}

#endif
