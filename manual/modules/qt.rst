**Qt** (GUI Creation and Visualization Framework)
=================================================

As it's core, this package provides a wrapper API for the development
of Qt-based GUI applications. The main class of this package is the
**qt::GUI**, which is a general container component. In addition to
wrapper components for all common *Qt-Widgets*, such as sliders,
buttons and check-boxes, we provide a set of highly optimized image
and data visualization *Widgets*. We will here give a complete but
concise overview of ICL's GUI create framework. Step by step examples
are given in the tutorial chapters (:ref:`tut.gui` and
:ref:`tut.interactive-gui-apps`)

Table of Contents
^^^^^^^^^^^^^^^^^

* :ref:`qt.motivation`
* :ref:`qt.gui-creation-framework`
* :ref:`qt.image-vis-framework`

  * :ref:`qt.glimg`            
  * :ref:`qt.iclwidget`      
  * :ref:`qt.icldrawwidget`  
  * :ref:`qt.icldrawwidget3d` 

.. _qt.motivation:

Motivation
^^^^^^^^^^

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
  
  .. todo:: more?

.. _qt.gui-creation-framework:

The **GUI** Creation Framework
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


There are most different approaches for the creation of graphical user
interfaces -- short GUI's. ICL follows the idea of the Qt-library,
that uses a object oriented class tree whose nodes describe
GUI-interfaces and classes. Actually, ICL's GUI creation framework is
a shallow wrapper around the Qt library, that always preserves
accessibility to its internally used Qt *Widgets*.

We basically distinguish between two different types of GUI
components: *normal components*, represented by the
**qt::GUIComponent** class interface and *container components*,
represented by the **qt::ContainerGUIComponent** class interface. Each
container type provides a special layout for its contained components,
which can either be normal- or other container components. By these
means it allows for creating very complex and hierarchical GUI
layouts.



Introduction
""""""""""""

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


A Simple Slider
"""""""""""""""

A Qt-expert might thing *"So what?"*, because he can create a slider
with layout in 1 minute with only 5 lines of code. ICL's GUI creation
framework also endows non-Qt-experts with the ability to create and
layout a slider using a single line of code.


+----------------------------------------------+-----------------------------------+  
| .. literalinclude:: examples/qt-slider.cpp   | .. image:: images/qt-slider.png   |
|    :linenos:                                 |                                   |
|    :language: c++                            |                                   |
+----------------------------------------------+-----------------------------------+  



The **ICLApplication** class
""""""""""""""""""""""""""""

