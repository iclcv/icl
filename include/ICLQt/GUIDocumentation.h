/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/GUIDocumentation.h                       **
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

#ifndef ICL_GUI_DOCUMENTATION_H
#define ICL_GUI_DOCUMENTATION_H


//    The GUI documentation is outsourced to decouple it from dependency-checks during build
//    process. By this means, the GUI documentation can be adapted without having to recompile
//    other sources.    

/** \class icl::GUI
    \brief Simple but powerful GUI Toolkit for the ICL \ingroup COMMON 

    \section GUI_INTRO Introduction to the ICL GUI Toolkit
      Simple creation of image visualization componets is one of the most mandatory convenience 
      features of a computer-vision library. Therefore ICL provides a powerful image visualization 
      component basing on the Qt-GUI-Library.\n
      However in many cases this GUI components are not enough, e.g.,
      - a value has to  be adjusted at run-time (using a slider, a spinbox or a simple textfield)
      - a mode shall be switched (using a radiobutton group or a combobox)
      - the application shall have a "pause"-button which stops the iteration thread temporarily
      - ...
      
      Actually all this capabilities can be implemented using the powerful Qt-Framework. But
      you will encounter some tricky problems then:
      - You must synchronize Qt's even-loop with the working thread
      - You have to handle user interaction using Qt's slots and signals
      - You have to create QObject classes using the Q_OBJECT macro and run Qt's meta-object-
        compiler (moc) on it. 
      - ...
      - And not at least: You have to layout your GUI using QLayouts, QWidgets and QSizePolicys
      
      Of course, a Qt-nerd will say "OK, but where is the problem!", but most of the ICL- users
      including me long for a framework that allows the programmer to "create a slider, and access its current
      value" with not more than 5 lines of additional code.\n
      The ICL-GUI API enables you to create a slider with one expression, and to access its value
      with another expression. The following example will demonstrate this:
      \code
      #include <ICLQt/GUI.h>
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
      This example produces the following GUI (with the very beautiful gnome desktop) 
      \image html Image01_ASlider.jpg
      
    \section CG Complex GUI's
      Ok! now let us create some more complex GUI:
      \code
      #include <ICLQt/GUI.h>
      #include <ICLQuick/Quick.h>
      
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
        // (a cell has a size of 20x20 pixels) The @handle=... statement makes
        // this component allocate a handle for it in its parent GUI object
        gui << "image[@handle=image@minsize=10x10]";
      
        // show the top level gui (this will create all components internally
        // and recursively. Before the top-level "show" function is called,
        // the gui data is inaccessible
        // alternatively you can use gui << "!show". This is sometimes easier
        // if you create you GUI in one multi-line streaming expression
        gui.show();

      
        // get the images drawing context (as an ImageHandle) and induce it 
        // to show a new image
        gui.getValue<ImageHandle>("image") = &image;
        // or use the shortcut gui["image"] = image;
            
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
      
      Each GUI definition string can be split in 3 parts, which are just
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
      GUI g1("slider(0,100,50,vertical)");
      GUI g2("combo(red,green,blue,yellow,magenty,!cyan,white,black)[@size=3x1@label=colors]");
      GUI g3("image");
      GUI g3("draw[@minsize=20x20]");
      GUI g4("label(input-image)");
      ...      
      \endcode
      
      As the examples demonstrates, only the TYPE part of the definition string is mandatory. 
      The other 2 parts can be left out sometimes.
      
      \subsection TYPES The Type String
      The mandatory type string defines what type of widget should be created. The correct syntax 
      of the TYPE_DEPENDEND_PARAMS list depends on this string. Possible type strings are:
      - <b>hbox</b> a horizontal layouted container 
      - <b>hscroll</b> a horizontally layouted container but with scrollbars if contents does not fit into it 
      - <b>vbox</b> a vertical layouted container
      - <b>hscroll</b> a vertically layouted container but with scrollbars if contents does not fit into it 
      - <b>tab</b> a tabbed contatiner widget 
      - <b>hsplit</b> a horizontal layouted containter (boundaries can be moved manually)
      - <b>vsplit</b> a vertical layouted containter (boundaries can be moved manually)
      - <b>border</b> a vertical layouted container with a labeled border
      - <b>button</b> a push button
      - <b>buttongroup</b> a set of radio buttons (exclusive)
      - <b>togglebutton</b> a button that can be toggled and untoggled
      - <b>checkbox</b> a standard checkbox
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
      - <b>camcfg</b> single button that pops up a camera configuration widget (see CamCfgWiget) if clicked
      - <b>config</b> single button or embedded tree-view that enables 
        runtime adaption of configuration file parameters

      \subsection CONTROL Special Control Sequences
      Currently two further string control sequences can also be streamed into GUI instances. Streaming
      "!show" into a GUI instances automatically calles the show() method. In the same way "!create"
      is forwarded to call the create method of the GUI instance. Perhaps there will be other
      control sequences in future.
      
      \code
      // create an empty GUI instance of type 'vbox'
      GUI gui;
      
      // embed an 'image' and create and show the GUI.
      gui << "image[@minsize=32x24]" << "!show";
      \endcode
    
        
      \subsection TYPEPARAMS Type Dependent Parameters
      The 2nd part of the GUI definition string is a comma separated list of type dependent parameters.
      This list is bounded by round brackets; if it is empty you can write "()" or just omit this list
      completely. Some types define default values for some given parameters (in C++-style). The 
      following list shows all components introduced above with their individual parameter list syntax:
      - <b>hbox</b>\n
        No params here! 
      - <b>hscroll</b>\n
        No params here! 
      - <b>vbox</b>\n
        No params here!
      - <b>vscroll</b>\n
        No params here!
      - <b>tab(string TAB_LABEL_1, string TAB_LABEL_2,..)</b>\n
        If more components are added, than names were given, error messages are shown, and the 
        tabs are added with some dummy names (e.g. "Tab 3")
      - <b>hsplit</b>\n
        No params here!
      - <b>vsplit</b>\n
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
      - <b>checkbox(string LABEL, checked|unchecked)</b>\n
        A checkbox with given label LABEL which is initially checked if 2nd argument is 'checked'
      - <b>label(string TEXT="")</b>\n
        Each label component can be used to show dynamic as well as static content. A label can be
        initialized with a given string (or an int/float as string). Later on this content can be
        changed by accessing this label from outside the GUI object.
      - <b>slider(int MIN,int MAX,int CURRENT[,vertical])</b>\n
        A slider is created by a given range {MIN,...,MAX} and a given initial value CURRENT. If the 4th 
        parameter is 'vertical', the slider is layouted in vertical direction. However, this argument is
        optional.
      - <b>fslider(float MIN,float MAX,float CURRENT[,vertical])</b>
        A fslider is created by a given range {MIN,...,MAX} and a given initial value CURRENT. If the 4th 
        parameter is 'vertical', the slider is layouted in vertical direction. However, this argument is
        optional.
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
      - <b>fps(int TIME_WINDOW_SIZE=10)</b>\n
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
      - <b>camcfg(deviceType="",deviceID="")
        You may use either two parameters (e.g. "camcfg(dc,0)" for the first dc-device) or no parameters.
        See section \ref CAMCFG for more details
      - <b>config(MODE)</b> MODE has to be either 'embedded' or 'popup' (whithout the '-ticks). If param is embedded, the
        ConfigFileGUI's tree-view (including some additional buttons for loading and saving configuration files)
        will be embedded directly where the component was put into the layout. Otherwise - if param is
        popup - only a single button labled 'config' is embedded. This button can be triggered to show
        an extra widget of type ConfigFileGUI

      \subsection GP General Parameters
      The 3rd part of the GUI definition string is a list of so called general params. "General" means here,
      that these params are available for all components, whereas some of these parameters must be compatible
      to the corresponding component.\n
      Each entry begins with the "@"-character and it ends
      with the following "@"-character or the closing angular bracket "]" at the end. 
      The parameters have the syntax PARAM_NAME=VALUE. Possible general parameters names are:
      - <b>\@label=string</b> adds a (bordered) label to the component
      - <b>\@out=output-ID</b> defines the ID which can be used to access the components output-value
      - <b>\@inp=input-ID</b> currently not used
      - <b>\@handle=handle-ID</b> defines the ID which can be used to access the component handle
      - <b>\@minsize=size</b> defines the mininum size (in 20x20-cells) for this component 
      - <b>\@maxsize=size</b> defines the maxinum size (in 20x20-cells) for this component
      - <b>\@size=size</b> defines the initial size (in 20x20-cells) of this component
      - <b>\@margin=int</b> defines the pixel margin around contained component (only for container 
        components)
      - <b>\@spacin=int</b> pixel-spacing between components (only for container components)

    <tiny>
    To decide which value/handle should be accessed by that call, each input/and output of a GUI 
    component is associated internally with a string-identifier. In addition each component can be
    controlled by a so called GUIHandle object which is implemented for each provided GUI component.
    By default, a gui component "xyz" provides a Handle class "XyzHandle". This handle is also allocated
    in the top-level GUI objects Data-Store, if the "@handle=NAME" token was defined in the definition
    string. The handles "NAME" is used as string identifier for the GUI-DataStore. (see the next 
    subsection for more details and some examples)
    </tiny>

    \subsection IO Output-Interface
    Dependend on the type of a GUI component, the top-level GUI instance gets additional outputs.
    If a gui-component is defined with an optinal parameters \@out=NAME, it automaticall creates a mutex-locked variable
    (inside of an interal GUI data base which is always automatically created by its top level GUI component), 
    and updates this variable each time a Qt-GUI-Event on this components occurs. The NAME of this output must
    be unique for all components that have a common top-level GUI instance. It is used to reference the particular
    output field in you applicaton. Consider the following example:

      \code
      #include <ICLQt/GUI.h>
      using namespace icl;
      int main(int n, char**ppc){
        QApplication app(n,ppc);

        // create a new slider component with range [0,100], initialized to 50
        // we name its output "the slider value"
        GUI g("slider(0,100,50)[@out=the slider value]");

        // Now, g is just a GUI-Definition - no QSlider was created internally and no
        // no slider data can be accessed, yet

        // make g visible, This will create all embedded Qt-Widgets internally and
        // it will allocate data for each named components output in the GUIs 
        // internal DataStore.
        g.show();
      
        // now a single integer value is allocated, which is assigned to the 
        // slider value, and which is updated immediately when the slider is moved.
        // The GUI's getValue<T>() template function can now be used to get a reference
        // (or a pointer) to this slider value
        int &val = g.getValue<int>("the slider value");
      
        // or
        int *pVal = &g.getValue<int>("the slider value");

        return app.exec();
      }
      \endcode

      Currently, most componets provide a single output pin only, however the 
      GUI-definition syntax can be used to define components with N inputs and M outputs.\n
      As each GUI component has different semantics, the count and the type of it's
      in- and output pins must be regarded. The following list shows all GUI components
      and explains their individual in- and output interface 
      
      <TABLE>
      <TR> <TD><b>type</b></TD>  <TD><b>handle</b></TD>     <TD><b>outputs</b></TD>     <TD><b>meaning</b></TD>                                      </TR>
      <TR> <TD>vbox</TD>         <TD>BoxHandle</TD>         <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>hbox</TD>         <TD>BoxHandle</TD>         <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>vscroll</TD>      <TD>BoxHandle</TD>         <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>hscroll</TD>      <TD>BoxHandle</TD>         <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>tab</TD>          <TD>TabHandle</TD>         <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>hsplit</TD>       <TD>SplitterHandle</TD>    <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>vsplit</TD>       <TD>SplitterHandle</TD>    <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>border</TD>       <TD>BorderHandle</TD>      <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>button</TD>       <TD>ButtonHandle</TD>      <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>buttongroup</TD>  <TD>ButtonGroupHandle</TD> <TD>1 type int</TD>         <TD>index of the currently toggled radio button </TD>        </TR>
      <TR> <TD>togglebutton</TD> <TD>ButtonHandle</TD>      <TD>1 type bool</TD>        <TD>true=toggled, false=untoggled</TD>                       </TR>
      <TR> <TD>checkbox</TD>     <TD>CheckBoxHandle</TD>    <TD>1 type bool</TD>        <TD>true=checked, false=unchecked</TD>                       </TR>
      <TR> <TD>label</TD>        <TD>LabelHandle</TD>       <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>slider</TD>       <TD>SliderHandle</TD>      <TD>1 type int</TD>         <TD>current slider value</TD>                                </TR>
      <TR> <TD>fslider</TD>      <TD>FSliderHandle</TD>     <TD>1 type float</TD>       <TD>current slider value</TD>                                </TR>
      <TR> <TD>int</TD>          <TD>IntHandle</TD>         <TD>1 type int</TD>         <TD>current text input field content</TD>                    </TR>
      <TR> <TD>float</TD>        <TD>FloatHandle</TD>       <TD>1 type float</TD>       <TD>current text input field content</TD>                    </TR>
      <TR> <TD>string</TD>       <TD>StringHandle</TD>      <TD>1 type std::string</TD> <TD>current text input field content</TD>                    </TR>
      <TR> <TD>disp</TD>         <TD>DispHandle</TD>        <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>image</TD>        <TD>ImageHandle</TD>       <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>draw</TD>         <TD>DrawHandle</TD>        <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>draw3D</TD>       <TD>DrawHandle<3D/TD>      <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>combo</TD>        <TD>ComboHandle</TD>       <TD>1 type std::string</TD> <TD>current selected item <b>don't use this, use the handle instead!</b></TD></TR>
      <TR> <TD>spinner</TD>      <TD>SpinnerHandle</TD>     <TD>1 type int</TD>         <TD>current value</TD>                                       </TR>
      <TR> <TD>fps</TD>          <TD>FPSHandle</TD>         <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>multidraw</TD>    <TD>MultiDrawHandle</TD>   <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>camcfg</TD>       <TD>no handle</TD>         <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      <TR> <TD>config</TD>       <TD>no handle</TD>         <TD>0</TD>                  <TD>-</TD>                                                   </TR>
      </TABLE>
      
      \section HVV Handles vs. Values

      
      In some cases accessing a components value is not enough. E.g. if a "label" component
      should be used to show another string, or a slider should be set externally to a
      specific value. \n
      To facilitate working with GUI objects, each component is able to allocate a so called
      "handle"-object of itself in its parent GUI-objects data store. This will be done
      if a "@handle=.." token is found in the general params list of this component definition
      (general params are inside angular brackets).\n
      All handle classes are derived from the GUIHandle<T> template class, which provides
      functionalities for storing a T-pointer (template parameter) and which offers functions
      to access this pointer (the *operator and the '->' operator). A ButtonHandle for example inherits
      the GUIHandle<QPushButton> i.e., it wraps a QPushButton* internally. For a more
      convenient use, each handle has some special functions that provide
      an abstracted direct access to the underlying class without knowing it. E.g. the image handle
      provides a 'setImage(ImgBase*)'-function and an 'update()'-function, which are sufficient to 
      make the underlying widget display a new image. All functions of the wrapped Qt-data type
      can also be accessed directly using the operator '->'
      
      \code
      #include <ICLQuick/Common.h>
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
    
        // the same as
        // h->setValue(7);


        // enter qts event loop
        return app.exec();
      }

      \endcode
      
      The following subsection shows some examples for different GUIHandles.
      
      \subsection LH LabelHandles
      
      Labels can be used to show strings, integers and floats
      
      \code
      
      #include <ICLQuick/Common.h>
      
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
      #include <ICLQt/GUI.h>
      
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
      which is used for the "button" type element. The button itself produces no data; it can
      only be accessed by using its handle. In contrast to other components, simple buttons
      are producing an event instead of some data. When accessing the button from the working
      thread, you don't need the information if the button is pressed at this time, but you
      may want to know if it <em>was</em> pressed since the last test. You might say that a simple boolean
      variable would be sufficient to handle this information, but the following stays doubtful:
      "Who resets this boolean variable, and when?". To avoid this problem the ButtonHandle
      data type, can be triggered (if the button is pressed) an it can be checked using its 
      wasTriggered() function, which returns whether the event was triggered and resets the internal
      boolean variable to false. The following example code illustrates this (Here also the <b>ICLApplication</b>
      class is used. This class facilitates implementation of multithreaded GUI application significantly) :

      \code
      #include <ICLQuick/Common.h>

      /// Use a static gui instance
      GUI gui;

      // initialization function (passed to ICLApplication)
      void init(){
        gui << "button(Click me!)[@handle=b]";
        gui.show();
      }

      // 1st threaded function (passed to ICLApplication)
      void run(){
        // equivalent to ButtonHandle b = gui.getValue<ButtonHandle>("b")
        gui_ButtonHandle(b); 
        if(b.wasTriggered()){
          printf("button was triggered! \n");
        }
        Thread::sleep(1.0);
      }
      
      int main(int n, char**ppc){
        return ICLApplication(n,ppc,"",init,run).exec();
      }
      \endcode
      
      \image html Image06_ButtonDemo.jpg
      
      \subsubsection EAB Registering callbacks at GUI-Components
      
      In some applications it might be necessary to associate an event to a button click,
      which is called immediately if the button is clicked, or either when another GUI interaction
      is performed. This is quite useful e.g. to interrupt the current working thread. 
      Actually, this feature is a "must-have" and it is integrated deeply into the GUI's structure.\n
      We use the special class 'GUI::CallbackPtr' as event type. This callbacks can be registered at
      most of the GUI components, and as long as such a callback is not unregistered the underlying function
      is called whenever the corresponding component receives certain mouse events. Furthermore all top-level GUI's provide
      the ability to register a given callback to all child widgets recursively.
      Callbacks can be registered at handles as well as at subclasses of icl::GUIWidget.

      
      The following example can also be found as ICLQt/examples/gui-callback-test.cpp
      \code

#include <ICLQuick/Common.h>

/// global gui instance
GUI gui;

// Our working thread, calling it's run function 
// asynchronously to the GUI Thread
void run(){
  // shortcut to extract currentTimeLabel from the gui
  gui_LabelHandle(currentTimeLabel);
  currentTimeLabel = Time::now().toString();
  Thread::sleep(1);
}

// a simple callback function of type "void (*callback)(void)"
void exit_callback(void){
  printf("exit callback was called \n");
  exit(0);
}

// another one (we can also access the GUI from here)
void click_callback(){
  gui_LabelHandle(currentTimeLabel);
  currentTimeLabel = "hello!";
}

// a more complex callback implementing the GUI::Callback interface
// In contrast to simple functions, this callbacks are able to have 'own data'
struct MyCallback : public GUI::Callback{
  Time m_lastTime;
  virtual void exec(){
    Time now = Time::now();
    Time dt = now-m_lastTime;

    // here we could use the macro gui_LabelHandle(timeDiffLabel) as well
    gui.getValue<LabelHandle>("timeDiffLabel") = str(dt.toSecondsDouble())+" sec";
    m_lastTime = now;
  }
};

void init(){
  // create some nice components
  gui << "label(something)[@handle=currentTimeLabel@label=current time]"
      << "label(something)[@handle=timeDiffLabel@label=time since last call]"
      << "button(Click me!)[@handle=click]"
      << "button(Click me too!)[@handle=click-2]"
      << "button(Exit!)[@handle=exit]";
  
  
  // create and show the GUI
  gui.show();
  
  /// sometimes, this works as well !
  gui["currentTimeLabel"] = Time::now().toString();
  
  // register callbacks (on the specific handles)
  gui.getValue<ButtonHandle>("exit").registerCallback(new GUI::Callback(exit_callback));
  gui.getValue<ButtonHandle>("click").registerCallback(new GUI::Callback(click_callback));

  // or let gui find the corresponding components internally
  gui.registerCallback(new MyCallback,"click-2");
  
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
    \endcode


      \subsubsection CCB Complex Callbacks 
      
      In addition to the simple callback structure using an empty exec() function, callbacks
      may use another function type (GUI::Callback::complex_callback_function). Here, the
      exec-function gets a string argument, which is filled with the source components handle
      name at runtime. By this means, it is possible to use a single callback function ( or
      of course a special implementation of the GUI::Callback interface) to handle a set of
      GUI components' events simultaneously at one code location. The following example, which
      is also available as icl-complex-gui-callback-test demonstrates use of this 
      mechanism:

      \code
#include <ICLQuick/Common.h>

GUI gui;

void handle_event(const std::string &handle){
  DEBUG_LOG("component " << handle << " was triggered!" );
}

void init(){
  gui << "slider(0,100,50)[@handle=slider@label=slider@out=_1]"
      << "togglebutton(off,on)[@handle=togglebutton@label=toggle button@out=_2]"
      << "button(click me)[@handle=button@label=a button]";
  gui.show();
  
  gui.registerCallback(new GUI::Callback(handle_event),"slider,button,togglebutton");
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init).exec();
}
      \endcode

      \subsubsection INDEX_OP GUI instances and the []-index operator (Meta-Handles)
    
      For even more convenience the GUI's parent class DataStore does also provide an implementation of the 
      index operator "[]". The index operator can be called with the entries string-ID in order to get a
      so called <b>meta-handle</b> for this entry of type DataStore::Data. This class provides intelligent implicit-cast-  and
      assignment-operators. These operators can be used to produce much prettier code:

      \code
      
      GUI gui;
      gui << "..." << "...";
      gui.show();
      // instead of writing
      gui.getValue<ImageHandle>("image")->setImage(myImage);
      gui.getValue<ImageHandle>("image")->updateFromOtherThread();
    
      // you can simply write
      gui["image"] = myImage;
      gui["image"].update();
    
    
      // this:
      int val = gui.getValue<float>("mySlider");
    
      // you can write this:
      int val = gui["mySlider"]
    
      // or also 
      ImageHandle h = gui["image"];
    
      \endcode
    
      This implicit cast/assignment mechanism works for many pairs of lvalue/rvalue types (note: "number values" are all
      common float and integer data types).
    
      - all handles and values can be accessed directly using 
        \code  type t = gui["id"]; \endcode
      - combo-handles can be assigned to string values (extracts selected item)
        and to number values (extracts current index) e.g.,  
        \code  string mode = gui["mode"]; \endcode
      - int- and float handles can be assigned to number-values and to strings e.g., 
        \code  int threshold = gui["thresh"]; \endcode
      - all image types and pointers of these can be assigned to image-, draw- and draw3D handles e.g., 
        \code  gui["image"] = myImage; \endcode
      - number-type = slider or spinner meta handle is supported and string = slider or spinner meta handle
      - slider or spinner = Range8u|32s|32f sets the range of the underlying component
      - button-group = number-type sets the current selected item of a button group
      - number-type = button-group gets the current selected item of a button group
      - label = string shows the string on the label
      - label = image|image-pointer (shows an image description on the label)
      - string = label extracts the current text of a label
      - num-type = button extracts whether the button is currently toggled
      - num-type|string = string-meta-handle extracts/parses the current input
      - the function registerCallback can be called on all meta-handles in order to register a GUI::Callback*
      - the function update() can be called on image-, draw- and draw3D meta handles. This function calls
        the internal components updateFromOtherThread()-method
      - the function install() can can also be called on these meta handles. It installs a MouseHandler*
        (passed as anonymous void*)
      - the function update() updates an fps-meta handle


      \subsubsection ABCDE Qt-Dialogs

      Please note, that some special Qt-Functionality just becomes usable with this callback
      mechanism, namely showing Dialogs. Unfortunately, Qt-Dialogs can only be invoked to
      become visible from the GUI-Thread itself. Hence if you try to call e.g. 
      QInputDialog::getDouble(...) from your working thread, your application will get stuck
      or even crash with some async-error message. 
      Therefore QDialogs must be created/shown using callbacks.

      
      
      \subsection EMB Embedding external QWidgets
      In some cases it might be necessary to embed QWidgets, which are not supported by the GUI-API.
      For this, the two box ("hbox","vbox", "hsplit", "vsplit", and "tab") also provide a special BoxHandle which
      wraps the underlying QWidget to provide access to it and its current layout. See the following 
      example for details:

      \code
      #include <ICLQuick/Common.h>
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
      #include <ICLQt/GUI.h>
      
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


      \subsection TABS Tabbed and Split Widgets
      One of the newest features are tabbed (and split) container widgets. Tabbed Widgets
      can be created with a comma separated string list containing tab labels; split-Widgets 
      have no parameters -- just like the other box container widgets. Each time a new child-
      component is added, it is added into the next free tab (or split section). If more 
      tabs are added than names
      have been given, new tabs are added with dummy names, but an error will be shown. 
      Tab- and  Split-widgets 
      are as easy to use as all the other ICL gui widgets, they widgets can again contain other
      container widgets, and of course also external/foreign QWidgets can be added directly using the 
      widget's Handle of type TabHandle (resp. SplitterHandle). These Handles provides an 'add'-function as well
      as an 'insert'-function to insert another component at a certain tab index/split slice. For more
      complex manipulation of the wrapped QTabWidget/QSplitter, it can be obtained conveniently by 
      'dereferencing' the Handle instance. (e.g. TabHandle &h = ...; QTabWidget *w=*h;)
      
      Here's an example for using tabs (available as gui-test-2.cpp):
      
      \code 
#include <ICLQuick/Common.h>
#include <QProgressBar>

GUI gui;

void init(){
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
}


void run(){
  static Img8u image = cvt8u(scale(create("parrot"),0.2));
  gui_ButtonHandle(click);
  
  for(int i=0;i<3;++i){
    gui["image"+str(i+1)] = image;
    gui["image"+str(i+1)].update();
  }
  if(click.wasTriggered()){
    std::cout << "button 'click' was triggered!" << std::endl;
  }
  Thread::msleep(50);
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
     \endcode

      \image html Image08_Tabs.jpg

      \subsection CAMCFG Camera Configuration
      In some camera based applications it might be helpful or even necessary to adapt camera 
      parameters at run-time. In this case, the camcfg component can be used. Simply add an
      embedded GUI component named camcfg, which gets a hint about which camera should be 
      configurable on that component. As the underlying CamCfgWidget's GUI is quite complex,
      it is shown in an extra window, therefore only a single button labeled "camcfg" is embedded
      into the GUI.\n
      You can pass either two or no parameters to the "camcfg()"-component. If two parameters are
      used, these are passed directly to the init-method of the underlying GenericGrabber. Therefore
      the parameters can be something like "camcfg(dc,0)" (e.g. for the first dc-device).  If you use
      ICL's default camera input specifier (via program argument -input device-type devide-ID), 
      you will have to the camcfg-component like "camcfg("+pa("-i",0)+","+pa("-i",1)+")". However, 
      in most cases, it might be sufficient to create the "camcfg()" component with no parameters.
      In this case, the component pop-up CamCfgWidget is set up for configuring all <b>already used</b>
      camera devices. Devices are <em>already used</em>, if a GenericGrabber instance for this device
      exists.
    
    \image html xcfpub.png "pop upped CamCfgWidget (demo application xcf-publisher)"
    
    
      \subsection CONFIG Embedded or Popup'ed ConfigFileGUI's
      Another recent component is the 'config' GUI component. It provides a mechanism to change
      configuration file entries at runtime. This functionality is encapsulated within the 
      ConfigFileGUI class (see documentation for further details). The 'config'-GUI component can
      be used in two ways:
      -# As a single button that occurs embedded and which can be triggered to show an extra widget
         of type ConfigFileGUI
      -# As an embedded complex tree-view with some additional buttons for loading and saving
         the current content of the configuration as xml-file

      Please note, that there is no possibility to pass a custom configuration file to the GUI component
      except the current static ConfigFile accessible as ConfigFile::getConfig(). Please ensure,
      that your configuration file is already loaded into the static ConfigFile instance, when you
      create your GUI by calling create() or show().
      
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
      to work with this reference later on.
  */

#endif
