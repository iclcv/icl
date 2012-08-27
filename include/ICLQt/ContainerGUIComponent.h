/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/ContainerGUIComponent.h                  **
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

#pragma once

#include <ICLQt/GUI.h>

namespace icl{
  namespace qt{
    /// Special GUI extension, that mimics the GUIComponent interface 
    /** The Container GUIComponent mimics the GUIComponent interface
        in order to provide a unified look and feel within a hierarchical
        GUI definition section. Internally a mutable GUIComponent is used
        for parameter accumulation
    */
    struct ContainerGUIComponent : public GUI{
      protected:
      /// we use these options to create the 
      /** Please note that inheritance is not possible because it leads to
          an abiguous overload for the GUI<<-operator */
      mutable GUIComponent component;
  
      /// protected constructor
      ContainerGUIComponent(const std::string &type, const std::string &params, QWidget *parent):
      GUI(type+'('+params+')',parent), component(type, params){}
        
      public:
      
      /// hierarchical stream operator to create complex GUIs
      GUI &operator<<(const GUIComponent &component) const{
        return const_cast<GUI*>(static_cast<const GUI*>(this))->operator<<(component);
      }
      
      /// hierarchical stream operator to create complex GUIs
      GUI &operator<<(const GUI &g) const{
        return const_cast<GUI*>(static_cast<const GUI*>(this))->operator<<(g);
      }
  
      /// sets the component's handle
      const ContainerGUIComponent &handle(const std::string &handle) const{
        component.handle(handle); return *this;
      }
        
      /// sets the component's label
      const ContainerGUIComponent &label(const std::string &label) const{
        component.label(label); return *this;
      }
        
      /// sets the component's initial size
      const ContainerGUIComponent &size(const Size &size) const {
        component.size(size); return *this;
      }
        
      /// sets the component's initial size
      const ContainerGUIComponent &size(int w, int h) const {
        return size(Size(w,h));
      }
  
      /// sets the component's minimum size constraint
      const ContainerGUIComponent &minSize(const Size &minSize) const {
        component.minSize(minSize); return *this;
      }
        
      /// sets the component's minimum size constraint
      const ContainerGUIComponent &minSize(int w, int h) const {
        return minSize(Size(w,h));
      }
        
      /// sets the component's maximum size constraint
      const ContainerGUIComponent &maxSize(const Size &maxSize) const {
        component.maxSize(maxSize); return *this;
      }
        
      /// sets the component's maximum size constraint
      const ContainerGUIComponent &maxSize(int w, int h) const {
        return maxSize(Size(w,h));
      }
        
      /// sets the component's layout margin
      const ContainerGUIComponent &margin(int margin) const{
        component.m_options.margin = margin; return *this;
      }
        
      /// sets the component's layout spacing
      const ContainerGUIComponent &spacing(int spacing) const{
        component.m_options.spacing = spacing; return *this;
      }
  
      /// sets the component's handle
      ContainerGUIComponent &handle(const std::string &handle){
        component.handle(handle); return *this;
      }
        
      /// sets the component's label
      ContainerGUIComponent &label(const std::string &label){
        component.label(label); return *this;
      }
        
      /// sets the component's initial size
      ContainerGUIComponent &size(const Size &size){
        component.size(size); return *this;
      }
        
      /// sets the component's initial size
      ContainerGUIComponent &size(int w, int h){
        return size(Size(w,h));
      }
  
      /// sets the component's minimum size constraint
      ContainerGUIComponent &minSize(const Size &minSize){
        component.minSize(minSize); return *this;
      }
        
      /// sets the component's minimum size constraint
      ContainerGUIComponent &minSize(int w, int h){
        return minSize(Size(w,h));
      }
        
      /// sets the component's maximum size constraint
      ContainerGUIComponent &maxSize(const Size &maxSize){
        component.maxSize(maxSize); return *this;
      }
        
      /// sets the component's maximum size constraint
      ContainerGUIComponent &maxSize(int w, int h){
        return maxSize(Size(w,h));
      }
        
      /// sets the component's layout margin
      ContainerGUIComponent &margin(int margin){
        component.m_options.margin = margin; return *this;
      }
        
      /// sets the component's layout spacing
      ContainerGUIComponent &spacing(int spacing){
        component.m_options.spacing = spacing; return *this;
      }
  
      protected:
      /// special reimplementation of the GUI::createDefinition method
      virtual std::string createDefinition() const { return component.toString();  }    
    };
      
    
  
  } // namespace qt
} // namespace icl


