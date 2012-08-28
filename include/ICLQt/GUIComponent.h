/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/GUIComponent.h                           **
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

#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Range.h>

namespace icl{
  namespace qt{
  
    /// The GUIComponent class servers as a generic interface for GUI definitions
    /** <b>Please refer to the ICL-manual for an introduction to the GUI toolkit</b>\n
     */
    class GUIComponent{
    
      /// friend container class
      friend class ContainerGUIComponent;
      
      /// friend GUI class
      friend class GUI;
  
      public:
      /// Actual options (set using the .xxx methods)
      struct Options {
      Options():margin(-1),spacing(-1), hide(false){}
        std::string handle;  //!< the component handle
        std::string out;     //!< the component output (only GUIComponentWithOutput subclasses)
        std::string in;      //!< not used!
        std::string label;   //!< label (results in a titeld border
        std::string tooltip; //!< component tooltip (not for containers) 
        int margin;          //!< layout margin (only for containers)
        int spacing;         //!< layout spacing (onyl for containers)
        utils::Size minSize;        //!< minimum size constraint of the component (in units of 20px)
        utils::Size maxSize;        //!< maximum size constraint of the component (in units of 20px)
        utils::Size size;           //!< intial size of the component (in units of 20px)
        bool hide;           //!< if true, the component is not created at all
      };
      protected:
      
      /// all component options (mutable for C++-reasons)
      mutable Options m_options;
      
      /// utility method to concatenate 3 values
      template<class A, class B, class C> 
      static std::string form_args_3(const A &a, const B &b, const C &c){
        std::ostringstream str;
        str << a << ',' << b << ',' << c;
        return str.str();
      }
      
      /// utility method to concatenate 4 values
      template<class A, class B, class C, class D> 
      static std::string form_args_4(const A &a, const B &b, const C &c, const D &d){
        std::ostringstream str;
        str << a << ',' << b << ',' << c << ',' << d;
        return str.str();
      }
      
      /// component type
      std::string m_type;
      
      /// component parameters
      std::string m_params;
      
      /// creates a component with given type and optionally given parameters
      /** the params parameter is a comma-separated list of single entries */
      GUIComponent(const std::string &type, const std::string &params=""):
      m_type(type),m_params(params){}
      public:
        
      /// sets the component handle
      const GUIComponent &handle(const std::string &handle) const{
        m_options.handle = handle; return *this;
      }
  
      /// sets the component label
      const GUIComponent &label(const std::string &label) const{
        m_options.label = label; return *this;
      }
  
      /// sets the component tooltip
      const GUIComponent &tooltip(const std::string &tooltip) const{
        m_options.tooltip = tooltip; return *this;
      }
  
      /// sets the component initial size
      const GUIComponent &size(const utils::Size &size) const {
        m_options.size = size; return *this;
      }
  
      /// sets the component initial size
      const GUIComponent &size(int w, int h) const {
        return size(utils::Size(w,h));
      }
  
      /// sets the component minimum size constraint
      const GUIComponent &minSize(const utils::Size &minSize) const {
        m_options.minSize = minSize; return *this;
      }
        
      /// sets the component minimum size constraint
      const GUIComponent &minSize(int w, int h) const {
        return minSize(utils::Size(w,h));
      }
      
      /// sets the component maximum size constraint
      const GUIComponent &maxSize(const utils::Size &maxSize) const {
        m_options.maxSize = maxSize; return *this;
      }
  
      /// sets the component maximum size constraint
      const GUIComponent &maxSize(int w, int h) const {
        return maxSize(utils::Size(w,h));
      }
  
      /// hides the component if the given flag is true
      /** this can be used to circumvent C++-language issues when creating
          GUI components optionally, e.g.
          \code
          bool flag = ....;
          GUI gui;
          gui << (flag ? Image() : Dummy()).handle("image"); // does not work
          
          gui << Image().hideIf(!flag).handle("image");    // works
          \endcode
          
      */
      const GUIComponent &hideIf(bool flag) const{
        if(flag) m_options.hide = true; return *this;
      }
  
      /// sets the component handle
      GUIComponent &handle(std::string &handle) {
        m_options.handle = handle; return *this;
      }
      
      /// sets the component label
      GUIComponent &label(std::string &label) {
        m_options.label = label; return *this;
      }
      
      /// sets the component tooltip
      GUIComponent &tooltip(std::string &tooltip) {
        m_options.tooltip = tooltip; return *this;
      }
      
      /// sets the component initial size
      GUIComponent &size(utils::Size &size)  {
        m_options.size = size; return *this;
      }
      
      /// sets the component initial size
      GUIComponent &size(int w, int h)  {
        m_options.size = utils::Size(w,h); return *this;
      }
      
      /// sets the component minimum size constraint
      GUIComponent &minSize(utils::Size &minSize)  {
        m_options.minSize = minSize; return *this;
      }
  
      /// sets the component minimum size constraint
      GUIComponent &minSize(int w, int h)  {
        m_options.minSize = utils::Size(w,h); return *this;
      }
      
      /// sets the component maximum size constraint
      GUIComponent &maxSize(utils::Size &maxSize)  {
        m_options.maxSize = maxSize; return *this;
      }
      
      /// sets the component maximum size constraint
      GUIComponent &maxSize(int w, int h)  {
        m_options.maxSize = utils::Size(w,h); return *this;
      }
  
      /// hides the component if the given flag is true
      /** \copydoc icl::GUIComponent::hideIf(bool)const */
      GUIComponent &hideIf(bool flag)  {
        if(flag) m_options.hide = true; return *this;
      }
      
      /// creates a string representation of the component
      std::string toString() const {
        if(m_options.hide) return "";
        std::ostringstream str;
        str << m_type;
        if(m_params.length()){
          str << '(' << m_params << ')';
        }
        if(m_options.handle.length() ||
           m_options.label.length() ||
           m_options.out.length() ||
           m_options.in.length() ||
           m_options.tooltip.length() ||
           m_options.margin > 0 ||
           m_options.spacing > 0 ||
           m_options.minSize != utils::Size::null ||
           m_options.maxSize != utils::Size::null ||
           m_options.size != utils::Size::null ){
          str << '[';
          if(m_options.handle.length()) str << "@handle=" << m_options.handle;
          if(m_options.out.length()) str << "@out=" << m_options.out;
          if(m_options.in.length()) str << "@in=" << m_options.in;
          if(m_options.label.length()) str << "@label=" << m_options.label;
          if(m_options.tooltip.length()) str << "@tooltip=" << m_options.tooltip;
          if(m_options.margin > 0) str << "@margin=" << m_options.margin;
          if(m_options.spacing > 0) str << "@spacing=" << m_options.spacing;
          if(m_options.minSize != utils::Size::null ) str << "@minsize=" << m_options.minSize;
          if(m_options.maxSize != utils::Size::null ) str << "@maxsize=" << m_options.maxSize;
          if(m_options.size != utils::Size::null ) str << "@size=" << m_options.size;
          str << "]";
        }
        return str.str();
      }
    };
  } // namespace qt
}

