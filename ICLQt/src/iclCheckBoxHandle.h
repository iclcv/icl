#ifndef ICL_CHECK_BOX_HANDLE_H
#define ICL_CHECK_BOX_HANDLE_H

#include <string>
#include <vector>
#include <iclGUIHandle.h>

/**\cond */
class QCheckBox;
/**\endcond */


namespace icl{
  
  /// Special Utiltiy class for handling Button clicks in the ICL GUI API \ingroup HANDLES
  class CheckBoxHandle : public GUIHandle<QCheckBox>{
    public:
    
    /// creates a n empty button handle
    CheckBoxHandle();

    /// create a new event with a given button id
    CheckBoxHandle(QCheckBox *cb, GUIWidget *w, bool *stateRef);

    /// checks this checkbox
    void check(bool execCallbacks=true);

    // unchecks this checkbox
    void uncheck(bool execCallbacks=true);

    // returns whether this the checkbox is currently checked
    bool isChecked() const;
    
    private:

    /// internal state reference variable
    bool *m_stateRef;

  };  
}

#endif
