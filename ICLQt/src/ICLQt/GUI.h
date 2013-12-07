/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/GUI.h                                  **
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

#include <string>
#include <vector>
#include <ICLUtils/SmartPtr.h>
#include <ICLUtils/Function.h>
#include <ICLQt/DataStore.h>
#include <ICLQt/GUIComponents.h>
#include <QtGui/QLayout>
#include <QtGui/QWidget>
#include <QtGui/QApplication>

/** \cond */
class QLayout;
/** \endcond */

namespace icl{
  namespace qt{
  
    /** \cond */
    class GUIWidget;
    class ProxyLayout;
    class GUIDefinition;
    /** \endcond */
  
    /// Main Class of ICL's GUI creation framework
    /** Please refer to the ICL manual for details */
    class ICL_QT_API GUI{
      private:
      /// stream operator to add new widgets
      /** if the given definition is "" or "dummy", this operator does nothing */
      virtual GUI &operator<<(const std::string &definition);
  
      protected:
      /// default constructor 
      GUI(const std::string &definition, QWidget *parent);
  
  
      public:
      /// registered widget type creator function
      typedef utils::Function<GUIWidget*,const GUIDefinition&> CreatorFunction;
      
      /// registers a new widget type
      /** The registered widget can then be added using a corresponding extension
          of the GUIComponent class */
      static void register_widget_type(const std::string &tag, CreatorFunction f);
      
      /// cell width (all sizes are given in this unit)
      static const int CELLW = 20;
      /// cell height (all sizes are given in this unit)
      static const int CELLH = 20;
      
      /// Default constructor, creates a vbox GUI Component
      GUI(QWidget *parent=0);
      
      /// creates a GUI from a givne GUIComponent
      GUI(const GUIComponent &component, QWidget *parent=0);
      
      /// copy constructor
      GUI(const GUI &gui,QWidget *parent=0);
  
      /// gui-copy operator
      GUI &operator=(const GUI &other);
  
      /// Destructor
      virtual ~GUI();
  
      /// adds a new GUI component 
      virtual GUI &operator<<(const GUIComponent &component);
      
      /// stream operator to add new other GUIs
      /** if the given GUI is a dummy GUI, this operator does nothing */
      virtual GUI &operator<<(const GUI &g);
      
      /// wraps the data-stores allocValue function
      template<class T>
      inline T &allocValue(const std::string &id, const T&val=T()){
        return m_oDataStore.allocValue<T>(id,val);
      }
  
      /// wraps the datastores release function
      template<class T>
      inline void release(const std::string &id){
        m_oDataStore.release<T>(id);
      }
      
      template<class T> 
      T &get(const std::string &id, bool typeCheck=true){
        return m_oDataStore.getValue<T>(id,typeCheck);
      }
  
      /// returns a Data instance from the datastore
      DataStore::Data operator[](const std::string &key){
        return m_oDataStore.operator[](key);
      }
      
      /// collects data from different components at once
      template<class T>
      std::vector<T> collect(const std::vector<std::string> &keys){
        return m_oDataStore.collect<T>(keys);
      }
      
      /// returns whether this gui is actually visible
      virtual bool isVisible() const;
      
      /// internally creates everything
      virtual void create();
  
      /// internally creates everything (and makes the gui visible)
      virtual void show();
      
      /// make this gui invisible (nothing more)
      virtual void hide();
      
      /// if widget is visible, this hides the widget, otherwise the widget is shown
      virtual void switchVisibility();
      
      /// returns the root widget of the gui (only avialable after create() or show())
      inline GUIWidget *getRootWidget(){
        return m_poWidget;
      }
      
      /// internally locks the datastore
      inline void lockData() {
        m_oDataStore.lock();
      }
      /// internally unlocks the data store
      inline void unlockData() {
        m_oDataStore.unlock();
      }
      /// waits for the gui to be created completely
      void waitForCreation();
  
      /// returns the GUI internal dataStore
      const DataStore &getDataStore() const { return m_oDataStore; }
      
      /// simple callback, that can be registered at GUI components
      /** Simple callback methods don't get any information about the source */
#ifdef ICL_SYSTEM_WINDOWS
      typedef utils::Function<void*> Callback;
#else
      typedef utils::Function<void> Callback;
#endif
      
      /// complex callback type that can be registered at GUI components
      /** Complex callback methods get the GUI components handle name as
          parameters. By these means, single callbacks can be registered
          to several components and still be able to handle events differently */
#ifdef ICL_SYSTEM_WINDOWS
      typedef utils::Function<void*,const std::string&> ComplexCallback;
#else
      typedef utils::Function<void, const std::string&> ComplexCallback;
#endif
      
      
      /// registers a callback function on each component 
      /** @param cb callback to execute 
          @param handleNamesList 'listDelim'-separated list of handle names 
          @param listDelim delimiter for the handle list
          ownership is passed to the childrens; deletion is performed by
          the smart pointers that are used...
      */
      void registerCallback(const Callback &cb, const std::string &handleNamesList, char listDelim=',');
      
      /// registeres a complex callback at a given GUI component
      /** complex callbacks are called with the actual GUI components handle name as parameter */
      void registerCallback(const ComplexCallback &cb, const std::string &handleNamesList, char listDelim=',');
  
      /// removes all callbacks from components
      void removeCallbacks(const std::string &handleNamesList, char listDelim=',');
  
      /// returns whether this GUI is a dummy GUI
      /** Please note: dummy GUIs
          * cannot be created
          * are not added by the stream operator
      */
      inline bool isDummy() const { return m_sDefinition == "" || m_sDefinition == "dummy"; }
      
      /// returns whether this GUI has been created or not
      bool hasBeenCreated() const;
  
      protected:
      /// can be overwritten in subclasses (such as ContainerGUIComponent)
      virtual std::string createDefinition() const { return m_sDefinition; }
  
      private:
  
      void create(QLayout *parentLayout,ProxyLayout *proxy, QWidget *parentWidget, DataStore *ds);
  
      /// own definition string
      std::string m_sDefinition;
      std::vector<GUI*> m_children;
      GUIWidget *m_poWidget;
      DataStore m_oDataStore;
      bool m_bCreated;
      QWidget *m_poParent;
    };
  } // namespace qt
}

