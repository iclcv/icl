/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/GUIWidget.h                              **
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

#ifndef ICL_GUI_WIDGET_H
#define ICL_GUI_WIDGET_H

#include <QWidget>
#include <ICLUtils/Size.h>
#include <ICLQt/GUI.h>
#include <ICLUtils/Uncopyable.h>

/** \cond */
class QGridLayout;
class QLayout;
/** \endcond */


namespace icl{
  /** \cond */
  class GUIDefinition;
  class ProxyLayout;
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
    /** @param def GUIDefinition instance
        @param minParamCount minimum count of expected parameters
        @param maxParamCount maximum count of expected parameters (if -1, this is also minParamCount)
        @param lt layout-type to use
        @param defMinSize default minimum size constraint for the widget
    */
    GUIWidget(const GUIDefinition &def, 
              int minParamCount, 
              int maxParamCount=-1,
              layoutType lt=gridLayout, 
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
    
    /// this shall help to add containes that dont work with layouts (such as tab-widgets)
    virtual ProxyLayout *getProxyLayout() { return 0; }
    
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
    void cb();

    /// returns the widgets preferres size 
    virtual QSize sizeHint () const;
    
    private:
    /// initial layout managers
    QGridLayout *m_poGridLayout;
    QLayout *m_poOtherLayout;
    GUI *m_poGUI;
    std::string *m_handle;
    std::vector<GUI::CallbackPtr> m_vecCallbacks;
    Size m_preferredSize;
  };
}
#endif
