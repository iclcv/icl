/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/GUIComponentWithOutput.h                 **
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

#include <ICLQt/GUIComponent.h>

namespace icl{
  namespace qt{
  
    struct GUIComponentWithOutput : public GUIComponent{
      GUIComponentWithOutput(const std::string &type, const std::string &params):
      GUIComponent(type,params){}
  
      /// sets the component handle
      const GUIComponentWithOutput &handle(const std::string &handle) const{
        m_options.handle = handle; return *this;
      }
  
      /// sets the component label
      const GUIComponentWithOutput &label(const std::string &label) const{
        m_options.label = label; return *this;
      }
  
      /// sets the component tooltip
      const GUIComponentWithOutput &tooltip(const std::string &tooltip) const{
        m_options.tooltip = tooltip; return *this;
      }
  
      /// sets the component initial size
      const GUIComponentWithOutput &size(const utils::Size &size) const {
        m_options.size = size; return *this;
      }
  
      /// sets the component initial size
      const GUIComponentWithOutput &size(int w, int h) const {
        return size(utils::Size(w,h));
      }
  
      /// sets the component minimum size constraint
      const GUIComponentWithOutput &minSize(const utils::Size &minSize) const {
        m_options.minSize = minSize; return *this;
      }
        
      /// sets the component minimum size constraint
      const GUIComponentWithOutput &minSize(int w, int h) const {
        return minSize(utils::Size(w,h));
      }
      
      /// sets the component maximum size constraint
      const GUIComponentWithOutput &maxSize(const utils::Size &maxSize) const {
        m_options.maxSize = maxSize; return *this;
      }
  
      /// sets the component maximum size constraint
      const GUIComponentWithOutput &maxSize(int w, int h) const {
        return maxSize(utils::Size(w,h));
      }
  
      /// hides the component if the given flag is true
      /** \copydoc icl::GUIComponent::hideIf(bool)const */     
      const GUIComponentWithOutput &hideIf(bool flag) const{
        if(flag) m_options.hide = true; return *this;
      }
  
      /// sets the component output id
      const GUIComponentWithOutput &out(const std::string &name) const {
        m_options.out = name; return *this;
      }
  
      /// sets the component handle
      GUIComponentWithOutput &handle(std::string &handle) {
        m_options.handle = handle; return *this;
      }
      
      /// sets the component label
      GUIComponentWithOutput &label(std::string &label) {
        m_options.label = label; return *this;
      }
      
      /// sets the component tooltip
      GUIComponentWithOutput &tooltip(std::string &tooltip) {
        m_options.tooltip = tooltip; return *this;
      }
      
      /// sets the component initial size
      GUIComponentWithOutput &size(utils::Size &size)  {
        m_options.size = size; return *this;
      }
      
      /// sets the component initial size
      GUIComponentWithOutput &size(int w, int h)  {
        m_options.size = utils::Size(w,h); return *this;
      }
      
      /// sets the component minimum size constraint
      GUIComponentWithOutput &minSize(utils::Size &minSize)  {
        m_options.minSize = minSize; return *this;
      }
  
      /// sets the component minimum size constraint
      GUIComponentWithOutput &minSize(int w, int h)  {
        m_options.minSize = utils::Size(w,h); return *this;
      }
      
      /// sets the component maximum size constraint
      GUIComponentWithOutput &maxSize(utils::Size &maxSize)  {
        m_options.maxSize = maxSize; return *this;
      }
      
      /// sets the component maximum size constraint
      GUIComponentWithOutput &maxSize(int w, int h)  {
        m_options.maxSize = utils::Size(w,h); return *this;
      }
  
      /// hides the component if the given flag is true
      /** \copydoc icl::GUIComponent::hideIf(bool)const */
      GUIComponentWithOutput &hideIf(bool flag)  {
        if(flag) m_options.hide = true; return *this;
      }
  
      /// sets the component output id
      GUIComponentWithOutput &out(const std::string &name){
        m_options.out = name; return *this;
      }
  
    };
  
  } // namespace qt
}
