/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/LabelHandle.h                          **
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
#include <QtCore/QString>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/CompabilityLabel.h>
#include <QWidget>
#include <string>


namespace icl{
  namespace qt{
  
    /// Class for GUI-Label handling \ingroup HANDLES
    /** The gui label is created inside the GUI class, it can be used
        to make GUI-"label" components show anther text 
        @see GUI */
    class ICLQt_API LabelHandle : public GUIHandle<CompabilityLabel>{
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

  } // namespace qt
}                               

