.. include:: ../js.rst

.. _qt:


########################################
GUI Creation and Visualization Framework
########################################

.. image:: /icons/185px/qt.png



As it's core, this package provides a wrapper API for the development
of Qt-based GUI applications. The main class of this package is the
:icl:`qt::GUI`, which is a general container component. In addition to
wrapper components for all common *Qt-Widgets*, such as sliders,
buttons and check-boxes, we provide a set of highly optimized image
and data visualization *Widgets*. We will here give a complete but
concise overview of ICL's GUI creation framework. Step by step
examples are given in the tutorial chapters (:ref:`tut.gui` and
:ref:`tut.interactive-gui-apps`)

In additions, the **ICLQt** modules provides :ref:`qt.quick`
for rapid prototyping.


.. _qt.gui-creation-framework:

The **GUI** Creation Framework
------------------------------

* :ref:`qt.gui-creation-framework-intro`
* :ref:`qt.gui-first-steps`

  * :ref:`qt.gui-creation-framework-simple-slider`
  * :ref:`qt.gui-creation-framework-iclapp`
  * :ref:`qt.gui-creation-framework-defexpr`
  * :ref:`qt.gui-creation-framework-hierarchical`
  * :ref:`qt.gui-creation-framework-dynamic`
  * :ref:`qt.gui-creation-framework-remote`

* :ref:`qt.gui-creation-framework-accessinggui`

  * :ref:`qt.gui-creation-framework-handles`
  * :ref:`qt.gui-creation-framework-index`
  * :ref:`qt.gui-index-operator-benchmark`
  * :ref:`qt.callbacks`

* :ref:`qt.component-table`

  * :ref:`qt.common-controls`
  * :ref:`qt.image-and-vis`
  * :ref:`qt.complex-components`
  * :ref:`qt.other-components`
  * :ref:`qt.container-components`

* :ref:`qt.special-components`
* :ref:`qt.image-vis-framework`

  * :ref:`qt.glimg`            
  * :ref:`qt.iclwidget`      
  * :ref:`qt.icldrawwidget`  
  * :ref:`qt.icldrawwidget3d` 

* :ref:`qt.mouse-handlers`


There are most different approaches for the creation of graphical user
interfaces -- short GUI's. ICL follows the idea of the Qt-library,
that uses a object oriented class tree whose nodes describe
GUI-interfaces and classes. Actually, ICL's GUI creation framework is
a shallow wrapper around the Qt library, that always preserves
accessibility to its internally used Qt *Widgets*.

We basically distinguish between two different types of GUI
components: *normal components*, represented by the
:icl:`qt::GUIComponent` class interface and *container components*,
represented by the :icl:`qt::ContainerGUIComponent` class
interface. Each container type provides a special layout for its
contained components, which can either be normal- or other container
components. By these means it allows for creating very complex and
hierarchical GUI layouts.


.. _qt.gui-creation-framework-intro:

Introduction
^^^^^^^^^^^^

**A fundamental question could be:**

  *"Why would one want to use a Qt-Wrapper rather than to use the well
  known and user friendly Qt-library directly?"*

**The answer is:**

  *"Because ICL's GUI creation framework allows for creating very
  complex GUIs with very little code, so that you can completely
  concentrate on the implementation of your computer vision stuff"*


**Here is a list of the advantages of ICL's GUI creation framework:**

  * very concise yet intuitive syntax (allows for writing *beautiful* code)
  * the most common features of all common GUI components are directly supported
  * for non-Qt experts: much simpler to learn and to use
  * neither Qt-*signals* and *-slots* nor the Qt-meta compiler (moc) is needed any more
  * solves all GUI-thread -- working-thread synchronization issues for you
  * provides implicit and standardized layout handling
  * always provides access to the wrapper Qt-components
  

During the implementation of computer vision applications, the
programmer does usually not want to spend many resources for the
implementation of the GUI-based interactive features of it, calling
for a powerful, yet easy to use GUI creation toolbox. In particular
image visualization, but also common GUI components, such as sliders,
buttons and check-boxes, should be creatable in a simple manner and
their current states have to be accessible in an easy way. Another
important factor for computer-vision applications, that in general
produce a high processor usage, is the decoupling of the GUI and the
working loop, which is usually implemented by using at least two
threads: a GUI thread, and one or more working threads.

Of course, this can be implemented using the powerful Qt-framework, 
however, there are many issues, that have to be solved manually

* The GUI-thread (Qt-event loop) and the working thread must be
  synchronized
* User interactions must be handled using Qt's signal and slot
  connections (here sometimes also the Qt-meta compiler needs to
  be used)
* Complex GUIs require complex layouts to be created, and whoever
  tried to rearrange certain Qt-components within complex GUI by
  adapting layouts and size-constraints and -policies knows, that
  this can be a difficult and time consuming task


  
.. _qt.gui-first-steps:

First Steps
^^^^^^^^^^^

Let's get a first glance at how things are connected. A more detail
impression can be obtained by working though the :ref:`tutorial<tut>`.

.. _qt.gui-creation-framework-simple-slider:

