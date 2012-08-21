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

    ContainerGUIComponent(const std::string &type, const std::string &params, QWidget *parent):
    GUI(type+'('+params+')',parent), component(type, params){}
      
    GUI &operator<<(const GUIComponent &component) const{
      return const_cast<GUI*>(static_cast<const GUI*>(this))->operator<<(component);
    }
    
    GUI &operator<<(const GUI &g) const{
      return const_cast<GUI*>(static_cast<const GUI*>(this))->operator<<(g);
    }

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

    ContainerGUIComponent &handle( std::string &handle) {
      component.handle(handle); return *this;
    }
    
    ContainerGUIComponent &label( std::string &label) {
      component.label(label); return *this;
    }
    
    ContainerGUIComponent &tooltip( std::string &tooltip) {
      component.tooltip(tooltip); return *this;
    }
    
    ContainerGUIComponent &size( Size &size)  {
      component.size(size); return *this;
    }
    
    ContainerGUIComponent &size(int w, int h)  {
      component.size(w,h); return *this;
    }
    
    ContainerGUIComponent &minSize( Size &minSize)  {
      component.minSize(minSize); return *this;
    }
    
    ContainerGUIComponent &minSize(int w, int h)  {
      component.minSize(w,h); return *this;
    }
    
    ContainerGUIComponent &maxSize( Size &maxSize)  {
      component.maxSize(maxSize); return *this;
    }
    
    ContainerGUIComponent &maxSize(int w, int h)  {
      component.maxSize(w,h); return *this;
    }
    
    ContainerGUIComponent &margin(int margin) {
      component.m_options.margin = margin; return *this;
    }
    
    ContainerGUIComponent &spacing(int spacing) {
      component.m_options.spacing = spacing; return *this;
    }

    protected:
    virtual std::string createDefinition() const { return component.toString();  }    
  };
    
  struct HBox : public ContainerGUIComponent{
    HBox(QWidget *parent=0):ContainerGUIComponent("hbox","",parent){}
  };
    
  struct VBox : public ContainerGUIComponent{
    VBox(QWidget *parent=0):ContainerGUIComponent("vbox","",parent){}
  };
    
  struct HScroll : public ContainerGUIComponent{
    HScroll(QWidget *parent=0):ContainerGUIComponent("hscroll","",parent){}
  };
    
  struct VScroll : public ContainerGUIComponent{
    VScroll(QWidget *parent=0):ContainerGUIComponent("vscroll","",parent){}
  };
    
  struct HSplit : public ContainerGUIComponent{
    HSplit(QWidget *parent=0):ContainerGUIComponent("hsplit","",parent){}
  };
    
  struct VSplit : public ContainerGUIComponent{
    VSplit(QWidget *parent=0):ContainerGUIComponent("vsplit","",parent){}
  };
    
  struct Tab : public ContainerGUIComponent{
    Tab(const std::string &commaSepTitles,QWidget *parent=0):
    ContainerGUIComponent("tab",commaSepTitles, parent){}
  };
  
  /** \cond */
  struct Border : public ContainerGUIComponent{
    friend class GUI;
    private:
    Border(const std::string &label, QWidget *parent=0):
    ContainerGUIComponent("border",label,parent){}
  };
  /** \encond */

} // namespace icl

#endif
