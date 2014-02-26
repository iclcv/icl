/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ComboHandle.h                          **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIHandle.h>
#include <string>

/** \cond */
class QComboBox;
/** \endcond */

namespace icl{
  namespace qt{
    
    /// Handle class for combo components \ingroup HANDLES
    class ComboHandle : public GUIHandle<QComboBox>{
      public:
      /// create an empty handle
      ComboHandle(){}
  
      /// create a new ComboHandle wrapping a given QComboBox
      ComboHandle(QComboBox *cb, GUIWidget *w):GUIHandle<QComboBox>(cb,w){}
      
      /// add an item
      ICLQt_API void add(const std::string &item);
  
      /// remove an item
      ICLQt_API void remove(const std::string &item);
      
      /// remove item at given index
      ICLQt_API void remove(int idx);
      
      /// void remove all items
      ICLQt_API void clear();
      
      /// returns the item at given index
      ICLQt_API std::string getItem(int idx) const;
  
      /// returns the index of a given item
      ICLQt_API int getIndex(const std::string &item) const;
      
      /// returns the currently selected index
      ICLQt_API int getSelectedIndex() const;
      
      /// returns the currently selected item
      ICLQt_API std::string getSelectedItem() const;
      
      /// returns the count of elements
      ICLQt_API int getItemCount() const;
      
      /// sets the current index
      ICLQt_API void setSelectedIndex(int idx);
      
      /// sets the current item
      ICLQt_API void setSelectedItem(const std::string &item);
  
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
    
  } // namespace qt
}


