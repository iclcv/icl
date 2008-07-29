#ifndef ICL_BORDER_HANDLE_H
#define ICL_BORDER_HANDLE_H

#include <iclGUIHandle.h>
#include <string>

/** \cond */
class QGroupBox;
/** \endcond */


namespace icl{
  /// Handle class for "border" gui components (only for explicit "border" components) \ingroup HANDLES
  class BorderHandle : public GUIHandle<QGroupBox>{
    public:
    /// Creates an empty border handle
    BorderHandle(){}

    /// Create a new border handle
    BorderHandle(QGroupBox *b, GUIWidget *w):GUIHandle<QGroupBox>(b,w){}
    
    /// get the borders title string
    std::string getTitle() const;
    
    /// setup the border to show another title
    void operator=(const std::string &title);
  };
  
}

#endif
