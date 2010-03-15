/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLQt/GUI.h                                    **
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
*********************************************************************/

#ifndef ICL_GUI_H
#define ICL_GUI_H

#include <string>
#include <vector>
#include <ICLUtils/SmartPtr.h>
#include <ICLQt/DataStore.h>

#include <QLayout>
#include <QWidget>
#include <QApplication>

/** \cond */
class QLayout;
/** \endcond */

namespace icl{

  /** \cond */
  class GUIWidget;
  class ProxyLayout;
  /** \endcond */

  // documentation in GUIDocumentation.h !
  class GUI{
    public:
    /// cell width (all sizes are given in this unit)
    static const int CELLW = 20;
    /// cell height (all sizes are given in this unit)
    static const int CELLH = 20;
    
    /// default constructor 
    GUI(const std::string &definition="vbox", QWidget *parent=0);
    
    /// copy constructor
    GUI(const GUI &gui,QWidget *parent=0);

    /// Destructor
    virtual ~GUI();
    
    /// stream operator to add new widgets
    virtual GUI &operator<<(const std::string &definition);
    
    /// stream operator to add new other GUIs
    virtual GUI &operator<<(const GUI &g);
    
    /// wraps the data-stores allocValue function
    template<class T>
    inline T &allocValue(const std::string &id, const T&val=T()){
      return m_oDataStore.allocValue<T>(id,val);
    }
    /// wraps the data-stores allocArray function
    template<class T>
    inline T *allocArray(const std::string &id,unsigned int n){
      return m_oDataStore.allocArray<T>(id,n);
    }
    /// wraps the datastores release function
    template<class T>
    inline void release(const std::string &id){
      m_oDataStore.release<T>(id);
    }
    
    /// wraps the datastores getValue function
    template<class T> 
    T &getValue(const std::string &id, bool typeCheck=true){
      return m_oDataStore.getValue<T>(id,typeCheck);
    }

    /// wraps the datastores getArray function
    template<class T> 
    inline T* getArray(const std::string &id, int *lenDst=0){
      return m_oDataStore.getArray<T>(id,lenDst);
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
    
    
    /// Callback helper class: Default implementation calls a callback function 
    struct Callback{
      /// typedef to wrapped function (only for default implementation)
      typedef void (*callback_function)(void);

      /// typedef to wrapped function (only used if exec(const std::string&) is not overloaded!)
      typedef void (*complex_callback_function)(const std::string&);
      
    private:
      /// internally used default callback function
      callback_function m_func;
      
      /// internally used complex  callback function
      /** complex callback functions do always get the event-sources handle
          name as argument*/
      complex_callback_function m_cfunc;
      
      protected:
      /// create a new callback object
      Callback():m_func(0),m_cfunc(0){}
      
      public:
      /// Default implementations constructor with given callback function
      Callback(callback_function func):m_func(func),m_cfunc(0){}

      /// Default implementations constructor with given complex callback function
      Callback(complex_callback_function cfunc):m_func(0),m_cfunc(cfunc){}
      
      /// vitual execution function
      virtual void exec(){
        if(m_func) m_func();
      }
      
      /// virtual complex execution function
      /** the complex function is called with given component's handle name, 
          so it can be used to handle events of different components */
      virtual void exec(const std::string &handle){
        if(m_cfunc) m_cfunc(handle);
      }

    };
    typedef SmartPtr<Callback> CallbackPtr;
    
    /// registers a callback function on each component 
    /** @param cb callback to execute 
        @param handleNamesList comma-separated list of handle names 
        ownership is passed to the childrens; deletion is performed by
        the smart pointers that are used...
    */
    void registerCallback(CallbackPtr cb, const std::string &handleNamesList);

    /// convenience wrapper for registration of callback functions
    /** internally this function creates and appropriate CallbackPtr 
        in order to pass the given callback_function f 
        to registerCallback(CallbackPtr,const std::string&)*/
    void registerCallback(GUI::Callback::callback_function f, const std::string &handleNamesList){
      registerCallback(new Callback(f),handleNamesList);
    }

    /// convenience wrapper for registration of complex callback functions
    /** internally this function creates and appropriate CallbackPtr 
        in order to pass the given complex_callback_function f 
        to registerCallback(CallbackPtr,const std::string&)*/
    void registerCallback(GUI::Callback::complex_callback_function f, const std::string &handleNamesList){
      registerCallback(new Callback(f),handleNamesList);
    }


    /// removes all callbacks from components
    void removeCallbacks(const std::string &handleNamesList);

    private:
    void create(QLayout *parentLayout,ProxyLayout *proxy, QWidget *parentWidget, DataStore *ds);

    /// own definition string
    std::string m_sDefinition;
    std::vector<GUI*> m_vecChilds;
    GUIWidget *m_poWidget;
    DataStore m_oDataStore;
    bool m_bCreated;
    QWidget *m_poParent;
  };  
}

#endif
