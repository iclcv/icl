#ifndef ICL_BUTTON_GROUP_HANDLE_H
#define ICL_BUTTON_GROUP_HANDLE_H

#include <vector>
#include <string>
#include <ICLQt/GUIHandle.h>
/** \cond */
class QRadioButton;
/** \endcond */

namespace icl{

  /// type definition for the ButtonGroup handle \ingroup HANDLES
  typedef std::vector<QRadioButton*> RadioButtonVec;

  /// Class for handling button goups \ingroup HANDLE
  class ButtonGroupHandle : public GUIHandle<RadioButtonVec> {
    public:
    /// Create an empty handle
    ButtonGroupHandle(){}

    /// Craete a valid handle
    ButtonGroupHandle(RadioButtonVec *buttons, GUIWidget *w): GUIHandle<RadioButtonVec>(buttons,w){ }
    
    /// select a button with given index
    void select(int id);
    
    /// get the selected index
    int getSelected() const;

    /// get the text of the currently selected button
    std::string getSelectedText() const;
    
    /// returns the text of a button with given index
    std::string getText(int id) const;
    
    /// sets the text of a button with index ot a given text
    void setText(int id, const std::string &text);
    
    private:
    /// utility function (number of elements)
    int n() const{ return (int)vec().size(); }

    /// utility function (check indices for being valid)
    bool valid(int id) const{ return id >= 0 && id < (int)vec().size(); }

    /// utitliy function returns the underlying vector
    RadioButtonVec &vec() { return ***this; }

    /// utitliy function returns the underlying vector (const)
    const RadioButtonVec &vec() const { return const_cast<ButtonGroupHandle*>(this)->vec(); }
  };
  
}
#endif