A Simple Slider
"""""""""""""""

A Qt-expert might thing *"So what?"*, because he can create a slider
with layout in 1 minute with only 5 lines of code. ICL's GUI creation
framework also endows non-Qt-experts with the ability to create and
layout a slider using a single line of code.


+----------------------------------------------+-----------------------------------+  
| .. literalinclude:: examples/qt-slider.cpp   | .. image:: images/qt-slider.png   |
|    :linenos:                                 |      :alt: shadow                 |
|    :language: c++                            |                                   |
+----------------------------------------------+-----------------------------------+  


.. _qt.gui-creation-framework-iclapp:

The **ICLApplication** class
""""""""""""""""""""""""""""

As we will seen in the following examples, the :icl:`ICLApplication`
(typedef'd to :icl:`ICLApp`), is a very central component of interactive
ICL applications. Usually, it is instantiated with a given
initialization and working-thread function pointer. The latter one is
not used and therefore left out in the example above. Its
:icl:`ICLApplication::exec`-method performs the following steps:

1. instantiating a *singelton* **QApplication** instance
2. parsing optionally given program arguments (see :ref:`utils.pa`)
3. calling the initialization method in the applications main thread
4. creating a working thread for each given *run-function-pointer*
5. start the thread, which loops this function
6. entering the Qt event loop


.. _qt.gui-creation-framework-defexpr:


**GUI** Definition Expressions
""""""""""""""""""""""""""""""

An expression that defines a GUI usually consists of several
stream-operator-based statements, each defining a single
GUI-component. Each component provides one or more constructors the
get component-related parameters, such as the minimum, maximum and
initial value of a slider. Additionally, a set of general options can
be added::
  
  gui << Slider(0,100,255).handle("slider").label(threshold).maxSize(100,2);

**handle(string)**

  defines the component's ID, which can later be used to access the GUI
  component to either read-out its current value, or to adapt its state

**label(string)**

  defines a component label, which is used to create a labeled border
  around the GUI component. 

**tooltip(string)**

  adds a component tooltip (this feature is not supported for container
  components

**minSize(xCells,yCells)**

  defines the minimum size of the component, given in units of 20 pixels

**maxSize(xCells,yCells)**

  defines the minimum size of the component, given in units of 20
  pixels. In combination with **minSize**, complex size-constraints
  can be created

**size(xCells,yCells)**

  defines the initial size of the component
  
**out(string)**

  this feature only provides for some of the components (see
  :ref:`qt.component-table`). It makes the component also register a
  single **int** or **float** variable, liked to the component value,
  which can be extracted by reference. Usually, declaring a **handle**
  is more recommended.

**hideIf(bool)**

  this is a very special options, that can be used to conditionally
  create GUI-components. If the boolean parameter is true, the
  component gets a dummy-status and is not translated into an actual
  Qt component at all

**margin(int)**

  This is only supported for container components and defines
  the layout's outer margin

**spacing(int)**

  This is only supported for container components and defines
  the layout's component spacing

.. _qt.gui-creation-framework-hierarchical:

Complex Hierarchical **GUI**\ s
"""""""""""""""""""""""""""""""

Using a single GUI container component does usually lead to
ill-layouted GUIs. In order to avoid this, ICL's GUI creation
framework allows for nesting hierarchical GUI creation expressions,
leading to advanced GUI layouts. A very common layout for simple
interactive image processing applications is to have a main image view
plus a simple vertical list of controls, which is usually at the right
hand side of the main window. In this case we recommend to use e.g. a
horizontal splitting component that contains the image view on the
left and a vertically layouted container containing the controls on
the right. The *control panel* can then again be layouted using
nested sub-containers:

+---------------------------------------------------+----------------------------------------+  
| .. literalinclude:: examples/qt-hierarchical.cpp  | .. image:: images/qt-hierarchical.png  |
|    :linenos:                                      |    :scale: 50%                         |
|    :language: c++                                 |    :alt: shadow                        |
|                                                   |                                        |
|                                                   | started with **appname -input create   |
|                                                   | mandril**                              |  
+---------------------------------------------------+----------------------------------------+


.. _qt.gui-creation-framework-dynamic:


Dynamic XML-based GUI Description
"""""""""""""""""""""""""""""""""

In order to separate the GUI layout from the actual implementation,
the :icl:`DynamicGUI` class can be used. This class enables the
programmer to define the GUI layout in a separate XML-file that is
then parsed by the application at runtime. The advantage of this
approach is that the adaption of GUI parameters or the adding of new
GUI components as well as re-arrangement of the GUI components no
longer requires to re-compile the application. In contrast to this,
the GUI definition syntax is no longer parsed and evaluated by the
compiler, which means that the development cycle is not neccessarily
shortened.

.. _qt.gui-creation-framework-remote:


RSB-based Remote GUIs
"""""""""""""""""""""

With the :icl:`qt::RSBRemoteGUI`, an additional separation of the GUI
and the actual application code can be reached. This class can wrapped
around an existing gui. It then analyses the wrapped GUI and provides
setter and getter RSB-based network interfaces for each of the GUI
components. By these means, ICL's GUI-creation framework can more easily be
used in a decoupled fashion. 

The ICL-application **icl-remote-gui** bundles this functionality by being able
to load a GUI-xml-file and then to automatically create the corresponding
RSB-interfaces for it

.. _qt.gui-creation-framework-accessinggui:


Accessing the GUI from the Working Thread
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As soon as our application needs a working loop, e.g. for image
processing, a **run** method can be passed to the :icl:`ICLApp`
constructor as well. The top-level GUI component -- usually
represented by a global **GUI** instance -- provides thread-safe
access to all contained component states. Please note, that :ICL:`GUI`
instances must be created by either streaming the special
:icl:`qt::Show` or :icl:`qt::Create` component into it or by calling
its :icl:`GUI::create` or :icl:`GUI::show` method. Before this is
done, only a hierarchical description of the GUI exists, but not
underlying Qt-components. GUI components can be access using the GUI's
index operator :icl:`GUI::operator[](const std::string &key)`, where
**key** refers to a component's *handle* or *output* ID, which is
given by the **.handle("handle-name")** method in the GUI-definition
expression.

+----------------------------------------------+-----------------------------------+  
| .. literalinclude:: examples/qt-access.cpp   | .. image:: images/qt-access.png   |
|    :linenos:                                 |      :alt: shadow                 |
|    :language: c++                            |                                   |
|    :emphasize-lines: 9,10,17,20              |                                   |
+----------------------------------------------+-----------------------------------+  

.. _qt.gui-creation-framework-handles:

Handles vs Outputs
""""""""""""""""""

For a some of the components (see :ref:`qt.component-table`) also an
*output* is provided. E.g. a slider component can be set up with an
output ID, which can then later be used for extracting an
*int*-reference, which is directly linked to the
slider-value. Accessing this *int*-reference is usually a little bit
faster than using the :ICL:`GUI`'s index operator, which has to perform a
map-lookup internally. Actually, a
:ref:`benchmark<qt.gui-index-operator-benchmark>` showed, that the
performance loss is completely negligible for 99.9% of all
applications developed. Therefore, we strongly recommend to use the
**GUI**'s index operator for accessing GUI-components.

.. literalinclude:: examples/qt-access-out.cpp
   :linenos:
   :language: c++
   :emphasize-lines: 5,12,14

.. _qt.gui-creation-framework-index:

The **GUI**'s Index Operator
""""""""""""""""""""""""""""

The index operator uses a smart utility class of type
:icl:`qt::DataStore::Data`, which allows for direct and internally
type-checked assignment. For this, the :icl:`qt::DataStore::Data`
instance returned by the index operator provides template-based
assignment and implicit cast operators::
  
  template<class T> void operator=(const T &t) ;

  template<class T> operator T() const ;
  
Internally, contained GUI entries, such as a slider value, are
registered with an RTTI-type, which is checked whenever one of these
operators is used. For each pair of *assignable* types, an
*assign*-method is registered internally. For transparency reasons,
the application **icl-gui-assignment-info** is provided, which is able
to list all supported assignment-rules, each defined by a source
(*rvalue*) and destination (*lvalue*) type, e.g.::
  
   icl-gui-assignment-info -s int

lists all assignment rules, that have an **int**-typed lvalue. The
source instance can be either part of the GUI, such as a slider-value
or an external integer variable that is assigned to an GUI component.
Here are some typical examples:

.. literalinclude:: examples/qt-gui-assignment.cpp
   :language: c++
   :linenos:

  
.. note::
   
   In a few cases, C++ cannot automatically infer the desired lvalue
   type of an assignment expression. For these cases the explicit
   template-based **as<T>** (:icl:`DataStore::Data::as`) method is provided::
     
     gui << Slider(0,100,50).handle("test") << Show();
     
     int i = gui["test"]; // works
     int j = 0;
     j = gui["test"]; // error: ambiguous assignment
     j = gui["test"].as<int>(); // works
  

  

  

.. _qt.gui-index-operator-benchmark:

Performance of the GUI-Index Operator
"""""""""""""""""""""""""""""""""""""

As mentioned above, the GUI index operator needs to perform a
map-lookup in order to locate the value of an actual component. The
following benchmark however proofs the negligibility of the time
consumption used for the index operator in common applications.

**System**

  2,4 GHz Intel Core2Duo, 4 GB Ram, Ubuntu 12.04 32 bit

**Setup**

  We used a complex GUI containing 10, 30, 50 and even 100
  GUI-components, which is usually much more then one would expect in
  a real-world example. The GUIs internal map contains twice as may
  entries, one for each component handle and one for the corresponding
  output. We also used extraordinary long output-names, build from
  random strings of length 30. For measurement accuracy, we took the
  time for 1000 uses of the index operator

**Results**

  ========================  ======  ========  ========  ========
  Number of Components:       10       30        50       100
  ========================  ======  ========  ========  ========
  Times for 1000 accesses   0.30ms   0.32ms    0.35ms    0.37ms 
  ========================  ======  ========  ========  ========

**Evaluation**

  As one can easily see, the used **std::map** is very fast. The
  performed benchmark also demonstrates the logarithmic search
  behavior of the map. In real-world applications, the time
  consumption all uses of the **GUI**'s index operator in total will
  never take more than a 10th of a millisecond -- usually even
  magnitudes less.

.. _qt.callbacks:

Callback Registration
"""""""""""""""""""""

Most GUI components support the ability to register callbacks to them
that are executed immediately when the component state is
changed. *Immediately* means that the callback is executed **in the
GUI-thread**. This is particularly necessary when the callback needs
to do Qt-GUI stuff, such as showing a dialog. There are two types of
possible callbacks, simple ones of type::
  
  utils::Function<void,void> 
  
  e.g. void myCallback(){...}

, or so called *complex* ones ::
  
  utils::Function<void,const std::string&>
  
  e.g. void myComplexCallback(const std::string &)

Simple callbacks are just executed, however, if registered to more
than one component, it is not possible to identify the source of the
component. The complex callback type has a single **std::string**
argument, that is filled with the source component's handle-name.

Due to the fact that callbacks are executed in the applications
GUI-thread, the results must be explicitly synchronized with the
working thread. Therefore, callbacks should only be used
*economically*. Callbacks can be registered directly to the
return-type of the :ICL:`GUI`'s index operator. The callbacks are always
executed *after* the component state has been changed, the callback
trigger is always the most trivial action, such as *moving a slider* or
*clicking a button*.


+------------------------------------------------+-------------------------------------+  
| .. literalinclude:: examples/qt-callbacks.cpp  | .. image:: images/qt-callbacks.png  |
|    :linenos:                                   |      :alt: shadow                   |
|    :language: c++                              |                                     |
+------------------------------------------------+-------------------------------------+  


.. _qt.component-table:

Overview of all GUI Components
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _qt.common-controls:

Common Controls
"""""""""""""""

+------------------------+-----------------------------------+------------------------+-----------------------------------+---------------------------------+
|  Component             | Corresonding Handle Type          |  Wrapped Qt-Component  | Description                       | Output                          |
+========================+===================================+========================+===================================+=================================+
| :icl:`qt::Combo`       | :icl:`qt::ComboHandle`            | QComboBox              |  combo box                        | --                              |
+------------------------+-----------------------------------+------------------------+-----------------------------------+---------------------------------+
| :icl:`qt::Label`       | :icl:`qt::LabelHandle`            | QLabel                 |  text label                       | --                              |
+------------------------+-----------------------------------+------------------------+-----------------------------------+---------------------------------+
| :icl:`qt::String`      | :icl:`qt::StringHandle`           | QTextInput             | input field for text              | --                              |
+------------------------+-----------------------------------+------------------------+-----------------------------------+---------------------------------+
| :icl:`qt::Float`       | :icl:`qt::FloatHandle`            | QTextInput             | input field for floats            | current value, **float**        |
+------------------------+-----------------------------------+------------------------+-----------------------------------+---------------------------------+
| :icl:`qt::Int`         | :icl:`qt::IntHandle`              | QTextInput             | input field for integers          | current value, **int**          |
+------------------------+-----------------------------------+------------------------+-----------------------------------+---------------------------------+
| :icl:`qt::Button`      | :icl:`qt::ButtonHandle`           | QPushButton            | push- or toggle button            | toggled state, **bool** [#f1]_  |
+------------------------+-----------------------------------+------------------------+-----------------------------------+---------------------------------+
| :icl:`qt::ButtonGroup` | :icl:`qt::ButtonGroupHandle`      | QButtonGroup           | list of exclusive radio buttons   | current selcted intex, **int**  |
+------------------------+-----------------------------------+------------------------+-----------------------------------+---------------------------------+
| :icl:`qt::CheckBox`    | :icl:`qt::CheckBoxHandle`         | QCheckBox              | check box                         | check state, **bool**           |
+------------------------+-----------------------------------+------------------------+-----------------------------------+---------------------------------+
| :icl:`qt::Slider`      | :icl:`qt::SliderHandle`           | QSlider                | simple integer-valued slider      | current value, **int**          |
+------------------------+-----------------------------------+------------------------+-----------------------------------+---------------------------------+
| :icl:`qt::FSlider`     | :icl:`qt::FSliderHandle`          | QSlider (*adapted*)    | float valued slider               | current value, **float**        |
+------------------------+-----------------------------------+------------------------+-----------------------------------+---------------------------------+
| :icl:`qt::Spinner`     | :icl:`qt::SpinnerHandle`          | QSpinBox               | a spin box component              | current vlaue, **int**          |
+------------------------+-----------------------------------+------------------------+-----------------------------------+---------------------------------+

.. [#f1] only for in case of toogle buttons


.. _qt.image-and-vis:

Image and Data Visualization Components
"""""""""""""""""""""""""""""""""""""""


+------------------------+-----------------------------------+----------------------------+-------------------------------------+---------------------------------+
|  Component             | Corresonding Handle Type          |  Wrapped Qt-Component      | Description                         | Output                          |
+========================+===================================+============================+=====================================+=================================+
| :icl:`qt::Image`       | :icl:`qt::ImageHandle`            | :icl:`qt::ICLWidget`       | ICL's image display                 | --                              |
+------------------------+-----------------------------------+----------------------------+-------------------------------------+---------------------------------+
| :icl:`qt::Draw`        | :icl:`qt::DrawHandle`             | :icl:`qt::ICLDrawWidget`   | Image display for image annotation  | --                              |
+------------------------+-----------------------------------+----------------------------+-------------------------------------+---------------------------------+
| :icl:`qt::Draw3D`      | :icl:`qt::DrawHandle3D`           | :icl:`qt::ICLDrawWidget3D` | Image display with 3D overlay       | --                              |
+------------------------+-----------------------------------+----------------------------+-------------------------------------+---------------------------------+
| :icl:`qt::Plot`        | :icl:`qt::PlotHandle`             | :icl:`qt::PlotWidget`      | 2D scatter and function plotting    | --                              |
+------------------------+-----------------------------------+----------------------------+-------------------------------------+---------------------------------+

.. _qt.complex-components:

Complex GUI Components
""""""""""""""""""""""

+------------------------+-----------------------------------+---------------------------+----------------------------------------+--------------------------------------+
|  Component             | Corresonding Handle Type          |  Wrapped Qt-Component     | Description                            | Output                               |
+========================+===================================+===========================+========================================+======================================+
| :icl:`qt::CamCfg`      | --                                | :icl:`qt::CamCfgWidget`   | Button to access camera properties     | --                                   |
+------------------------+-----------------------------------+---------------------------+----------------------------------------+--------------------------------------+
| :icl:`qt::ColorSelect` | :icl:`qt::ColorHandle`            | :icl:`qt::ColorLabel`     | RGB or RGBA color selection            | current color, :icl:`core::Color4D`  |
+------------------------+-----------------------------------+---------------------------+----------------------------------------+--------------------------------------+
| :icl:`qt::Disp`        | :icl:`qt::DispHandle`             | :icl:`qt::LabelMatrix`    | 2D array of float labels               | --                                   |
+------------------------+-----------------------------------+---------------------------+----------------------------------------+--------------------------------------+
| :icl:`qt::Fps`         | :icl:`qt::FPSHandle`              | QLabel (*adapted*)        | shows the application's FPS count      | --                                   |
+------------------------+-----------------------------------+---------------------------+----------------------------------------+--------------------------------------+
| :icl:`qt::Prop`        | --                                | only internally used      | GUI for :icl:`utils::Configurable`\ s  | --                                   |
+------------------------+-----------------------------------+---------------------------+----------------------------------------+--------------------------------------+
| :icl:`qt::Ps`          | --                                | only internally used      | shows process information              | --                                   |
+------------------------+-----------------------------------+---------------------------+----------------------------------------+--------------------------------------+

.. _qt.other-components:

Other GUI Components
""""""""""""""""""""

+---------------------+---------------------------------------------------------------------+
|  Component          | Description                                                         |
+=====================+=====================================================================+
| :icl:`qt::Dummy`    | Non visible dummy instance (not created)                            |
+---------------------+---------------------------------------------------------------------+
| :icl:`qt::Create`   | Finalizes the GUI definition and actually *creates* the GUI [#f2]_  |
+---------------------+---------------------------------------------------------------------+
| :icl:`qt::Show`     | Finalizes the GUI definition and *creates* and shows the GUI [#f2]_ |
+---------------------+---------------------------------------------------------------------+

.. [#f2] 
   
   The actual GUI does not exist, before either :icl:`qt::Create` or
   :icl:`qt::Show` was streamed into it (alternatively the top level
   GUI's :icl:`GUI::create` and :icl:`GUI::show` methods can be
   used). After this, the GUI is created, its definition phase is
   ended and the stream operators :icl:`GUI::operator<<` wont work anymore. Please
   note, that *create* means, that the GUI is created only, while
   *show* means, that the GUI is created and shown. Additional calls
   to **create** will not do anything, a *created* GUI instance can be
   shown using **show**.

.. _qt.container-components:

Container GUI Components
""""""""""""""""""""""""

+---------------------+-----------------------------------+------------------------------------------------------------+
|  Component          | Corresponding Handle Type         | Description                                                |
+=====================+===================================+============================================================+
| :icl:`qt::HBox`     | :icl:`qt::BoxHandle`              | horizontally alligned box layout                           |
+---------------------+-----------------------------------+------------------------------------------------------------+
| :icl:`qt::VBox`     | :icl:`qt::BoxHandle`              | vertically alligned box layout                             |
+---------------------+-----------------------------------+------------------------------------------------------------+
| :icl:`qt::HSplit`   | :icl:`qt::BoxHandle`              | horizonatal splitting component                            |
+---------------------+-----------------------------------+------------------------------------------------------------+
| :icl:`qt::VSplit`   | :icl:`qt::BoxHandle`              | vertical splitting component                               |
+---------------------+-----------------------------------+------------------------------------------------------------+
| :icl:`qt::HScroll`  | :icl:`qt::BoxHandle`              | like a HBox, but within a scrollable area                  |
+---------------------+-----------------------------------+------------------------------------------------------------+
| :icl:`qt::VScroll`  | :icl:`qt::BoxHandle`              | like a VBox, but within a scrollable area                  |
+---------------------+-----------------------------------+------------------------------------------------------------+
| :icl:`qt::Tab`      | :icl:`qt::TabHandle`              | *tab'ed* widget                                            |
+---------------------+-----------------------------------+------------------------------------------------------------+

.. note::
   
   All container components can be filled successively with an
   arbitrary number of components. This is also true for the :icl:`Tab`
   component, however the tab-labels are given to the
   :icl:`Tab`-constructor. Therefore if more components are streamed into
   a :icl:`Tab` container, dummy labels are created

.. _qt.special-components:

Special GUI components
^^^^^^^^^^^^^^^^^^^^^^

Most GUI-components are now sufficiently described. However, some of
the components require some extra knowledge in order to use them
correctly and efficiently. These special components are listed and
explained here.

:icl:`qt::Int`, :icl:`qt::Float` and :icl:`qt::String`

  These components are translated into simple text-input fields. The
  different possible input types are implemented using special
  *Qt-validators*. The components will only allow for writing
  *validated* text. **The input must always be confirmed by pressing
  enter** before, the last valid entry is returned

:icl:`qt::Button`

  The button has two special properties, that need to be
  explained. First, if it's constructor gets two string parameters,
  the created button becomes a toggle-button, that has two states,
  indicated by the two button texts. Only in case of creating a
  toggle-button, an **bool** typed output can be created.  The other
  special property is, how we recommend to process button
  clicks. Here, it is suggested to extract a :icl:`ButtonHandle`
  instance and to use it's :icl:`ButtonHandle::wasTriggered` method
  directly in the working thread, which returns whether the button has
  been clicked between the last call to
  :icl:`ButtonHandle::wasTriggered` and now. An example is given in
  the tutorial chapter :ref:`tut.buttons`

:icl:`qt::CamCfg`

  This camera property configuration component is also very
  special. It can either be instantiated with a given input device
  filter, such as e.g. "dc" and "0", which will allow for configuring
  the first dc camera device only or it can be instantiated without
  any parameters. If no parameters are given, it will automatically
  query a list of currently instantiated :icl:`io::GenericGrabber`
  instances (see :ref:`io.generic-grabber`) and prepare the property
  widget for these instances. However, this does only work if the
  :icl:`io::GenericGrabber` was initialized (using its **init**-method)
  *before* the GUI is actually created.

:icl:`qt::Ps`

  The :icl:`Ps` component creates a small info widget, that visualizes
  current process information. Right now, we use QProgressBars here,
  but we plan to use a real plot that shows the processor and memory
  usage history. In addition, the component shows the applications
  thread count. The additional processor usage consumed by the widget
  itself is usually less than 1%.

:icl:`qt::Prop`

  The **Prop** component also provides a very powerful addition to the
  whole GUI framework. It is tightly integrated with the
  :icl:`utils::Configurable` -interface (see :ref:`utils.configurable`
  and the :ref:`tutorial<tut.configurable>`). The GUI component will
  read out the properties of a :icl:`utils::Configurable` instance
  and then create a complex widget for the real-time adjustment
  of these properties

:icl:`qt::Fps`

  The :icl:`Fps` component is rather simple. Once embedded into a GUI,
  it must be manually updated once in every cycle of the working
  thread::
    
    void init(){
      gui << Fps().handle("fps") << Show();
    }
    void run(){
      gui["fps"].render();
    }

:icl:`qt::Image`, :icl:`qt::Draw` and :icl:`qt::Draw3D`

  These are the main classes for image visualization and annotation. The wrapped
  QWidget-classes are explained in :ref:`qt.image-vis-framework`. Their usage 
  is demonstrated in several tutorials (see :ref:`tut`)
  


.. _qt.image-vis-framework:

The Image Visualization and Annotation Framework
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Issues to be solved by the framework**
   
  * How can images be visualized at all?
  * How can this be done efficiently?
  * How can different image *depths* be handled (and how can this be
    done efficiently)?
  * How must images be scaled and moved to fit optimally into a given
    widget?
  * How can image processing and image visualization be decoupled to
    avoid that the GUI gets stuck if the processing loop needs a lot
    of time for each cycle?
  * How can different threads for processing and visualisation be
    synchronized?
  * How can the user change image visualization parameters (e.g,
    brightness or contrast adjustment)?
  * How can images be annotated in online applications (here again,
    one has to face synchronization issues)?
  * How can the image annotation be abstracted from visualization
    features (e.g., current zoom)?
  * How can 3D objects be drawn into a scene so that it matches it's
    real-world counter part?
  * Can 3D overlay be implemented using OpenGL?


+----------------------------------+--------------------------------------------+
| **ICL's image visualization and  |                                            |
| annotation framework             |                                            |
| essentially consists of these    |                                            |
| four classes:**                  |                                            |
|                                  |                                            |
| * :ref:`qt.glimg`                | .. image:: images/qt-drawing-layers.png    |
| * :ref:`qt.iclwidget`            |      :alt: shadow                          |
| * :ref:`qt.icldrawwidget`        |                                            |
| * :ref:`qt.icldrawwidget3d`      |                                            |
+----------------------------------+--------------------------------------------+

.. _qt.glimg:

:icl:`qt::GLImg`
""""""""""""""""

  At the lowest layer, the :icl:`qt::GLImg` provides an interfaces for
  converting :icl:`core::ImgBase` instances into an OpenGL texture (if
  the image is larger than OpenGL's maximum texture sizes, it has to
  be split into several texture) that can be drawn arbitrarily into an
  OpenGL scene. Internally, the :icl:`qt::GLImg` class is used for
  supporting different image depths. Here, OpenGL's pixel-transfer
  parameters are used for hardware accelerated brightness and contrast
  adjustment. Furthermore, fitting images into the widget viewport can
  simply be performed by the graphics hardware. The :icl:`GLImg` can also
  be used as efficient video texture. In order to reduce the use of
  graphics memory bandwidth, the :icl:`qt::GLImg` class uses a
  *dirty-flag* to determine whether an image texture actually needs to
  be updated.

.. _qt.iclwidget:

:icl:`qt::ICLWidget`
""""""""""""""""""""

  The next layer is implemented by the :icl:`qt::ICLWidget` class, which
  inherits Qt's QGLWidget class for the creation of an embedded OpenGL
  context and viewport. The :icl:`qt::ICLWidget` provides a software
  interface for setting different visualisation parameters as well as
  an embedded user interface for GUI-based adaption of these
  parameters. Furthermore, the :icl:`qt::ICLWidget` provides the simple
  to use method::

    setImage(core::ImgBase*)

  which simply lets it visualize a new image immediately. Internally,
  the image is buffered into a mutex-protected interleaved
  intermediate format, which can more easily be transferred to the
  graphics buffer. Therefore **setImage** can simply be called from
  the application's working thread without any explicit
  synchronization. Once an new image is given, the
  :icl:`qt::ICLWidget` will automatically post a *Qt-update-event* by
  calling the :icl:`ICLWidget::render` method. By these means, the
  internally used OpenGL context is actually re-rendered
  asynchronously in the application's GUI thread.

.. _qt.icldrawwidget:

:icl:`qt::ICLDrawWidget`
""""""""""""""""""""""""

  For image annotation, such as rendering box- or symbol-overlay for
  the visualization of current image processing results, the
  :icl:`qt::ICLDrawWidget` is provided. It works like a *drawing
  state-machine* that automatically synchronized image annotation
  commands with Qt's event loop. Internally, this is achieved by using
  two thread-safe *draw-command-queues*. One of these queues can be
  filled with new draw commands, while the other queue belongs to the
  GUI thread and is rendered. Every time, the parent
  :icl:`qt::ICLWidget` classe's **render**-method is called, the
  queues are swapped, and the queue that is now being filled with new
  commands is automatically cleared. At this point, the
  :icl:`qt::ICLDrawWidget` adapts the behavior of the parent
  :icl:`qt::ICLWidget` class, by not automatically calling **render**
  when a new background image is given. Since usually setting the
  background image is followed by posting a set of *draw-commands*,
  the **render**-method must be called later manually when the image
  annotation is finished. Image visualization is part of several
  tutorial chapters (see :ref:`tut.mouse-and-vis`)
  


.. _qt.icldrawwidget3d:


:icl:`qt::ICLDrawWidget3D`
""""""""""""""""""""""""""

  At the last level, the :icl:`qt::ICLDrawWidget3D`, which again
  extends the :icl:`qt::ICLDrawWidget` class, provides an interfaces
  for rendering 3D scenes on top of an image. The
  :icl:`qt::ICLDrawWidget3D` provides a :icl:`ICLDrawWidget3D::link`
  method, which links a simple OpenGL callback function to it. Each
  time, the :icl:`qt::ICLDrawWidget3D` is rendered, it will also
  execute the linked OpenGL callback function, synchronously to the
  GUI Thread, while still being able to render 2D annotations.

  .. note::
     
     It is highly recommended to use the :icl:`geom::Scene` class to
     create 3D rendering overlays (**add link here**). The scene class
     can easily provide an appropriate OpenGL callback function for
     it's contained cameras.

  .. todo:: add references and link to the ICLGeom package and the
            Scene class






.. _qt.quick:

The **Quick** Framework
-----------------------

In contrast to the other modules, the **Quick** framework focuses
mainly the programmer's convenience rather than on efficiency. It is
meant to be used for rapid prototyping or for test applications.
However, also in real-time applications some parts are usually not
strongly restricted to real-time constraints, in particular where user
interactions are processed. Most functions do what is usually
completely forbidden in real-time applications: *they return newly
created results images*. This of course usually leads to performance
issues due to run-time memory allocation. However this is not the case
for the **Quick** framework functions that use an internal memory
manager, which reuses no-longer needed temporary images. Even though,
image are returned by instance, due to the :icl:`core::Img`'s *shallow
copy*-property, no deep copies are performed.


**Overview**

  * :ref:`qt.quick.affinity`
  * :ref:`qt.quick.show`
  * :ref:`qt.quick.image-creation`
  * :ref:`qt.quick.filtering`
  * :ref:`qt.quick.operators-1`
  * :ref:`qt.quick.math`
  * :ref:`qt.quick.operators-2`

The framework is basically available directly by including the
**ICLQt/Quick.h** header, which is automatically included by the
*commonly* used header **ICLQt/Common.h**. This header contains a huge
set of global functions that simplify some things significantly --
however sometimes a little bit less efficient. Most of these functions
are right now implemented for the :icl:`core::Img32f` type only. The
Quick-header uses a typedef to :icl:`qt::ImgQ` to underline the fact that
we are working with this framework. Furthermore, the Quick header does
also automatically use all icl-namespaces and also the std::namespace,
which is why it should never be included by other header files.
Here is an example for a simple *difference image*-application:

+----------------------------------------------+-----------------------------------+  
| .. literalinclude:: examples/qt-quick.cpp    | .. image:: images/qt-quick.png    |
|    :linenos:                                 |      :alt: shadow                 |
|    :language: c++                            |                                   |
+----------------------------------------------+-----------------------------------+  

.. _qt.quick.affinity:

Affinity to **ImgQ**
^^^^^^^^^^^^^^^^^^^^

Many of the global functions are implemented twice, as template and as
normal function for the :icl:`ImgQ` (aka :icl:`core::Img32f`)-type
e.g.::
  
  load("myimage"); 

returns an :icl:`qt::ImgQ`, while::

  load<icl8u>("myimage");
  
returns the image as :icl:`core::Img8u`.


.. _qt.quick.show:

The Very Special Function **show**
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:icl:`qt::show`

  shows an image using an external viewer application. In order to
  avoid complex GUI handling and thread synchronization issues, the
  :icl:`qt::show` function simply writes the given image to a
  temporary image file, that is then opened using the **icl-xv**
  application. **icl-xv** is called with the **-d** flag, which lets
  **show** delete the temporary image immediately when loaded. This
  does of course only work if **icl-xv** can be found in the users
  **PATH** variable. Alternatively the function :icl:`qt::showSetup`
  can be used to define a custom image viewer command

.. _qt.quick.image-creation:

Image Creation Tools
^^^^^^^^^^^^^^^^^^^^

:icl:`qt::zeros` and :icl:`qt::ones`

  create images in *matlab*-manner. **zeros(100,100,3)** creates an
  empty 3-channel image of size 100x100. **ones** behaves equally, but
  sets all pixel values to 1


:icl:`qt::load`

  just loads an image from a file, which is usually much easier than
  using an :icl:`io::Grabber` for this. However, please note that the
  **load** function does not preserve the image depth, which is
  :icl:`core::depth32f` by default, or must be explicitly given using
  **load<target-pixel-type>("filename")**.

:icl:`qt::create`

  creates test image, that is hard-coded within the ICLIO-library.
  Supported test images are "lena", "cameraman", "mandril", "parrot",
  and a few others.
   
:icl:`qt::grab`

  just grabs an image using an internal grabber handling

.. _qt.quick.filtering:

Image Filtering and Conversion Functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:icl:`qt::filter` 

  applies one of a set of predefined image filters

:icl:`qt::scale`

  scales an image by either a factor or to a target size
  
:icl:`qt::levels`
  
  re-quantifies an images value domain to a given number of levels
 
:icl:`qt::copy` 

  performs a deep copy of an image::
    
    ImgQ a = zeros(100,100,1);
    ImgQ b = copy(a);

:icl:`qt::flipx` and :icl:`qt::flipy`

  flips an image

:icl:`qt::cvt`
  
  converts images of any type to :icl:`ImgQ`
  
:icl:`qt::cvt8u`, :icl:`qt::cvt16s`, ...

  convert images to target depth

:icl:`qt::blur`

  blurs an image by given mask radius using a separatale Gaussian
  filter

:icl:`qt::rgb`, :icl:`qt::hls`, :icl:`qt::gray`, ...

  converts a given image to target format

:icl:`qt::channel`

  extract a single channel from an image
  
:icl:`qt::thresh`

  applies a threshold operation 

:icl:`qt::label`

  adds a simple text-label to the top left corner of an image.

  .. note::
     
     This function does not create a new image, but it works on the
     given image (of which a shallow copy is also returned)


:icl:`qt::roi`

  allows for copying/extracting an image ROI::

    ImgQ a = ones(100,100,5);
    a.setROI(Rect(10,10,30,30));
    ImgQ b = roi(a);
    roi(a) = 255;

.. _qt.quick.operators-1:

Arithmetical and Logical Operators
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The **Quick**-header also provides simple to use C++ operators for the
:icl:`qt::ImgQ` (aka :icl:`core::Img32f`) class. The binary operators
**+**, **-**, ***** and **/** are defined for pixel-wise operations on
two images and pixel-wise operations with a scalar:
  
+--------------------------------------------------+---------------------------------------+  
| .. literalinclude:: examples/quick-operators.cpp | .. image:: images/quick-operators.png |
|    :linenos:                                     |    :scale: 60%                        |
|    :language: c++                                |    :alt: shadow                       |
+--------------------------------------------------+---------------------------------------+  

The same is true for the logical operators **&&** and **||**, which
perform a pixel-wise logical combination of two images.


.. _qt.quick.math:

Math Functions
^^^^^^^^^^^^^^

In addition to the mathematical operators, also some mathematical
standard functions are overloaded for the :icl:`qt::ImgQ` type:
:icl:`qt::sqr`, :icl:`qt::sqrt`, :icl:`qt::exp`, :icl:`qt::ln`, and
:icl:`qt::abs` are performed pixel-wise.


.. _qt.quick.operators-2:

Image Concatenation Operators
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As it could be seen in the example above, also the three operators
**,**, **%** and **|** are implemented. However, it is important to
mention that we strongly adapted the expected default behavior of
these operators:

**The ,-Operator** 

  is used to concatenate image horizontally. If the image heights are
  not equal, the smaller image is just made higher (without scaling the
  contents)
  
**The %-Operator**

  performs a vertical image concatenation
  
**The |-Operator**

  stacks the channels of two images.

.. _qt.mouse-handlers:

The MouseHandler Interface
^^^^^^^^^^^^^^^^^^^^^^^^^^

The :icl:`qt::MouseHandler` interface is provided for mouse-based
interaction with- or on top of images. The :icl:`qt::MouseHandler` 's
single interface function

  :icl:`qt::MouseHandler::process(const qt::MouseEvent &)`

can be implemented to get custom mouse handling behavior.  The
:icl:`qt::MouseEvent` class does always provide information about the
current image position and pixel color, the mouse is currently at.
(Detailed information on this can be found in the tutorial chapter
:ref:`tut.mouse-and-vis`.

There are some special predefined MouseHandler implementations for
common purposes. The :icl:`qt::DefineRectanglesMouseHandler` can be
used to define a given number of rectangles on top of an image. These
can then be accessed and used for further processing steps. The
:icl:`qt::DefineQuadrangleMouseHandler` is provided to define a single
arbitrary quadrangle in an image.

