.. include:: ../js.rst

.. _tut.gui:

################
GUI-Applications
################

One of ICL's main features is it's tight integration with the GUI
creation framework, which allows for intuitive and simple creation of
GUI-applications. In this section we will develop a simple viewer
application that grabs images from arbitrary image sources and
displays the image.

For a more general overview about the GUI framework, we recommend to 
also study :ref:`qt`.

.. _simple-example:

Simple Example
""""""""""""""

.. literalinclude:: examples/gui-apps-1.cpp
   :language: c++                         
   :linenos:                              

.. note:: 
   a documented version of this source code can be found :ref:`below<documented-gui-app>`

A basic ICL GUI-application can be structured into three major
functions: **main**, **init** and **run**

**main**
""""""""

This function simply creates an instance of type
**ICLApplication** (or short ICLApp). The ICLApp instance always
gets the following arguments: 

* the program argument count the program

* argument list the program

* argument definition string (which can be set to **""**, if program
  argument support shall not be supported)

* an initialization function

* further functions, that are automatically distributed to extra threads

In fact, The **ICLApp** instances does a lot of work for you.

* it creates a **QApplication**

* it calls your initialization function before the QApplications
  event-loop is entered

* it parses all program arguments (and notifies errors)

* it creates working threads for each given **run**-function

* it joins all threads before the internal Qt-event loop is shut
  down.


**init**
""""""""
This function is automatically invoked by the **ICLApp**
instance that was created in **main**. Here, the application and in
particular the **GUI** can be initialized. Program arguments can
also be used here.

**run**
""""""" 
This function is called in a loop in the applications working
thread. Normally **run** has the steps 

1. image acquisition
2. image processing
3. restult visualization
         
In the examples the images are visualized without any processing


Documented Example
""""""""""""""""""

.. _documented-gui-app:

First, we include the 'care-free'-header
:icl:`ICLQt/Common.h<Common.h>` which includes all most common
ICL-headers. Please note, this header also includes all icl
sub-namespaces :icl:`icl::utils`, :icl:`icl::math`, etc. as well as
the **std**-namespace

.. literalinclude:: examples/gui-apps-1.cpp
   :language: c++                         
   :lines: 1-5
   :emphasize-lines: 1

We need a global GUI instance that is visible from your **init**- and
from your **run**-function. As you application normally has only one
global GUI instance, the use of a global variable is justifiable and
natural here.

.. literalinclude:: examples/gui-apps-1.cpp
   :language: c++                         
   :lines: 1-5
   :emphasize-lines: 3


The same is true for the image source of your application.
Here, we use an instance of type :icl:`GenericGrabber`,
which can be configured simply using program arguments

.. literalinclude:: examples/gui-apps-1.cpp
   :language: c++                         
   :lines: 1-5
   :emphasize-lines: 4


We use the init function for initialization of the
grabber and the GUI instance

.. literalinclude:: examples/gui-apps-1.cpp
   :language: c++                         
   :lines: 6-9
   :emphasize-lines: 1


The grabber can be initialized directly with the given
program argument, which can be accessed by the :icl:`utils::pa`
function. :icl:`pa` gets one of the program argument's
name aliases and optionally also the sub-argument index.

.. literalinclude:: examples/gui-apps-1.cpp
   :language: c++                         
   :lines: 6-9
   :emphasize-lines: 2

now we have to create a GUI component for image
visualization. To this ends, we use ICL's powerful
Qt-based GUI creation framework. Here, GUI components
can conveniently streamed into each other to create
complex GUIs, but also for simple GUI as necessary
in this example, it is convenient to use. 

.. literalinclude:: examples/gui-apps-1.cpp
   :language: c++                         
   :lines: 6-9
   :emphasize-lines: 3
       

The run function contains our processing loop which is
acutually very simple in this case. It just grabs a new
image and visualizes it with our image visualization GUI
component.

.. literalinclude:: examples/gui-apps-1.cpp
   :language: c++                         
   :lines: 10-12
   :emphasize-lines: 1

Here, we pass the next grabbed image from the grabber instance
directly to the visualization component

.. literalinclude:: examples/gui-apps-1.cpp
   :language: c++                         
   :lines: 10-12
   :emphasize-lines: 2

Finally, the main function creates an :icl:`ICLApp` instance and
returns the result of its :icl:`ICLApplication::exec()`-function. It
also specifies the list of allowed program arguments. In this case,
only a single argument is allowed, that gets two sub-arguments. The
"[m]"-prefix makes the argument mandatory

.. literalinclude:: examples/gui-apps-1.cpp
   :language: c++                         
   :lines: 13-15

Compiling and Running
"""""""""""""""""""""

Again, you can set up you build-environment as described :ref:`before
<hello-icl>`. If your used ICL build does at least support Qt and
libjepg, you can run your application with

**./application-name -input create lena**

The program argument **-input device-type device-info** is used for
most ICL-applications. In combination with an instance of type
**GenericGrabber**, this enables you to create applications that are
able to work with arbitrary image sources. Here are some further
examples:


**-input create parrot** 
  uses a static *hard-coded* test image showing
  a parrot as input. Each time, the **GenericGrabber**'s grab-function is
  called, the same image is returned.

**-input file image.pnm**
  uses a constant image file as input

**-input file 'images/*.jpg'**
   uses all images as input that match the given pattern. Note: the
   pattern is expanded internally, so you have to set it into tics (')
   to avoid that the pattern is already expanded by your bash

**-input v4l 0** 
  This will use the first *Video4Linux2*-based camera (usually used
  for webcams and builtin cameras) that is available [#f1]_

**-input dc 0** 
  This will use the first firewire-camera that is connected to your PC
  as input [#f2]_ 

**-input video myvideo.avi** 
  This will grab images frame by frame from the given video file [#f3]_

.. note:: 
   A complete list of supported device-types and their device
   selection parameters is given in the **GenericGrabber** reference
   (**TODO:** a link would be better here!). 

.. rubric:: Footnotes

.. [#f1] needs ICL built with v4l2 support
.. [#f2] needs ICL built with libdc support
.. [#f3] 

   needs ICL built with opencv support, and the video codec must
   be compatible
