/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/ComboHandle.h                            **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_COMBO_HANDLE_H
#define ICL_COMBO_HANDLE_H

#include <string>
#include <ICLQt/GUIHandle.h>

/** \cond */
class QComboBox;
/** \endcond */

namespace icl{
  
  /// Handle class for combo components \ingroup HANDLES
  class ComboHandle : public GUIHandle<QComboBox>{
    public:
    /// create an empty handle
    ComboHandle(){}

    /// create a new ComboHandle wrapping a given QComboBox
    ComboHandle(QComboBox *cb, GUIWidget *w):GUIHandle<QComboBox>(cb,w){}
    
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
    void setSelectedItem(const std::string &item);

    /// convenience operator wrapping getSelectedIndex
    inline operator int() const { return getSelectedIndex(); }

    /// convenience operator wrapping getSelectedItem
    inline operator std::string() const { return getSelectedItem(); }
    
    private:
    /// utility function (internally used)
    QComboBox *cbx() { return **this; } 

    /// utility function (internally used)
    const QComboBox *cbx() const{ return **this; } 
  };
  
}


#endif
