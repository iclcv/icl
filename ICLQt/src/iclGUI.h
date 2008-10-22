#ifndef ICL_GUI_H
#define ICL_GUI_H

#include <string>
#include <vector>
#include <iclSmartPtr.h>
#include <iclDataStore.h>


#include <iclWidget.h>
#include <iclDrawWidget.h>
#include <iclDrawWidget3D.h>

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

  /// Simple but powerful GUI Toolkit for the ICL \ingroup COMMON
  /** \section INTRO Introduction
      When working on computer-vision tasks, one whats to have some graphical user interface 
      to show the current images. This can be achieved by the use of the ICLQt's ICLWidget and 
      ICLDrawWidget classes. \n
      However in many cases this GUI components are not enough, e.g.:
      - a value shall be adjusted at run-time (using a slider, a spinbox or a simple textfield)
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
        // (a cell has a size of 15x15 pixels) The @handle=... statement makes
        // this component allocate a handle for it in its parent GUI object
        gui << "image[@handle=image@minsize=10x10]";
      
        // show the top level gui (this will create all components internally
        // and recursively. Before the top-level "show" function is called,
        // the gui data is inaccessible
        gui.show();
      
        // get the images drawing context (as an ImageHandle) and induce it 
        // to show a new image
        gui.getValue<ImageHandle>("image") = &image;
      
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
      - <b>tab</b> a tabbed contatiner widget 
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
      - <b>draw3D</b> an ICLDrawWidget3D component
      - <b>combo</b> a combo box
      - <b>spinner</b> a spin box (integer valued with given range)
      - <b>fps</b> label component showing current frames per second
      - <b>multidraw</b> tabbed widget of draw widget components accessible via string index

      
        
      \subsection TYPEPARAMS Type Dependent Parameters
      The 2nd part of the GUI definition string is a comma separated list of type dependent parameters.
      This list is bounded by round brackets; if it is empty you can write "()" or just omit this list
      completely. Some types define default values for some given parameters (in C++-style). The 
      following list shows all components introduced above with their individual parameter list syntax:
      - <b>hbox</b>\n
        No params here! 
      - <b>vbox</b>\n
        No params here!
      - <b>tab(string TAB_LABEL_1, string TAB_LABEL_2,..)</b>\n
        If more components are added, than names were given, error messages are shown, and the 
        tabs are added with some dummy names (e.g. "Tab 3")
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
      - <b>draw3D</b>\n
        No parameters here!
      - <b>combo(string ENTRY1,string ENTRY2,...)</b>\n
        Creates a combox with given entries. The entry list is comma separated and must have at least on
        element.
      - <b>spinner(int MIN,int MAX,int CURRENT)</b>\n
        An integer valued Spin-Box with a given range {MIN,...,MAX} and a given initial value CURRENT.
      - <b>fps(int TIME_WINDOW_SIZE)</b>\n
        A Label component showing current frames per second averaged over last TIME_WINDOW_SIZE time steps.
      - <b>multidraw(string buffermode=!one, string buffermethod=!deepcopy, CommaSepListOfTabs)</b>\n
        Creates a tabbed widget containing an image widget on each tab to visualize multiple images 
        simultaneously (selectable by mouse using tabs). Internally this is achieved using a single
        DrawWidget which is set up with different images. The calling variable <em>buffermode</em> (possible 
        values are "!all" or "!one") allows to 
        to set up the widget to buffer all images internally (dependent on value of buffermethod either using 
        ImageBase::deepCopy (value "!deepcopy") or using a shallow pointer copy (value "!shallowcopy") ) or to buffer 
        images only, when corresponding tab is really shown actually. <b>Note</b> that obviously the control parameters
        begin with a '!'-character!\n      
        The <b>multidraw</b> component provides a powerful handle, which implements
        the array-index operator([]) with std::string-argument to manipulate the content of a certain tab
        only.\n
        If your application runs at high frame rate (e.g. 15Hz), it will be sufficient to use default
        settings (buffermode=one). Otherwise, if application runs slowly (e.g. only 2Hz, this) it will become
        more responsive if buffermode is set to "all". If images displayed are held permanently, it will
        speed up performance if buffermethod is set to "shallowcopy" then.
      
      

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
      - <b>input and output interface definition:</b> (\@inp=.., \@out=...\@handle=...) As mentioned above, 
        each embedded GUI components current value(s) or can be accessed from outside by calling the 
        "getValue<T>()"-function template on the top level GUI object. 
        To decide which value/handle should be accessed by that call, each input/and output of a GUI 
        component is associated internally with a string-identifier. In addition each component can be
        controlled by a so called GUIHandle object which is implemented for each provided GUI component.
        By default, a gui component "xyz" provides a Handle class "XyzHandle". This handle is also allocated
        in the top-level GUI objects Data-Store, if the "@handle=NAME" token was defined in the definition
        string. The handles "NAME" is used as string identifier for the GUI-DataStore. (see the next 
        subsection for more details and some examples)

      \subsection IO Input- and Output-Interface
      Depend on the type of a GUI component, the top level GUI gets additional input and output "pins". 
      (<b>Note: input pins have been replaced by the GUIHandles, but they remain in the API.</b>)
      In contrast to software frameworks like Neo/NST or TDI, no explicit connection must be established 
      to access a GUI objects input/and output data. Instead, a component allocates a mutex-locked variable
      inside of a so called DataStore that is created by its top level GUI component, and updates this
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
        // it will allocate data for each component in the GUIs internal DataStore.
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

      Yet, only the most components provide an output pin, however the 
      GUI-definition syntax can be used to define components with N inputs and M outputs.\n
      As each GUI component has different semantics, the count and the type of it's
      in- and output pins must be regarded. The following list shows all GUI components
      and explains their individual in- and output interface: 
      
      
      <TABLE>
      <TR> <TD><b>type</b></TD>  <TD><b>handle</b></TD>     <TD><b>outputs</b></TD>     <TD><b>meaning</b></TD>                                     </TR>
      <TR> <TD>vbox</TD>         <TD>BoxHandle</TD>         <TD>0</TD>                  <TD>-</TD>                                                  </TR>
      <TR> <TD>tab</TD>          <TD>TabHandle</TD>         <TD>0</TD>                  <TD>-</TD>                                                  </TR>
      <TR> <TD>hbox</TD>         <TD>BoxHandle</TD>         <TD>0</TD>                  <TD>-</TD>                                                  </TR>
      <TR> <TD>border</TD>       <TD>BorderHandle</TD>      <TD>0</TD>                  <TD>-</TD>                                                  </TR>
      <TR> <TD>button</TD>       <TD>ButtonHandle</TD>      <TD>1 type GUIEvent</TD>    <TD>handle for this button (see below!)</TD>                </TR>
      <TR> <TD>buttongroup</TD>  <TD>ButtonGroupHandle</TD> <TD>1 type int</TD>         <TD>index of the currently toggled radio button </TD>       </TR>
      <TR> <TD>togglebutton</TD> <TD>ButtonHandle</TD>      <TD>1 type bool</TD>        <TD>true=toggled, false=untoggled</TD>                      </TR>
      <TR> <TD>label</TD>        <TD>LabelHandle</TD>       <TD>0</TD>                  <TD>handle for this label (see below!)</TD>                 </TR>
      <TR> <TD>slider</TD>       <TD>SliderHandle</TD>      <TD>1 type int</TD>         <TD>current slider value</TD>                               </TR>
      <TR> <TD>fslider</TD>      <TD>FSliderHandle</TD>     <TD>1 type float</TD>       <TD>current slider value</TD>                               </TR>
      <TR> <TD>int</TD>          <TD>IntHandle</TD>         <TD>1 type int</TD>         <TD>current text input field content</TD>                   </TR>
      <TR> <TD>float</TD>        <TD>FloatHandle</TD>       <TD>1 type float</TD>       <TD>current text input field content</TD>                   </TR>
      <TR> <TD>string</TD>       <TD>StringHandle</TD>      <TD>1 type std::string</TD> <TD>current text input field content</TD>                   </TR>
      <TR> <TD>disp</TD>         <TD>DispHandle</TD>        <TD>0</TD>                  <TD>handle for the label matrix (see below!) </TD>          </TR>
      <TR> <TD>image</TD>        <TD>ImageHandle</TD>       <TD>0</TD>                  <TD>handle for the embedded ICLWidget</TD>                  </TR>
      <TR> <TD>draw</TD>         <TD>DrawHandle</TD>        <TD>0</TD>                  <TD>handle for the embedded ICLDrawWidget</TD>              </TR>
      <TR> <TD>draw3D</TD>       <TD>DrawHandle<3D/TD>      <TD>0</TD>                  <TD>handle for the embedded ICLDrawWidget3D</TD>            </TR>
      <TR> <TD>combo</TD>        <TD>ComboHandle</TD>       <TD>1 type std::string</TD> <TD>current selected item</TD>                              </TR>
      <TR> <TD>spinner</TD>      <TD>SpinnerHandle</TD>     <TD>1 type int</TD>         <TD>current value</TD>                                      </TR>
      <TR> <TD>fps</TD>          <TD>FPSHandle</TD>         <TD>0</TD>                  <TD>-</TD>                                                  </TR>
      <TR> <TD>multidraw</TD>    <TD>MultiDrawHandle</TD>   <TD>0</TD>                  <TD>handle of [string]-accessible ICLDrawWidgets</TD>       </TR>
      </TABLE>
      
      \section HVV Handles vs. Values

      
      In some cases accessing a components value is not enough. E.g. if a "label" component
      should be used to show another string, of if a slider should be set externally to a
      specific value. \n
      To facilitate working with GUI objects, each component is able to allocate a so called
      "handle"-object of itself in its parent GUI-objects data store. This will be done
      if a "@handle=.." token is found in the general params list of this component definition
      (general params are inside angular brackets).\n
      All handle classes are derived from the GUIHandle<T> template class, which provides
      functionalities for storing a T-pointer (template parameter) and which offers functions
      to access this pointer (the *operator). A ButtonHandle for example inherits
      the GUIHandle<QPushButton>, so it wraps a QPushButton* internally. For a more
      convenient working process, each handle has some special functions which provide
      and abstract and direct access of the underlying class without knowing it.
      
      \code
      #include <iclGUI.h>
      #include <QSlider>
      
      using namespace icl;
      int main(int n, char**ppc){
        QApplication app(n,ppc);
      
        // create a new slider with output value and a handle id string
        GUI g("slider(0,100,50)[@out=the slider value@handle=the slider handle]");
      
        // show it
        g.show();
      
        // access the sliders value
        int &val = g.getValue<int>("the slider value");
      
        // val cannot be used to set a new slider position
        val = 5; // the slider will not change
      
        // accessing the Sliders handle
        SliderHandle h = g.getValue<SliderHandle>("the slider handle");
      
        // the handle can be used to affect the underlying QSlider component
        h = 5;
  
        // And the handle can be used to access the QSlider directly 
        // To work with this slider, the \#include <QSlider> statement is mandatory
        // because the SliderHandle uses the QSlider class forward declared.
        QSlider *sl = *h;  
      
        // now the slider itself can be manipulated
        sl->setValue(7);
      
        // enter qts event loop
        return app.exec();
      }

      \endcode
      
      The following subsection shows some examples for different GUIHandles.
      
      \subsection LH LabelHandles
      
      Labels can be used to show strings, integers and floats
      
      \code
      
      #include <iclGUI.h>
      #include <iclQuick.h>
      
      int main(int n, char **ppc){
        QApplication app(n,ppc);
      
        /// create a container 
        GUI gui("vbox");
      
        // add some labels
        gui << "label(Text 1)[@handle=L1@size=6x1@label=Label 1]";
        gui << "label(Text 2)[@handle=L2@size=6x1@label=Label 2]";
        gui << "label(Text 3)[@handle=L3@size=6x1@label=Label 3]";
      
        // show the gui
        gui.show();
      
        // access the sliders value 
        gui.getValue<LabelHandle>("L1") = "A New Text";
        gui.getValue<LabelHandle>("L2") = 5;
        gui.getValue<LabelHandle>("L3") = 3.1415;
      
        return app.exec();
      }

      \endcode
      
      \image html Image05_LabelDemo.jpg

      \subsection DISPC DispHandles
      
      The disp component implements a 2D-Array of label components (e.g. to visualize
      a matrix). It makes use of the LabelHandle class to provide an interface
      of type DispHandle which wraps a matrix of LabelHandles using the ICLUtils/
      SimpleMatrix template class. \n
      Matrix elements - of type "LabelHandle&" - can be addressed using the 
      [][]-operator (see SimpleMatrix).
      
      \code
      #include <iclGUI.h>
      
      using namespace icl;
      
      int main(int n, char**ppc){
        QApplication app(n,ppc);
      
        // create top-level container
        GUI gui;
      
        // add a new 4x4 display component
        // with a bordered label, a min. size of 14 x 6 cells and
        // handle id mydisp
        gui << "disp(4,4)[@label=My Display Component@minsize=14x6@handle=mydisp]";
      
        // show this gui (remember: QWidgets are created here,
        // and the interface data is allocated
        gui.show();
      
        // Extract the displays handle
        DispHandle &m = gui.getValue<DispHandle>("mydisp");
      
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
      
      \subsection BHA Button Handles
      
      The next interface type, that should be introduced here in detail, is the ButtonHandle type,
      which is used for the "button" type interface. The button itself produces no data; it can
      only be accessed by using its handle. In contrast to other components, simple buttons
      are producing an event instead of some data. When accessing the button from the working
      thread, you don't need the information if the button is pressed at this time, but you
      may want to know if it <em>was</em> pressed since the last test. You might say that a simple boolean
      variable would be sufficient to handle this information, but the following stays doubtful:
      "Who resets this boolean variable, and when?". To avoid this problem the ButtonHandle
      data type, can be triggered (if the button is pressed) an it can be checked using its 
      wasTriggered() function, which returns if the event was triggered and resets the internal
      boolean variable to false. The following example code illustrates this:

      \code
      #include <iclGUI.h>
      #include <iclThread.h>
      
      using namespace icl;
      
      /// Use a static gui pointer (accessible in main an in the Thread class
      GUI *gui = 0;
      
      // create a Thread class, which implements the working loop. For this example
      // this loop will test each second if the button was pressed or not
      class DemoThread : public Thread{
        virtual void run(){
          while(true){
            if(gui->getValue<ButtonHandle>("b").wasTriggered()){
              printf("button was triggered! \n");
            }else{
              printf("button was not triggered! \n"); 
            }
            sleep(1.0);
          }
        }
      };
      
      int main(int n, char**ppc){
        // create a QApplication object
        QApplication app(n,ppc);
      
        // create the top level container
        gui = new GUI;  
      
        // add a button to this container
        (*gui) << "button(Click me!)[@handle=b]";
      
        // show it
        gui->show();
      
        // create the working loop thread
        DemoThread t;
      
        // start it
        t.start();
      
        // enter Qt's event loop
        return app.exec();
      }
      \endcode
      
      \image html Image06_ButtonDemo.jpg
      
      \subsubsection EAB Events and Buttons
      
      In some applications it might be necessary to associate an event to a button click,
      which is called immediately if the button is clicked. This is quite useful e.g. to interrupt
      the current working thread. However this feature is more complex, then the claim of the
      ICL GUI API can stand, this feature is a "must-have" and it is wrapped into the GUI API
      to avoid that it is implemented many times elsewhere.\n
      First we have to decide how we formalize an "event". Here an event is a callback-function or a
      function object which can be triggered (the function is called). To differentiate between
      these Callback-Objects and functions, two type-definition were made inside of the ButtonHandle
      class:
      \code
      /// Special Utility class for handling Button clicks in the ICL GUI API \ingroup HANDLES
      class ButtonHandle : public GUIHandle<QPushButton>{
        public:
      
        /// type definition for a callback function
        typedef void (*callback)(void);
    
        /// Interface for callback objects (functors)
        struct Callback{
          /// Destructor
          virtual ~Callback(){}
          /// call back function (pure virtual)
          virtual void operator()() = 0;
        };
      
        ...
      \endcode
      Each button handle provides two functions named "registerCallback(..)" to add callbacks
      to a button, which are called exactly when the button is pressed. The following example 
      extends the example above by a simple exit button:

      \code
      #include <iclGUI.h>
      #include <iclThread.h>
      
      using namespace icl;
      
      /// Use a static gui pointer (accessible in main an in the Thread class
      GUI *gui = 0;
      
      // create a Thread class, which implements the working loop. For this example
      // this loop will test each second if the button was pressed or not
      class DemoThread : public Thread{
       ... see above!
      };

      // a simple callback function (type "void (*callback)(void)"
      void exit_callback(void){
        printf("exit callback was called \n");
        exit(0);
      }
      
      int main(int n, char**ppc){
        // create a QApplication object
        QApplication app(n,ppc);
      
        // create the top level container
        gui = new GUI;  
      
        // add a button to this container
        (*gui) << "button(Click me!)[@handle=b]";
        (*gui) << "button(Exit!)[@handle=exit]";
      
        // show it
        gui->show();
      
        // register the call back function
        gui->getValue<ButtonHandle>("exit").registerCallback(exit_callback);
      
        // create the working loop thread
        DemoThread t;
      
        // start it
        t.start();
      
        // enter Qt's event loop
        return app.exec();
      }
      \endcode
      
      
      \subsection EMB Embedding external QWidgets
      In some cases it might be necessary to embed QWidgets, which are not supported by the GUI-API.
      For this, the two box-components ("hbox" and "vbox") do also provide a special BoxHandle which
      wraps the underlying QWidget to provide access to it and its current layout.See the following 
      example for more details:

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
        gui << "vbox[@label=Progress@handle=conti]";
      
        // create the gui (this allocates input and output data)
        gui.show();
      
        // create a new QProgressBar using the containers widget as parent
        QProgressBar *pb = new QProgressBar(0);
        pb->setValue(50);
      
        // add it to the widgets layout
        gui.getValue<BoxHandle>("conti").add(pb);
      
        // enter Qt's event loop
        return app.exec();
      }     
      \endcode
      
      \image html Image04_ExternalWidget.jpg

      \subsection GUIInGUI Embedding other GUIs
      GUI objects can be embedded into other GUI objects by using the optional constructor parameter
      "parent" of the constructor. This can be very useful when creating re-usable GUI classes
      by using the ICL GUI API. To demonstrate this, we adapt the example above:

      \code
      #include <iclGUI.h>
      
      using namespace icl;
      
      int main(int n, char**ppc){
        QApplication app(n,ppc);
      
        // create the top level gui component
        GUI gui_1("hbox");
        
        // add some buttons vertically alligned
        gui_1 << ( GUI("vbox") 
                   << "button(click me)[@out=click0]"
                   << "button(no !click me)[@out=click1]"
                   << "button(no no no! me!)[@out=click2]" 
                 );
      
        // add a container widget
        gui_1 << "vbox[@label=GUI 2@handle=box-handle]";
      
        // create the gui (this allocates input and output data)
        gui_1.show();
      
        /// create to "to be embeded gui (with given parent)
        GUI gui_2("vbox",*gui_1.getValue<BoxHandle>("box-handle"));
      
        /// add something to this gui
        gui_2 << "label(GUI 2!!)";
      
        /// finally create the underlying Qt-stuff
        gui_2.show();
      
        // enter Qt's event loop
        return app.exec();
      }
      
      \endcode

      \image html Image07_GUIInGUI.jpg


      \subsection TABS Tabbed widges
      One of the newes features are tabbed container widgets. These GUIs can be created
      with a comma separated string list containing tab labels. Each time a new child-
      component is added, it is added into the next free tab. If more tabs are added than names
      have been given, new tabs are added with dummy names, but an error will be shown. Tab widges 
      as as easy to use as all the other ICL gui widgets, tab widgets can again contain other
      tab widgets, and of course also external/foreign QWidgets can be added directly using the 
      tab widget's Handle of type TabHandle. The TabHandle provides an 'add'-function as well
      as an 'insert'-function to insert another component at a certain tab index. For more
      complex manipulation of the wrapped QTabWidget, it can be obtained conveniently by 
      'dereferencing' the TabHandle. (TabHandle &h = ...; QTabWidget *w=*h;)
      
      Here's an example for using tabs (available as gui-test-2.cpp):
      
      \code 
      #include <iclCommon.h>
      #include <QProgressBar>
      
      GUI gui;
      
      void run(){
        Img8u image = cvt8u(scale(create("parrot"),0.2));
        ImageHandle *ws[3] = {
          &gui.getValue<ImageHandle>("image1"),
          &gui.getValue<ImageHandle>("image2"),
          &gui.getValue<ImageHandle>("image3")
        };
        ButtonHandle &click = gui.getValue<ButtonHandle>("click");
        while(1){
          for(int i=0;i<3;++i){
            *ws[i] = image;
            ws[i]->update();
          }
          if(click.wasTriggered()){
            std::cout << "button 'click' was triggered!" << std::endl;
          }
          Thread::msleep(50);
        }
      }
      
      int main(int n, char **ppc){
        QApplication app(n,ppc);
      
        gui = GUI("tab(a,b,c,d,e,f)[@handle=tab]");
        
        gui << "image[@handle=image1@label=image1]"
            << "image[@handle=image2@label=image2]"
            << "image[@handle=image3@label=image3]";
      
        GUI v("tab(a,b,c,d,e,f)[@label=internal tab widget]");
            v << "slider(-1000,1000,0)[@out=the-int1@maxsize=35x1@label=slider1@minsize=1x2]"
              << "slider(-1000,1000,0)[@out=the-int2@maxsize=35x1@label=slider2@minsize=1x2]"
              << "slider(-1000,1000,0)[@out=the-int3@maxsize=35x1@label=slider3@minsize=1x2]"
              << "combo(entry1,entry2,entry3)[@out=combo@label=the-combobox]"
              << "spinner(-50,100,20)[@out=the-spinner@label=a spin-box]"
              << "button(click me)[@handle=click]";
      
      
        gui << v;
      
        gui.show();
      
        gui.getValue<TabHandle>("tab").insert(2,new ICLWidget,"ext. 1");
        gui.getValue<TabHandle>("tab").add(new QProgressBar,"ext. 2");
     
        exec_threaded(run);
      
        return app.exec();
      }
      \endcode

      \image html Image08_Tabs.jpg
      

      
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
  */
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
    virtual ~GUI(){}
    
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

    /// returns the GUI internal dataStore
    const DataStore &getDataStore() const { return m_oDataStore; }
    
    
    /// Callback helper class: Default implementation calls a callback function 
    class Callback{
      /// typedef to wrapped function (only for default implementation)
      typedef void (*callback_function)(void);
      
      /// internally used default callback function
      callback_function m_func;
      
      protected:
      /// create a new callback object
      Callback():m_func(0){}
      
      public:
      /// Default implementations constructor with given callback function
      Callback(callback_function func):m_func(func){}
      
      /// vitual execution function
      virtual void exec(){
        if(m_func) m_func();
      }
    };
    typedef SmartPtr<Callback,PointerDelOp> CallbackPtr;
    
    /// registers a callback function on each component 
    /** @param cb callback to execute 
        @param handleNamesList comma-separated list of handle names 
        ownership is passed to the childrens; deletion is performed by
        the smart pointers that are used...
    */
    void registerCallback(CallbackPtr cb, const std::string &handleNamesList);

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
