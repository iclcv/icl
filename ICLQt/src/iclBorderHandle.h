#ifndef ICL_BORDER_HANDLE_H
#define ICL_BORDER_HANDLE_H

#include <iclGUIHandle.h>
#include <string>

/** \cond*/
class QGroupBox;
/** \endcond*/


namespace icl{
  /// Handle class for "border" gui components (only for explicit "border" components) \ingroup HANDLES
  class BorderHandle : public GUIHandle<QGroupBox>{
    public:
    /// Create a new border handle
    BorderHandle(QGroupBox *b=0):GUIHandle<QGroupBox>(b){}
    
    /// get the borders title string
    std::string getTitle() const;
    
    /// setup the border to show another title
    void operator=(const std::string &title);
  };
  
}

#endif
