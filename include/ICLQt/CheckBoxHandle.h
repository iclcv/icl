/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/CheckBoxHandle.h                         **
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

#ifndef ICL_CHECK_BOX_HANDLE_H
#define ICL_CHECK_BOX_HANDLE_H

#include <string>
#include <vector>
#include <ICLQt/GUIHandle.h>

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
