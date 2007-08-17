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

#include <QLayout>
#include <QWidget>
#include <QApplication>

/** \cond */
class QLayout;
/** \endcond */


/// The icl namespace
namespace icl{

  /** \cond */
  class GUIWidget;
  /** \endcond */

  /// Simple but powerful GUI Toolkit for the ICL \ingroup COMMON
  /** \section INTRO Introduction
      When working on computer-vision tasks, one whats to have some graphical user interface 
      to show the current images. This can be achieved by the use of the ICLQt's ICLWidget and 
      ICLDrawWidget classes. \n
      However in many cases this GUI components are not enough, e.g.:
      - a value shall be adjusted at runtime (using a slider, a spinbox or a simple textfield)
      - a mode shall be switched (using a radiobutton group or a combobox)
      - the application shall have a "pause"-button which stops the iteration thread temporarily
      - ...
      
      Actually all this capabilities can be implemented using the powerful Qt-Framework. But
      you will encounter some tricky problems then:
      - You must synchronize Qt's even-loop with the working thread
      - You have to handle user interaction using Qt's slots and signals
      - You have to create QObject classes using the Q_OBJECT macro and run Qt's meta-object-
        compiler (moc) on it. (Yet this isn't possible inside of an "examples" or "application"-
        folder of the ICL makefile structure)
      - ...
      - And not at least: You have to layout your GUI using QLayouts, QWidgets and QSizePolicys
      
      Of course, a Qt-nerd will say "OK, but where is the problem!", but most of the ICL- users
      including me long for a framework that allows you to "create a slider, and access its current
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
      This will show the following GUI (with the very beautiful gnome desktop) 
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
        // the gui data is inaccessible
        gui.show();
      
        // get the images drawing context (of type ICLWidget*) and induce it 
        // to show a new image
        gui.getValue<ICLWidget*>("image")->setImage(&image);
      
        // start Qt's event loop
        return app.exec();
      }
      
      \endcode
      \image html Image02_FirstImage.jpg

      \section SBS Step by Step
      
      In the last section we have seen, that the GUI API of the ICL helps us to
      create gui-components very quickly and conveniently. Furthermore each GUI
      object provides a comfortable access function (getValue<T>) to access the
      current values of all embedded GUI components.\n
      In this section we'll have a look on the general syntax for the creation
      of GUI components.\n
      
      Each GUI definition string can be divided in 3 parts, which are just
      concated: 
      -# a type string
      -# a comma separated list of mandatory and type dependent parameters (in
         round brackets)
      -# a list of optional and general parameters (in angular brackets)
      
      <pre>
      TYPE(TYPE_DEPENDEND_PARAMS)[GENERAL_PARAMS]
      </pre>
      
      Examples:
      \code
      GUI g1("slider(0,100,50)");
      GUI g2("combo(red,green,blue,yellow,magenty,!cyan,white,black)[@size=3x1@label=colors]");
      GUI g3("image");
      GUI g3("draw[@minsize=20x20]");
      GUI g4("label(input-image)");
      ...      
      \endcode
      
      As the examples show, the TYPE part alone is mandatory; the other 2 parts can be omitted
      in some cases:
      
      \subsection TYPES The Type String
      The mandatory type string defines what type of widget should be created. The correct syntax 
      of the TYPE_DEPENDEND_PARAMS list depends on this string. Possible type strings are:
      - <b>hbox</b> a horizontal layouted container 
      - <b>vbox</b> a vertical layouted container
      - <b>border</b> a vertical layouted container with a labeled border
      - <b>button</b> a push button
      - <b>buttongroup</b> a set of radio buttons (exclusive)
      - <b>togglebutton</b> a button that can be toggled and untoggled
      - <b>label</b> a label displaying static or dynamic content (integers, floats and string)
      - <b>slider</b> a integer-valued slider with given range
      - <b>fslider</b> a float-valued slider with given range
      - <b>int</b> an input field accepting integers within a given range
      - <b>float</b> an input field accepting float values within a given range
      - <b>string</b> a string input field accepting string with given max. length
      - <b>disp</b> a matrix of labels
      - <b>image</b> an ICLWidget component
      - <b>draw</b> an ICLDrawWidget component
      - <b>combo</b> a combo box
      - <b>spinner</b> a spin box (integer valued with given range)
      - <b>hcontainer</b> a horizontal layouted container which can be used to embed external QWidgets
      - <b>hcontainer</b> a vertical layouted container which can be used to embed external QWidgets
      
      \subsection TYPEPARAMS Type Dependent Parameters
      The 2nd part of the GUI definition string is a comma separated list of type dependent parameters.
      This list is bounded by round brackets; if it is empty you can write "()" or just omit this list
      completely. Some types define default values for some given parameters (in C++-style). The 
      following list shows all components introduced above with their individual parameter list syntax:
      - <b>hbox</b>\n
        No params here! 
      - <b>vbox</b>\n
        No params here!
      - <b>border(string LABEL)</b>\n
        LABEL defines the border label of that layout widget. <b>Please Note:</b> that labels
        and borders can also be added using a "@label=xxx" token in the list of general parameters.
      - <b>button(string TEXT)</b>\n
        TEXT is the button text
      - <b>buttongroup(string TEXT1,string TEXT2,...)</b>\n
        The parameters must be a comma separated list of strings. Each token of that list defines
        a single radio button with that text. One of these texts may have a "!"-prefix (e.g.
        "buttongroup(button1,!button2,button3)[...]". The corresponding button will be selected
        initially. If no "!"-prefix can be found, the first button is selected initially.
      - <b>togglebutton(string UNTOGGLED_TEXT, string TOGGLED_TEXT)</b>\n
        A toggle button switches its text depended on its current "toggle-state". If the button
        should not switch its text, UNTOGGLED_TEXT must be equal to TOGGLED_TEXT.
      - <b>label(string TEXT="")</b>\n
        Each label component can be used to show dynamic as well as static content. A label can be
        initialized with a given string (or an int/float as string). Later on this content can be
        changed by accessing this label from outside the GUI object.
      - <b>slider(int MIN,int MAX,int CURRENT)</b>\n
        A slider is created by a given range {MIN,...,MAX} and a given initial value CURRENT
      - <b>fslider(float MIN,float MAX,float CURRENT)</b>
        A fslider is created by a given range {MIN,...,MAX} and a given initial value CURRENT
      - <b>int</b>\n
        An integer input field is created by a given range {MIN,...,MAX} and a given initial value CURRENT.
        The text field will only allow inputs that are inside of this range.
      - <b>float</b>\n
        A float input field is created by a given range {MIN,...,MAX} and a given initial value CURRENT.
        The text field will only allow inputs that are inside of this range.
      - <b>string(int MAX_LEN)</b>\n
        A string input field is created with a given max length of allowed input-strings.
      - <b>disp(unsigned int NW,unsigned int NH)</b>\n
        Creates a display matrix of size NW x NH. To avoid empty matrices, NW*NH must be > 0. Each
        label of this matrix can later on be accessed to display integers, floats and strings.
      - <b>image</b>\n
        No parameters here!
      - <b>draw</b>\n
        No parameters here!
      - <b>combo(string ENTRY1,string ENTRY2,...)</b>\n
        Creates a combox with given entries. The entry list is comma separated and must have at least on
        element.
      - <b>spinner(int MIN,int MAX,int CURRENT)</b>\n
        An integer valued Spin-Box with a given range {MIN,...,MAX} and a given initial value CURRENT.
      - <b>hcontainer</b>\n
        No parameters here!
      - <b>vcontainer</b>\n
        No parameters here!
      
      \subsection GP General Parameters
      The 3rd part of the GUI definition string is a list of so called general params. "General" means here,
      that these params are available for all components, whereas some of these parameters must be compatible
      to the corresponding component.\n
      The list of general parameters is not comma separated because some of the parameter values are 
      comma separated lists themselves. Instead, each entry must begin with the "@"-character and it ends
      with the following "@"-character or the closing angular bracket "]". The set of general params can 
      be subdivided into two parts:
      - <b>layouting parameters:</b> These parameters affect the layout and the appearance of the widget
        which is created by that component (\@size=..., \@minsize=..., \@maxsize=.., \@label=...)
      - <b>input and output interface definition:</b> (\@inp=.., \@out=...) As mentioned above, each 
        embedded GUI components
        current value(s) or "handle" can be accessed from outside by calling the "getValue<T>()"-function
        template on the top level GUI object. To decide which value/handle should be accessed by that
        call, each input/and output of a GUI component is associated internally with a string-identifier.
        (see the next subsection for more details and some examples)

      \subsection IO Input- and Output-Interface
      Depend on the type of a GUI component, the top level GUI gets additional input and output "pins". 
      In contrast to software frameworks like Neo/NST or TDI, no explicit connection must be established 
      to access a GUI objects input/and output data. Instead, a component allocates a mutex-locked variable
      inside of a so called GUIDataStore that is created by its top level GUI component, and updates this
      variable any time a Qt-GUI-Event on this components occurs. Let's have a look on a short example to
      understand this better:
      \code
      #include <iclGUI.h>
      using namespace icl;
      int main(int n, char**ppc){
        QApplication app(n,ppc);

        // create a new slider component with range [0,100], initialized to 50
        // we name its output "the slider value"
        GUI g("slider(0,100,50)[@out=the slider value]");

        // Yet, g is just a GUI-Definition - no QSlider was created internally and no
        // no slider data can be accessed

        // make g visible, This will create all embedded Qt-Widgets internally and
        // it will allocate data for each component in the GUIs internal GUIDataStore.
        g.show();
      
        // now a single integer value is allocated, which is assigned to the 
        // slider value, and which is updated immediately when the slider is moved.
        // The GUI's getValue<T>() template function can now be used to get a reference
        // (or a pointer) to this slider value
        int &riSliderValue = g.getValue<int>("the slider value");
      
        // or
        int *piSliderValue = &g.getValue<int>("the slider value");
        return app.exec();
      }
      \endcode

      Yet, each component only provides a single out- <b>or</b> input pin, however the 
      GUI-definition syntax can be used to define components with N inputs and M outputs.\n
      As each GUI component has different semantics, the count and the type of it's
      in- and output pins must be regarded. The following list shows all GUI components
      and explains their individual in- and output interface: 
      
      
      <TABLE>
      <TR> <TD>type</TD>         <TD>inputs</TD>          <TD>outputs</TD>            <TD>meaning</TD>                                            </TR>
      <TR> <TD>vbox</TD>         <TD>0</TD>               <TD>0</TD>                  <TD>-</TD>                                                  </TR>
      <TR> <TD>hbox</TD>         <TD>0</TD>               <TD>0</TD>                  <TD>-</TD>                                                  </TR>
      <TR> <TD>border</TD>       <TD>0</TD>               <TD>0</TD>                  <TD>-</TD>                                                  </TR>
      <TR> <TD>button</TD>       <TD>0</TD>               <TD>1 type GUIEvent</TD>    <TD>handle for this button (see below!)</TD>                </TR>
      <TR> <TD>buttongroup</TD>  <TD>0</TD>               <TD>1 type int</TD>         <TD>index of the currently toggled radio button </TD>       </TR>
      <TR> <TD>togglebutton</TD> <TD>0</TD>               <TD>1 type bool</TD>        <TD>true=toggled, false=untoggled</TD>                      </TR>
      <TR> <TD>label</TD>        <TD>1 type GUILabel</TD> <TD>0</TD>                  <TD>handle for this label (see below!)</TD>                </TR>
      <TR> <TD>slider</TD>       <TD>0</TD>               <TD>1 type int</TD>         <TD>current slider value</TD>                               </TR>
      <TR> <TD>fslider</TD>      <TD>0</TD>               <TD>1 type float</TD>       <TD>current slider value</TD>                               </TR>
      <TR> <TD>int</TD>          <TD>0</TD>               <TD>1 type int</TD>         <TD>current text input field content</TD>                   </TR>
      <TR> <TD>float</TD>        <TD>0</TD>               <TD>1 type float</TD>       <TD>current text input field content</TD>                   </TR>
      <TR> <TD>string</TD>       <TD>0</TD>               <TD>1 type std::string</TD> <TD>current text input field content</TD>                   </TR>
      <TR> <TD>disp</TD>         <TD>1 type GUILabelMatrix</TD> <TD>0</TD>            <TD>handle for the label matrix (see below!) </TD>          </TR>
      <TR> <TD>image</TD>        <TD>1 type ICLWidget*</TD>     <TD>0</TD>            <TD>handle for the embedded ICLWidget</TD>                  </TR>
      <TR> <TD>draw</TD>         <TD>1 type ICLDrawWidget*</TD> <TD>0</TD>            <TD>handle for the embedded ICLDrawWidget</TD>              </TR>
      <TR> <TD>combo</TD>        <TD>0</TD>               <TD>1 type std::string</TD> <TD>current selected item</TD>                              </TR>
      <TR> <TD>spinner</TD>      <TD>0</TD>               <TD>1 type int</TD>         <TD>current value</TD>                                      </TR>
      <TR> <TD>hcontainer</TD>   <TD>2 QWidget*, QLayout*</TD>  <TD>0</TD>             <TD>The widget itself and its QHBoxLayout</TD>              </TR>
      <TR> <TD>vcontainer</TD>   <TD>2 QWidget*, QLayout*</TD>  <TD>0</TD>             <TD>The widget itself and its QVBoxLayout</TD>              </TR>
      </TABLE>
      
      \subsubsection SpT Special Interface Types (handles)

      Some components use a special interface type to facilitate working with GUI objects. E.g. 
      the "label" component uses the special type GUILabel. To understand this, we have to
      concentrate on what we actually want to do:
      When accessing a labels output interface using e.g. 
      \code
      GUILabel &l = g.getValue<GUILabel>("the labels id");
      \endcode
      we want to get an object, which can be used to make the corresponding QLabel to
      show another string. To achieve this, the GUILabel object contains a back-link
      to the associated QLabel, which is used to make this QLabel show strings, integers
      and floats that are assigned to the GUILabel. This sounds complicated, but in
      practice it is very simple:
      
      \code
      GUILabel &l = g.getValue<GUILabel>("the labels id");
      
      // make the label to show a string
      l = "ICL rocks!";
      
      // make it show an integer
      l = 5;
      
      // or a float
      l = 3.1415;      
      \endcode

      The disp component makes use of the GUILabel class to provide an interface
      of type GUILabelMatrix. Internally the GUILabelMatrix is just a typedef to a matrix
      of GUILabel objects:
      \code
      typedef SimpleMatrix<GUILabel,GUILabelAlloc>  GUILabelMatrix;
      \endcode
      As matrix elements can be addressed using the [][]-operator (see SimpleMatrix),
      all labels of a disp-component can easily be set up.
      
      \code
      #include <iclGUI.h>
      
      using namespace icl;
      
      int main(int n, char**ppc){
        QApplication app(n,ppc);
  
        // create a top-level horizontal box
        GUI gui("hbox");
      
        // add a new 4x4 display component
        // with a bordered label, a min. size of 14 x 6 
        // cells and interface id "mydisp"
        gui << "disp(4,4)[@label=My Disp@minsize=14x6@inp=mydisp]";
      
        // show this gui (remember QWidgets are created here,
        // and the interface data is allocated
        gui.show();

        // Extract the disps handle of Type GUILabelMatrix
        GUILabelMatrix &m = gui.getValue<GUILabelMatrix>("mydisp");
      
        // assign the first row with string
        m[0][0] = "";
        m[1][0] = "column 1";
        m[2][0] = "column 2";
        m[3][0] = "column 3";

        // assign the first column with strings
        m[0][1] = "row 1";
        m[0][2] = "row 2";
        m[0][3] = "row 3";
      
        // assign the rest with integers
        for(int x=1;x<4;x++){
          for(int y=1;y<4;y++){
            m[x][y] = 10*x+y;
          }
        }
      
        // enter Qt's event loop
        return app.exec();
      }
      \endcode
      \image html Image03_DispExample.jpg 
      
      The last interface type, that needs further explanations is the GUIEvent type, which is
      used for the "button" type interface. In contrast to all other components, simple buttons
      are producing an event instead of some data. When accessing the button from the working
      thread, you don't need the information if the button is pressed at this time, but you
      may want to know if it was pressed since the last test. You might say that a simple boolean
      variable would be sufficient to handle this information, but the following stays doubtful:
      "Who resets this boolean variable, and when?". To avoid this problem the GUIEvent
      data type, can be triggered (if the button is pressed) an it can be checked using its 
      wasTriggered() function, which returns if the event was triggered and resets the internal
      boolean variable to false.

      \subsection EMB Embedding external QWidgets
      In some cases it might be necessary to embed QWidgets, which are not supported by the GUI-API.
      For this, two additional components (hcontainer and vcontainer) are provided. This containers
      use the input interface, to pass the containers QWidget itself and its layout to the GUI
      interface. By this means you can access theses pointers directly as parents for your own
      widget. See the following example for more details:
      \code
      #include <iclGUI.h>
      #include <QProgressBar>
      
      using namespace icl;
      
      int main(int n, char**ppc){
        QApplication app(n,ppc);
      
        // create the top level gui component
        GUI gui("vbox");
      
        // add some buttons with funny texts
        gui << "button(click me)[@out=click0]"<< "button(no !click me)[@out=click1]"<< "button(no no no! me!)[@out=click2]";
      
        // add the container widget (comma separated list of output identifiers)
        gui << "hcontainer[@inp=widget,layout@label=Progress]";
      
        // create the gui (this allocates input and output data)
        gui.show();
      
        // create a new QProgressBar using the containers widget as parent
        QProgressBar *pb = new QProgressBar(gui.getValue<QWidget*>("widget"));
        pb->setValue(50);
      
        // add it to the widgets layout
        gui.getValue<QLayout*>("layout")->addWidget(pb);
      
        // enter Qt's event loop
        return app.exec();
      }
      \endcode
      
      \image html Image04_ExternalWidget.jpg
      
      \subsection LOCK Locking
      Some interface types involve the danger to be corrupted when accessed by the working thread
      as well as by Qts GUI-Thread. This is true for all complex data types like strings and so on.
      To avoid errors rising on multi-threaded data access, call the GUIs lockData() and unlockData()
      function.

      \subsection PERF Performance
      Although the implementation of the GUIs data access function is highly optimized, 
      it costs some processor cycles to pick a value of a specific component. Internally
      all values are stored in a large hash-map, which allows the getValue<T> template
      function to find a specific entry quickly (even if the GUI consist of 100 components
      which is not realistic). If a value is found, the RTTI (c++ Run-Time Type 
      identification) interface is used, to decide whether the correct template parameter
      was used, otherwise an error occurs, and the program is aborted.\n
      So it is much faster to extract a value from a gui only once (at reference or pointer) and
      work with this reference later on.
      
      \subsection EX Extensions
      An extension, which must be implemented is a special GUI constructor which allows to
      embed arbitrary QWidgets into the GUI. This GUIWidgets could allocate themselves as
      in input pin (...)
  */
  class GUI{
    public:
    /// cell width (all sizes are given in this unit)
    static const int CELLW = 20;
    /// cell height (all sizes are given in this unit)
    static const int CELLH = 20;
    
    /// default constructor 
    GUI(const std::string &definition="vbox");
    
    /// copy constructor
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
