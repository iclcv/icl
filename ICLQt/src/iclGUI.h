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

#include <QApplication>

/** \cond */
class QLayout;
/** \endcond */


/// The icl namespace
namespace icl{

  /** \cond */
  class GUIWidget;
  /** \endcond */

  /// Simple but powerful GUI Toolkit for the ICL
  /** \section INTRO Introduction
      When working on computer-vision tasks, one whats to have some graphical user interface 
      to show the current images. This can be achieved by the use of the ICLQt's ICLWidget and 
      ICLDrawWidget classes. \n
      However in many cases this GUI components are not enough, e.g.:
      - a value shall be adjusted at rumtime (using a slider, a spinbox or a simple textfield)
      - a mode shall be swiched (using a radiobutton group or a combobox)
      - the application shall have a "pause"-button which stops the iteration thread temporarily
      - ...
      
      Actually all this capabilities can be implemented using the powerful Qt-Framework. But
      you will encounter some tricky problems then:
      - You must syncronize Qt's even-loop with the working thread
      - You have to handle user interaction using Qt's slots and signals
      - You have to create QObject classes using the Q_OBJECT macro and run Qt's meta-object-
        compiler (moc) on it. (Yet this isn't possible inside of an "expamples" or "application"-
        folder of the ICL makefile structure)
      - ...
      - And not at least: You have to layout your GUI using QLayouts, QWidgets and QSizePolicys
      
      Of course, a Qt-nerd will say "OK, but where is the problem!", but most of the ICL- users
      including me long for a framwork that allows you to "create a slider, and access its current
      value" with not more than 5 lines of additional code.\n
      The new ICL-GUI API enables you to create a slider with one expression, and to access its value
      with another expression. The following example will demonstrate this:
      \code
      #include <iclGUI.h>
      using namespace icl;
      int main(int n, char**ppc){
        QApplication app(n,ppc);
        GUI g("slider(0,100,50)[@out=the slider value]");
        g.show();
      return app.exec();
      }
      \endcode
      To access the current value of this slider from the working thread you just have to call:
      \code
      int val = g.getValue<int>("the slider value");
      \endcode
      This will show the following GUI (with the very beatiful gnome desktop) 
      \image html Image01_ASlider.jpg
      
      \section CG Complex GUI's
      Ok! now let us create some more complex GUI:
      \code
      #include <iclGUI.h>
      #include <iclQuick.h>
      
      // create a nice image, which should be shown (using iclQuick)
      Img8u image = cvt8u(scale(create("parrot"),0.2));
      
      int main(int n, char**ppc){
        QApplication app(n,ppc);

        // create the top level gui component (a vertical layouted box here)
        // gui objects can be filled with other objects using the "<<" operator
        GUI gui("vbox");
      
        // add a slider to the hbox (range 0-100, current value=50)
        gui << "slider(0,100,50)[@out=width]";

        // add another slider
        gui << "slider(0,100,50)[@out=height]";

        // add an image component (ensure its size to be at least 10x10 cells 
        // (a cell has a size of 15x15 pixels)  
        gui << "image[@inp=image@minsize=10x10]";
      
        // show the top level gui (this will create all components internally
        // and recursively. Before the top-level "show" function is called,
        // the gui data is unaccessible
        gui.show();
      
        // get the images drawing context (of type ICLWidget*) and induce it 
        // to show a new image
        gui.getValue<ICLWidget*>("image")->setImage(&image);
      
        // start Qt's event loop
        return app.exec();
      }
      
      \endcode
      \image html Image02_FirstImage.jpg


  */
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
