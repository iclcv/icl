// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/ContainerGUIComponent.h>

namespace icl{
  namespace qt{

    /// Horizonal Box container component
    struct HBox : public ContainerGUIComponent{
      /// create HBox with optionally given parent
      HBox(QWidget *parent=0):ContainerGUIComponent("hbox","",parent){}
    };

    /// Vertical Box container component
    struct VBox : public ContainerGUIComponent{
      /// create VBox with optionally given parent
      VBox(QWidget *parent=0):ContainerGUIComponent("vbox","",parent){}
    };

    /// Horizontal scroll area
    struct HScroll : public ContainerGUIComponent{
      /// create HScroll with optionally given parent
      HScroll(QWidget *parent=0):ContainerGUIComponent("hscroll","",parent){}
    };

    /// Vertical scroll area
    struct VScroll : public ContainerGUIComponent{
      /// create VScroll with optionally given parent
      VScroll(QWidget *parent=0):ContainerGUIComponent("vscroll","",parent){}
    };

    /// Horizontal split-component
    struct HSplit : public ContainerGUIComponent{
      /// create HSplit with optionally given parent
      HSplit(QWidget *parent=0):ContainerGUIComponent("hsplit","",parent){}
    };

    /// Vertical split-component
    struct VSplit : public ContainerGUIComponent{
      /// create HSplit with optionally given parent
      VSplit(QWidget *parent=0):ContainerGUIComponent("vsplit","",parent){}
    };

    /// Tab-compnent
    struct Tab : public ContainerGUIComponent{
      /// create Tab with given list of tab-labels optionally given parent
      /** If more components are included, than tab-labels were given,
          a warning is shown and a dummy tab label is created */
      Tab(const std::string &commaSepTitles,QWidget *parent=0):
      ContainerGUIComponent("tab",commaSepTitles, parent){}
    };

    /// internally used component
    struct Border : public ContainerGUIComponent{
      friend class ::icl::qt::GUI;
      private:
      /// create HBox with optionally given parent
      Border(const std::string &label, QWidget *parent=0):
      ContainerGUIComponent("border",label,parent){}
    };
  } // namespace qt
}
