#ifndef ICL_COMBO_HANDLE_H
#define ICL_COMBO_HANDLE_H

#include <string>
#include <iclGUIHandle.h>

/** \cond */
class QComboBox;
/** \endcond */

namespace icl{
  
  /// Handle class for combo components \ingroup HANDLES
  class ComboHandle : public GUIHandle<QComboBox>{
    public:
    
    /// create a new ComboHandle wrapping a given QComboBox
    ComboHandle(QComboBox *cb=0):GUIHandle<QComboBox>(cb){}
    
    /// add an item
    void add(const std::string &item);

    /// remove an item
    void remove(const std::string &item);
    
    /// remove item at given index
    void remove(int idx);
    
    /// void remove all items
    void clear();
    
    /// returns the item at given index
    std::string getItem(int idx) const;

    /// returns the index of a given item
    int getIndex(const std::string &item) const;
    
    /// returns the currently selected index
    int getSelectedIndex() const;
    
    /// returns the currently selected item
    std::string getSelectedItem() const;
    
    /// returns the count of elements
    int getItemCount() const;
    
    /// sets the current index
    void setSelectedIndex(int idx);
    
    /// sets the current item
    void setSelctedItem(const std::string &item);
    
    private:
    /// utility function (internally used)
    QComboBox *cb() { return **this; } 

    /// utility function (internally used)
    const QComboBox *cb() const{ return **this; } 
  };
  
}


#endif
