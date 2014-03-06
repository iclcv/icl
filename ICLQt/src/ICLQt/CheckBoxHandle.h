/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/CheckBoxHandle.h                       **
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

#include <string>
#include <vector>
#include <ICLQt/GUIHandle.h>

/**\cond */
class QCheckBox;
/**\endcond */


namespace icl{
  namespace qt{
    
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

      /// defines the check-state
      inline void doCheck(bool on, bool execCallbacks=true){
        if(on) check(execCallbacks);
        else uncheck(execCallbacks);
      }
  
      // returns whether this the checkbox is currently checked
      bool isChecked() const;
      
      private:
  
      /// internal state reference variable
      bool *m_stateRef;
  
    };  
  } // namespace qt
}

