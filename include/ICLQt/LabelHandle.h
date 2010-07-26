/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/LabelHandle.h                            **
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

#ifndef ICL_GUI_LABEL_H
#define ICL_GUI_LABEL_H

#include <string>
#include <QtCore/QString>
#include <QtGui/QWidget>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/CompabilityLabel.h>


namespace icl{

  /// Class for GUI-Label handling \ingroup HANDLES
  /** The gui label is created inside the GUI class, it can be used
      to make GUI-"label" components show anther text 
      @see GUI */
  class LabelHandle : public GUIHandle<CompabilityLabel>{
    public:
    
    /// Create an empty handle
    LabelHandle(){}
    
    /// Create a new LabelHandle
    LabelHandle(CompabilityLabel *l, GUIWidget *w):GUIHandle<CompabilityLabel>(l,w){}
    
    ///  assign a std::string (makes the underlying label show that string)
    void operator=(const std::string &text);

    ///  assign a QString (makes the underlying label show that string)
    void operator=(const QString &text);

    ///  assign a const char* (makes the underlying label show that string)
    void operator=(const char *text);

    ///  assign an int (makes the underlying label show that integer)
    void operator=(int num);

    ///  assign a double (makes the underlying label show that double)
    void operator=(double num);
    
    /// appends text to the current text
    void operator+=(const std::string &text);
    
    private:
    /// utitlity function
    CompabilityLabel *lab() { return **this; }

    /// utitlity function
    const CompabilityLabel *lab() const { return **this; }
  };

 
  
 
}                               

#endif
