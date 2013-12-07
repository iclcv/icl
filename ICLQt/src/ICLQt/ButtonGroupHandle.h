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

#include <vector>
#include <string>
#include <ICLQt/GUIHandle.h>
/** \cond */
class QRadioButton;
/** \endcond */

namespace icl{
  namespace qt{
  
    /// type definition for the ButtonGroup handle \ingroup HANDLES
    typedef std::vector<QRadioButton*> RadioButtonVec;
  
    /// Class for handling button goups \ingroup HANDLE
    class ICL_QT_API ButtonGroupHandle : public GUIHandle<RadioButtonVec> {
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
      
      /// disables all radio buttons
      void disable();
      
      /// enables all radio buttons
      void enable();
      
      /// disables button at index
      void disable(int index);
      
      /// enables button at index
      void enable(int index);
      
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
