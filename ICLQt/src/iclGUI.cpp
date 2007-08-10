#include <iclGUI.h>
#include <QWidget>
#include <QLayout>
#include <iclStrTok.h>
#include <iclSize.h>
#include <iclCore.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <map>
#include <iclGUIWidget.h>
#include <iclGUIDefinition.h>
#include <QPushButton>
#include <QLabel>

using namespace std;
using namespace icl;

namespace icl{
  
  struct HBoxGUIWidget : public GUIWidget{
    // {{{ open
    HBoxGUIWidget(const GUIDefinition &def):GUIWidget(def,false){
      setLayout(new QHBoxLayout(this));
      layout()->setMargin(2);
      layout()->setSpacing(2);
    }
  };
  
  // }}}
  struct VBoxGUIWidget : public GUIWidget{
    // {{{ open
    VBoxGUIWidget(const GUIDefinition &def):GUIWidget(def,false){
      setLayout(new QVBoxLayout(this));
      layout()->setMargin(2);
      layout()->setSpacing(2);
    }
  };

  // }}}
  struct ButtonGUIWidget : public GUIWidget{
    // {{{ open
    ButtonGUIWidget(const GUIDefinition &def):GUIWidget(def){
      QPushButton *b = new QPushButton(def.param(0).c_str());
      printf("created button with text %s \n",def.param(0).c_str());
      addToGrid(b);
      connect(b,SIGNAL(pressed()),this,SLOT(ioSlot()));
      
      getGUI()->lockData();
      m_pbClicked = &getGUI()->allocValue<bool>(def.output(0),false);
      getGUI()->unlockData();
    }
    virtual void processIO(){
      *m_pbClicked = true;
    }
    virtual Size getDefaultSize() { 
      return Size(4,1); 
    }
  private:
    bool *m_pbClicked;
  };

  // }}}
 
  /// template for creating arbitrary GUIWidget's
  template<class T>
  GUIWidget *create_widget_template(const GUIDefinition &def){
    // {{{ open

    return new T(def);
  }

  // }}}
  
  /// Definition for an arbitrary GUIWidget creator function
  typedef GUIWidget* (*gui_widget_creator_function)(const GUIDefinition &def);
  
  // Type definition for a function map to accelerate the gui creation process
  typedef std::map<string,gui_widget_creator_function> CreatorFuncMap;

  
  /// NEW CREATOR MAP ENTRIES HERE !!!
  /*  
      This function is called by the GUI::create function,
      to create arbitrary widgets. To accelerate the widget creation process
      it build a CreatorFuncMap which uses the GUIDefinitinos type-string as
      identifier to estimate which creation function must be called.         */
  GUIWidget *create_widget(const GUIDefinition &def){
    // {{{ open

    /// use a static map to accelerate the widget creation process
    static bool first = true;
    static CreatorFuncMap MAP_CREATOR_FUNCS;
    if(first){
      first = false;
      /// Fill the map with creator function ( use the template if possible )
      MAP_CREATOR_FUNCS["hbox"] = create_widget_template<HBoxGUIWidget>;
      MAP_CREATOR_FUNCS["vbox"] = create_widget_template<VBoxGUIWidget>;
      MAP_CREATOR_FUNCS["button"] = create_widget_template<ButtonGUIWidget>;
    }
    
    /// find the creator function
    CreatorFuncMap::iterator it = MAP_CREATOR_FUNCS.find(def.type());
    if(it != MAP_CREATOR_FUNCS.end()){
      /// call the function if it could be found
      return it->second(def);
    }else{
      ERROR_LOG("unknown type \""<< def.type() << "\"");
      return 0;
    }
  }

  // }}}
  
  GUI::GUI(const std::string &definition,const GUIDataStore &ds):
    // {{{ open
    m_sDefinition(definition),m_poWidget(0),m_oDataStore(ds){
  }

  // }}}
  GUI::GUI(const GUI &g, const GUIDataStore &ds):
    // {{{ open
    m_sDefinition(g.m_sDefinition),
    m_vecChilds(g.m_vecChilds),
    m_poWidget(NULL),
    m_oDataStore(ds){
  }
  // }}}
  
  GUI &GUI::operator<<(const std::string &definition){
    // {{{ open

    if(m_poWidget) { ERROR_LOG("this GUI is already visible"); return *this; }
    m_vecChilds.push_back(new GUI(definition,m_oDataStore));
    return *this;
  }

  // }}}
  GUI &GUI::operator<<(const GUI &g){
    // {{{ open

    if(m_poWidget) { ERROR_LOG("this GUI is already visible"); return *this; }
    m_vecChilds.push_back(new GUI(g, m_oDataStore));
    return *this;
  }

  // }}}
  
  void GUI::create(QLayout *parentLayout){
    // {{{ open
    GUIDefinition def(m_sDefinition,this,parentLayout);
    m_poWidget = create_widget(def);
    printf("creating component with parent = %p  and %d childs\n",(void*)parentLayout,m_vecChilds.size());

    if(!m_poWidget){
      ERROR_LOG("Widget could not be created ( errors may follow ) \n");
    }
    QLayout *layout = m_poWidget->layout();
    if(!layout && m_vecChilds.size()){
      ERROR_LOG("GUI widget has no layout, "<< m_vecChilds.size() <<" child components can't be added!");
      return;
    }
    for(unsigned int i=0;i<m_vecChilds.size();i++){
      m_vecChilds[i]->create(layout);
    }
  }

  // }}}
  void GUI::show(){
    // {{{ open

    create(0);
    m_poWidget->show();
  }

  // }}}

}
