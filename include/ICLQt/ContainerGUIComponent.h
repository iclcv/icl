#ifndef ICL_CONTAINER_GUI_COPONENT_H
#define ICL_CONTAINER_GUI_COPONENT_H

#include <ICLQt/GUI.h>

namespace icl{
  struct ContainerGUIComponent : public GUI{
    protected:
    /// we use these options to create the 
    /** Please note that inheritance is not possible because it leads to
        an abiguous overload for the GUI<<-operator */
    mutable GUIComponent component;
      
    public:

    ContainerGUIComponent(const std::string &definitionString, QWidget *parent):
    GUI(definitionString,parent), component(definitionString){}
      
    const ContainerGUIComponent &handle(const std::string &handle) const{
      component.handle(handle); return *this;
    }
      
    const ContainerGUIComponent &label(const std::string &label) const{
      component.label(label); return *this;
    }
      
    const ContainerGUIComponent &tooltip(const std::string &tooltip) const{
      component.tooltip(tooltip); return *this;
    }
      
    const ContainerGUIComponent &size(const Size &size) const {
      component.size(size); return *this;
    }
      
    const ContainerGUIComponent &size(int w, int h) const {
      return size(Size(w,h));
    }
      
    const ContainerGUIComponent &minSize(const Size &minSize) const {
      component.minSize(minSize); return *this;
    }
      
    const ContainerGUIComponent &minSize(int w, int h) const {
      return minSize(Size(w,h));
    }
      
    const ContainerGUIComponent &maxSize(const Size &maxSize) const {
      component.maxSize(maxSize); return *this;
    }
      
    const ContainerGUIComponent &maxSize(int w, int h) const {
      return maxSize(Size(w,h));
    }
      
    const ContainerGUIComponent &margin(int margin) const{
      component.m_options.margin = margin; return *this;
    }
      
    const ContainerGUIComponent &spacing(int spacing) const{
      component.m_options.spacing = spacing; return *this;
    }

    protected:
    virtual std::string createDefinition() const { return component.toString();  }
  };
    
  struct HBox : public ContainerGUIComponent{
    HBox(QWidget *parent=0):ContainerGUIComponent("hbox",parent){}
  };
    
  struct VBox : public ContainerGUIComponent{
    VBox(QWidget *parent=0):ContainerGUIComponent("vbox",parent){}
  };
    
  struct HScroll : public ContainerGUIComponent{
    HScroll(QWidget *parent=0):ContainerGUIComponent("hscroll",parent){}
  };
    
  struct VScroll : public ContainerGUIComponent{
    VScroll(QWidget *parent=0):ContainerGUIComponent("vscroll",parent){}
  };
    
  struct HSplit : public ContainerGUIComponent{
    HSplit(QWidget *parent=0):ContainerGUIComponent("hsplit",parent){}
  };
    
  struct VSplit : public ContainerGUIComponent{
    VSplit(QWidget *parent=0):ContainerGUIComponent("vsplit",parent){}
  };
    
  struct Tab : public ContainerGUIComponent{
    Tab(const std::string &commaSepTitles,QWidget *parent=0):
    ContainerGUIComponent("tab"+'('+commaSepTitles+')', parent){}
  };

} // namespace icl

#endif
