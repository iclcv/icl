/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ButtonGroupHandle.h                    **
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
#include <vector>
#include <string>

/** \cond */
class QRadioButton;
/** \endcond */

namespace icl{
  namespace qt{
  
    /// type definition for the ButtonGroup handle \ingroup HANDLES
    typedef std::vector<QRadioButton*> RadioButtonVec;
  
    /// Class for handling button goups \ingroup HANDLE
    class ButtonGroupHandle : public GUIHandle<RadioButtonVec> {
      public:
      /// Create an empty handle
      ICL_QT_API ButtonGroupHandle(){}
  
      /// Craete a valid handle
      ICL_QT_API ButtonGroupHandle(RadioButtonVec *buttons, GUIWidget *w) : GUIHandle<RadioButtonVec>(buttons, w){ }
      
      /// select a button with given index
      ICL_QT_API void select(int id);
      
      /// get the selected index
      ICL_QT_API int getSelected() const;
  
      /// get the text of the currently selected button
      ICL_QT_API std::string getSelectedText() const;
      
      /// returns the text of a button with given index
      ICL_QT_API std::string getText(int id) const;
      
      /// sets the text of a button with index ot a given text
      ICL_QT_API void setText(int id, const std::string &text);
      
      /// disables all radio buttons
      ICL_QT_API void disable();
      
      /// enables all radio buttons
      ICL_QT_API void enable();
      
      /// disables button at index
      ICL_QT_API void disable(int index);
      
      /// enables button at index
      ICL_QT_API void enable(int index);
      
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
    
  } // namespace qt
}
