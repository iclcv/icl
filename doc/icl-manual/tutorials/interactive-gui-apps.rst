.. include:: ../js.rst

.. _tut.interactive-gui-apps:

Interactive GUI Applications
============================


Most of the time, applications demand user input e.g. clicking
interesting image regions, or adjusting a threshold using a
slider. ICL's **GUI** creation framework provides a very intuitive and
easy-to-use interface to create even complex interactive applications.

Let's start with a very simple interactive application. Consider a
simple online image thresholding application, where a slider is to be
used to define the current threshold value at run-time.


+---------------------------------------------------------+--------------------------------------------+
| .. literalinclude:: examples/interactive-gui-apps-2.cpp | .. image:: images/interactive-gui-apps.png |
|   :language: c++                                        |      :alt: shadow                          |
|   :linenos:                                             |                                            |
+---------------------------------------------------------+--------------------------------------------+


Step by Step
""""""""""""

Ok, let's go through the code line by line in order to discuss it's
ingredients. we begin with including the headers, needed for our example.

.. literalinclude:: examples/interactive-gui-apps-2.cpp
   :language: c++
   :lines: 1-3

Now, some global data is defined. Even though, using global data is not
optimal in terms of programming style, this is completely ok here, because
we define a final application rather then a reusable tool. Therefore, the
global variables cannot interfere with other symbols linked against later.
Again, we use a **GUI** instance and a **GenericGrabber**. Additionally,
we instantiate a **UnaryCompareOp** for the thresholding operation [#f1]_
and an **FPSLimiter** that can be used to limit the run loops iteration
count per second.

.. literalinclude:: examples/interactive-gui-apps-2.cpp
   :language: c++
   :lines: 4-8

Once the application data is instantiated, it needs to be initialized,
which is commonly performed in the **init** function, called by the
**ICLApp** instance, created in **main**. In this example, the GUI is
set up to also show a slider that is used for adapting the threshold
at run time. As one can see, the **GUI**'s stream-operator **<<** can
be used add **GUIComponent**\ s in a very simple manner. Components,
that need to be accessed later, can be referenced by a string-ID
called *handle*. Each **GUIComponent** has a set of component-specific
parameters that are given to the component constructor an a set of
more general, but optional parameters, that can be added using the
**.**-operator.

.. literalinclude:: examples/interactive-gui-apps-2.cpp
   :language: c++
   :lines: 11-14

.. note::
   The GUI-creation framework is also described in **TODO**

The actual image processing part is implemented in the **run**
method. First, we need to extract the current slider value from our
global **GUI** instance. Accessing GUI components for both reading or
setting values is performed using the index operator
(**gui["handle"]**). This allows to seamlessly synchronize the GUI
thread and the working thread of interactive applications. First, the
current slider value is used as comparison threshold for the
**UnaryCompareOp** instance, which is then applied on the next grabbed
image. The result is directly passed to the visualization. Finally,
**fps.wait()** will wait to ensure the desired FPS count.

.. literalinclude:: examples/interactive-gui-apps-2.cpp
   :language: c++
   :lines: 18-22

The run method basically stayed the same as in the prior example.

.. rubric:: Footnotes

.. [#f1] 
   Actually, even though we commonly describe the shown operations as
   *thresholding*, it should be named *pixel-wise comparison*
