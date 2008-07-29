#ifndef ICL_GUI_WIDGET_H
#define ICL_GUI_WIDGET_H

#include <QWidget>
#include <iclSize.h>
#include <iclGUI.h>
#include <iclUncopyable.h>

/** \cond */
class QGridLayout;
class QLayout;
/** \endcond */


namespace icl{
  /** \cond */
  class GUIDefinition;
  /** \endcond */

  /// Abstract class for GUI components \ingroup UNCOMMON
  class GUIWidget : public QWidget, public Uncopyable{
    Q_OBJECT;
    public:
    enum layoutType{
      noLayout,       // do not call setLayout(..)
      hboxLayout,     // use a QHBoxLayout
      vboxLayout,     // use a QVBoxLayout
      gridLayout      // use the default GridLayout
    };
    
    /// create a new GUIWidget ( this constructor must be called by all subclasses )
    GUIWidget(const GUIDefinition &def, 
              layoutType lt=gridLayout, 
              int ensureNumInputs=-1,
              int ensureNumOutputs=-1,
              int ensureNumParams=-1, 
              const Size &defMinSize=Size(0,0));


    /// Destructor
    virtual ~GUIWidget();
    
    public slots:
    /// imediately calls processIO
    /** This slot must be connected to the custom widgets
        "somethis-has-changed"-signal e.g. A button's "clicked"
        or a sliders "moved" function */
    void ioSlot();

    /// help function to add new sub components
    /** This function does only work as long the underlying layout manamer is a QGridLayout */
    void addToGrid(QWidget *c, int x=0, int y=0, int width=1,int height=1); 
    
    /// virtual function which must be implemented for a components custom "new-data"-event
    virtual void processIO(){}

    /// this function must be reimplemented for other layouts the hbox, vbox or grid
    virtual QLayout *getGUIWidgetLayout() { return m_poGridLayout ? (QLayout*)m_poGridLayout : m_poOtherLayout; }
    
    /// returns the underlying GUI structure
    GUI *getGUI(){ return m_poGUI; }

    /// registers a callback on this gui widget
    void registerCallback(GUI::CallbackPtr cb){
      m_vecCallbacks.push_back(cb);
    }
    /// remove all callbacks from this widget
    void removeCallbacks(){
      m_vecCallbacks.clear();
    }

    /// Callback execution 
    /** this function must be called by each special component
        when registered callbacks should be executed
    */
    void cb(){
      for(unsigned int i=0;i<m_vecCallbacks.size();++i){
        m_vecCallbacks[i]->exec();
      }
    }
    
    private:
    /// initial layout managers
    QGridLayout *m_poGridLayout;
    QLayout *m_poOtherLayout;
    GUI *m_poGUI;

    std::vector<GUI::CallbackPtr> m_vecCallbacks;
  };
}
#endif
