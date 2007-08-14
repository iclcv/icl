#ifndef ICL_GUI_H
#define ICL_GUI_H

#include <string>
#include <vector>
#include <iclSmartPtr.h>
#include <iclGUIDataStore.h>
#include <iclGUIEvent.h>
#include <iclGUILabel.h>
#include <iclWidget.h>
#include <iclDrawWidget.h>


/** \cond */
class QLayout;
/** \endcond */


/// The icl namespace
namespace icl{

  /** \cond */
  class GUIWidget;
  /** \endcond */


  /// GUI class 
  /** TODO: Document this !! */
  class GUI{
    public:
    static const int CELLW = 20;
    static const int CELLH = 20;
    
    /// default constructor 
    GUI(const std::string &definition);
    GUI(const GUI &gui);
    /// Destructor
    virtual ~GUI(){}
    
    /// stream operator to add new widgets
    virtual GUI &operator<<(const std::string &definition);
    
    /// stream operator to add new other GUIs
    virtual GUI &operator<<(const GUI &g);
    
    /// wraps the datastores allocValue function
    template<class T>
    inline T &allocValue(const std::string &id, const T&val=T()){
      return m_oDataStore.allocValue<T>(id,val);
    }
    /// wraps the datastores allocArray function
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
    T &getValue(const std::string &id){
      return m_oDataStore.getValue<T>(id);
    }

    /// wraps the datastores getArray function
    template<class T> 
    inline T* getArray(const std::string &id, int *lenDst=0){
      return m_oDataStore.getArray<T>(id,lenDst);
    }
    
    /// internally creates everything
    virtual void show();
    
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
    
    private:
    void create(QLayout *parentLayout,QWidget *parentWidget, GUIDataStore *ds);

    /// own definition string
    std::string m_sDefinition;
    std::vector<GUI*> m_vecChilds;
    GUIWidget *m_poWidget;
    GUIDataStore m_oDataStore;
    bool m_bCreated;
  };  
}

#endif