As we will seen in the following examples, the **ICLApplication**
(typedef'd to **ICLApp**), is a very central component of interactive
ICL applications. Usually, it is instantiated with a given
initialization and working-thread function pointer. The latter one is
not used and therefore left out in the example above. Its
**exec**-method performs the following steps:

1. instantiating a *singelton* QApplication instance
2. parsing optionally given program arguments (see :ref:`utils.pa`)
3. calling the initialization method in the applications main thread
4. creating a working thread for each given *run-function-pointer*
5. start the thread, which loops this function
6. entering the Qt event loop


Accessing the GUI from the Working Thread
"""""""""""""""""""""""""""""""""""""""""

As soon as our application needs a working loop, e.g. for image
processing, a **run** method can be passed to the **ICLApp**
constructor as well. The top-level GUI component -- usually
represented by a global **GUI** instance -- provides thread-safe
access to all contained component states. Please note, that **GUI**
instances must be created by either streaming the special **qt::Show**
or **qt::Create** component into it or by calling its **create** or
**show** method. Before this is done, only a hierarchical description
of the GUI exists, but not underlying Qt-components. GUI components
can be access using the GUI's index operator **gui["text"]**, where
**"text"** refers to a component's *handle* or *output* ID, which is
given by the **.handle("handle-name")** method in the GUI-definition
expression.

+----------------------------------------------+-----------------------------------+  
| .. literalinclude:: examples/qt-access.cpp   | .. image:: images/qt-access.png   |
|    :linenos:                                 |                                   |
|    :language: c++                            |                                   |
|    :emphasize-lines: 9,10,17,20              |                                   |
+----------------------------------------------+-----------------------------------+  




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
| * :ref:`qt.iclwidget`            |                                            |
| * :ref:`qt.icldrawwidget`        |                                            |
| * :ref:`qt.icldrawwidget3d`      |                                            |
+----------------------------------+--------------------------------------------+

.. _qt.glimg:

The **qt::GLImg** class
"""""""""""""""""""""""

  At the lowest layer, the **qt::GLImg** provides an interfaces for
  converting **core::ImgBase** instances into an OpenGL texture (if
  the image is larger than OpenGL's maximum texture sizes, it has to
  be split into several texture) that can be drawn arbitrarily into an
  OpenGL scene. Internally, the **qt::GLImg** class is used for
  supporting different image depths. Here, OpenGL's pixel-transfer
  parameters are used for hardware accelerated brightness and contrast
  adjustment. Furthermore, fitting images into the widget viewport can
  simply be performed by the graphics hardware. The **GLImg** can also
  be used as efficient video texture. In order to reduce the use of
  graphics memory bandwidth, the **qt::GLImg** class uses a
  *dirty-flag* to determine whether an image texture actually needs to
  be updated.

.. _qt.iclwidget:

The **qt::ICLWidget** class
"""""""""""""""""""""""""""

  The next layer is implemented by the **qt::ICLWidget** class, which
  inherits Qt's QGLWidget class for the creation of an embedded OpenGL
  context and viewport. The **qt::ICLWidget** provides a software
  interface for setting different visualisation parameters as well as
  an embedded user interface for GUI-based adaption of these
  parameters. Furthermore, the **qt::ICLWidget** provides the simple
  to use method::

    setImage(core::ImgBase*)

  which simply lets it visualize a new image immediately. Internally,
  the image is buffered into a mutex-protected interleaved
  intermediate format, which can more easily be transferred to the
  graphics buffer. Therefore **setImage** can simply be called from
  the application's working thread without any explicit
  synchronization. Once an new image is given, the **qt::ICLWidget**
  will automatically post a *Qt-update-event* by calling the
  ICLWidget::render() method. By these means, the internally used
  OpenGL context is actually re-rendered asynchronously in the
  application's GUI thread.

.. _qt.icldrawwidget:

The **qt::ICLDrawWidget** class
"""""""""""""""""""""""""""""""

  For image annotation, such as rendering box- or symbol-overlay for
  the visualization of current image processing results, the
  **qt::ICLDrawWidget** is provided. It works like a *drawing
  state-machine* that automatically synchronized image annotation
  commands with Qt's event loop. Internally, this is achieved by using
  two thread-safe *draw-command-queues*. One of these queues can be
  filled with new draw commands, while the other queue belongs to the
  GUI thread and is rendered. Every time, the parent **qt::ICLWidget**
  classe's **render**-method is called, the queues are swapped, and
  the queue that is now being filled with new commands is
  automatically cleared. At this point, the **qt::ICLDrawWidget**
  adapts the behavior of the parent **qt::ICLWidget** class, by not
  automatically calling **render** when a new background image is
  given. Since usually setting the background image is followed by
  posting a set of *draw-commands*, the **render**-method must be called
  later manually when the image annotation is finished.

  .. todo:: add example code or link to example code

.. _qt.icldrawwidget3d:

The qt::ICLDrawWidget3D class
"""""""""""""""""""""""""""""


  At the last level, the **qt::ICLDrawWidget3D**, which again extends
  the **qt::ICLDrawWidget** class, provides an interfaces for
  rendering 3D scenes on top of an image. The **qt::ICLDrawWidget3D**
  provides a **link** method, which links a simple OpenGL callback
  function to it. Each time, the **qt::ICLDrawWidget3D** is rendered,
  it will also execute the linked OpenGL callback function,
  synchronously to the GUI Thread, while still being able to render 2D
  annotations. 

  .. note::
     
     It is highly recommended to use the **geom::Scene** class to
     create 3D rendering overlays (**add link here**). The scene class
     can easily provide an appropriate OpenGL callback function for
     it's contained cameras.

  .. todo:: add references and link to the ICLGeom package and the
            Scene class


