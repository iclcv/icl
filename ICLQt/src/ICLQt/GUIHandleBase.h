/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/GUIHandleBase.h                        **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLQt/GUIWidget.h>

namespace icl{
  namespace qt{
    
    /// Base class for GUIHandles providing functions to register callbacks \ingroup UNCOMMON
    class GUIHandleBase{
  
      protected:
      /// create a new GUIHandleBase
      GUIHandleBase(GUIWidget *w=0):m_poGUIWidget(w){}
  
      public:
      
      /// virtual destructor
      virtual ~GUIHandleBase(){}
  
      /** \cond */
      friend class icl::qt::GUI;
      friend class icl::qt::DataStore;
      friend class icl::utils::MultiTypeMap;
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
                 functionality. By default, the paremter 'events' is not regarded at all 
      */
      virtual void registerCallback(const GUI::Callback &cb, const std::string &events="all"){
        (void)events;
        if(m_poGUIWidget){
          m_poGUIWidget->registerCallback(cb);
        }else{
          ERROR_LOG("unable to register a callback function on a null handle");
        }
      }
  
      /// registers a complex callback on this gui component
      virtual void registerCallback(const GUI::ComplexCallback &cb, const std::string &events="all"){
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
  } // namespace qt
}

