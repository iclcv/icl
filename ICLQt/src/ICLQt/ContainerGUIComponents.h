/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ContainerGUIComponents.h               **
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

#include <ICLQt/ContainerGUIComponent.h>

namespace icl{
  namespace qt{
    
    /// Horizonal Box container component
    struct ICL_QT_API HBox : public ContainerGUIComponent{
      /// create HBox with optionally given parent
      HBox(QWidget *parent=0):ContainerGUIComponent("hbox","",parent){}
    };
    
    /// Vertical Box container component
    struct ICL_QT_API VBox : public ContainerGUIComponent{
      /// create VBox with optionally given parent
      VBox(QWidget *parent=0):ContainerGUIComponent("vbox","",parent){}
    };
      
    /// Horizontal scroll area
    struct ICL_QT_API HScroll : public ContainerGUIComponent{
      /// create HScroll with optionally given parent
      HScroll(QWidget *parent=0):ContainerGUIComponent("hscroll","",parent){}
    };
      
    /// Vertical scroll area
    struct ICL_QT_API VScroll : public ContainerGUIComponent{
      /// create VScroll with optionally given parent
      VScroll(QWidget *parent=0):ContainerGUIComponent("vscroll","",parent){}
    };
      
    /// Horizontal split-component
    struct ICL_QT_API HSplit : public ContainerGUIComponent{
      /// create HSplit with optionally given parent
      HSplit(QWidget *parent=0):ContainerGUIComponent("hsplit","",parent){}
    };
      
    /// Vertical split-component
    struct ICL_QT_API VSplit : public ContainerGUIComponent{
      /// create HSplit with optionally given parent
      VSplit(QWidget *parent=0):ContainerGUIComponent("vsplit","",parent){}
    };
      
    /// Tab-compnent
    struct ICL_QT_API Tab : public ContainerGUIComponent{
      /// create Tab with given list of tab-labels optionally given parent
      /** If more components are included, than tab-labels were given,
          a warning is shown and a dummy tab label is created */
      Tab(const std::string &commaSepTitles,QWidget *parent=0):
      ContainerGUIComponent("tab",commaSepTitles, parent){}
    };

    /// internally used component
    struct ICL_QT_API Border : public ContainerGUIComponent{
      friend class ::icl::qt::GUI;
      private:
      /// create HBox with optionally given parent
      Border(const std::string &label, QWidget *parent=0):
      ContainerGUIComponent("border",label,parent){}
    };
  } // namespace qt
}

