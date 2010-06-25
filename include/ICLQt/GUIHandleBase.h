/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/GUIHandleBase.h                          **
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

#ifndef ICL_GUI_HANDLE_BASE_H
#define ICL_GUI_HANDLE_BASE_H

#include <ICLQt/GUIWidget.h>

namespace icl{
  
  /// Base class for GUIHandles providing functions to register callbacks \ingroup UNCOMMON
  class GUIHandleBase{

    protected:
    /// create a new GUIHandleBase
    GUIHandleBase(GUIWidget *w=0):m_poGUIWidget(w){}

    public:

    /** \cond */
    friend class icl::GUI;
    friend class icl::DataStore;
    friend class icl::MultiTypeMap;
    /** \endcond */
    
    /// returns parent GUIWidget pointer
    GUIWidget *getGUIWidget(){
      return m_poGUIWidget;
    }
    
    /// registers a callback on this gui widget
    /** This function can be re-implemented to bypass event propagation. By default,
        events are passed to the parent GUIWidget which, then again passes the events
        implementation-dependently to the actual widget.

        @param cb callback functor to call
        @param events comma separated list of event types to register on.
               This list is handled internally by each special GUIHandlerBase implementation
               currently only the ImageHandle DrawHandle and DrawHandle3D re-implement this 
               functionality. At default, events is not regarded at all 
    */
    virtual void registerCallback(GUI::CallbackPtr cb, const std::string &events="all"){
      (void)events;
      if(m_poGUIWidget){
        m_poGUIWidget->registerCallback(cb);
      }else{
        ERROR_LOG("unable to register a callback function on a null handle");
      }
    }
    
    /// removes all callbacks from parent GUIWidget component
    virtual void removeCallbacks(){
      if(m_poGUIWidget){
        m_poGUIWidget->removeCallbacks();
      }else{
        ERROR_LOG("unable to remove callbacks from a null handle");
      }
    }
    
    /// envokes all registered callbacks
    virtual void cb(){
      if(m_poGUIWidget){
        m_poGUIWidget->cb();
      }else{
        ERROR_LOG("unable to call callbacks on a null handle");
      }
    }
    
    private:
    GUIWidget *m_poGUIWidget;
  };
}

#endif
