.. include:: ../js.rst

.. _tut.buttons:

#####################################
Buttons: Callbacks vs. wasTriggered()
#####################################


The :icl:`Button` component behaves slightly different from most of
the other components. If we consider a slider, that is used to adjust
a certain value, it is completely clear, how this value, even though
it is adapted in the application's GUI thread, can be accessed in the
working thread. Here, our implementation will just keep a registered
**int** variable up to date, whose value is returned when the slider
value is queried from the :icl:`GUI` instance. In contrast, a button
usually triggers some kind of event. However, we can not simply
register a callback to the buttons *click* event, since the callback
would then be executed in the GUI thread, leaving the user all the
extra work for explicit synchronization with the application's working
thread. 

Actually, in some special situations, a callback registration is
exactly what we want, in particular, if e.g. a button-click affects
some other Qt-GUI components directly. For this, the :icl:`GUI` class
provides the ability to register callbacks to nearly all components.

However, as introduced above, most of the time, a button-click is not
to be executed immediately, but the next time, the working-thread run
through. For this purpose the button-component's handle class
:icl:`ButtonHandle` provides the
:icl:`wasTriggered<ButtonHandle::wasTriggered>` method, that returns
true if the button was pressed since the method was called before.

****************
A Simple Example
****************

Lets just implement a simple image processing application, that
applies one of the morphological operators provided by ICL in
real-time on a given input image. The application has three buttons:

1. use next filter
2. save current result image
3. show an extra GUI for the input image

For the the buttons 1 and 2 we will use the :icl:`ButtonHandle` and
its :icl:`wasTriggered<ButtonHandle::wasTriggered>` method. Since
button 3  affects a Qt-GUI component, we will use the callback
mechanism for this.

+------------------------------------------------+----------------------------------------+
|                                                | **main GUI**                           |
|                                                |                                        |
| .. literalinclude:: examples/gui-buttons.cpp   |   .. image:: images/gui-buttons-1.png  | 
|   :language: c++                               |        :alt: shadow                    |
|   :linenos:                                    |                                        |
|                                                | **extra GUI**                          |
|                                                |                                        |
|                                                |   .. image:: images/gui-buttons-2.png  |
|                                                |        :alt: shadow                    |
|                                                |                                        |
|                                                | started with::                         |
|                                                |                                        |
|                                                |   ./example -input create cameraman    |
|                                                |                                        |
+------------------------------------------------+----------------------------------------+


Step by Step
************


After including the prototyping header
:icl:`ICLQt/Common.h<Common.h>`, and the header for the
:icl:`filter::MorphologicalOp` that is used, the static application
data is declared.  We use two GUI instances, one of the
:icl:`GUI`-subclass :icl:`HSplit`. Furthermore, a
:icl:`GenericGrabber` is used for image acquisition and a
simple integer variable for the current morphological filter type.

.. literalinclude:: examples/gui-buttons.cpp
   :language: c++
   :lines: 1-8

As usual, the application uses the default **init** -- **run** --
**main** combination. In **init**, we first initialize the
:icl:`GenericGrabber` instance by linking it to our program argument
"-input". Now, two seperate :icl:`GUI` instances are to be created:
our main GUI called **gui** and a simple extra GUI for the optional
visualization of input images, called **input**. The main
:icl:`HSplit` GUI is filled with two sub-components, an :icl:`Image`
for the visualization of the result images and an extra :icl:`VBox`
container for the buttons, each set up with a unique handle for later
access. The GUI is immediately created and shown by streaming a
:icl:`Show`-instance at the end of the GUI definition expression.

.. literalinclude:: examples/gui-buttons.cpp
   :language: c++
   :lines: 10-18

The **input**-GUI is only filled with a simple extra image
visualization component. Here, we use :icl:`Create` instead of
:icl:`Show`, which creates the GUI without directly showing it.

.. literalinclude:: examples/gui-buttons.cpp
   :language: c++
   :lines: 20

Finally, we register a callback to the "show src" Button. This is
necessary because we cannot affect Qt-Windows and Widgets from the
applications working thread. Registered callbacks are always processed
in the GUI thread, where this is allowed. We simply create an
anonymous member function here, that changes the **input** GUI's
*visible*-flag at each klick.

.. literalinclude:: examples/gui-buttons.cpp
   :language: c++
   :lines: 22,23

Once, the initialization is finished, the run method is
implemented. Here, we first extract two :icl:`ButtonHandle` instances
from the applications main GUI. These are later used for the
:icl:`wasTriggered()<ButtonHandle::wasTriggered>`-mechanism.

.. literalinclude:: examples/gui-buttons.cpp
   :language: c++
   :lines: 27,28


Now, the actual image processing loop is implemented. The next
image is acquired form the :icl:`GenericGrabber` instance. Before
the morphological operator is applied to the input image, we check
whether the *next filter* button has been pressed since the last
loop cycle. 

.. literalinclude:: examples/gui-buttons.cpp
   :language: c++
   :lines: 30-35

If this is true, the next valid :icl:`MorphologicalOp::optype` value
is estimated and used to set up our operator. Now, the filter is applied
and the result image can be visualized.

.. literalinclude:: examples/gui-buttons.cpp
   :language: c++
   :lines: 37-39

Only if the **input**-GUI is visible, the source image is visualized
as well. This is not completely necessary, but leads to a cheap
optimization in case of the **input**-GUI is not visible, because the 
image data must not be transferred to the GUI-thread in this case.

.. literalinclude:: examples/gui-buttons.cpp
   :language: c++
   :lines: 41-43

Finally, we check the **save**-button. If this was triggered, the 
current result image is saved.

.. literalinclude:: examples/gui-buttons.cpp
   :language: c++
   :lines: 45-47

.. note:: 

   It is very important to know, that we cannot make use of a Qt-Dialog
   here to ask the user for a desired file-name. If a GUI-based dialog is 
   to be used, this step has to be transferred to the GUI-thread by using
   a GUI callback for this button as well. 


GUI-Dialogs
***********

As motivated and discussed before, GUI-Dialogs can only be
instantiated in the application's GUI thread, but not in the working
thread. However, if dialogs processed in the GUI-thread by using the
:icl:`GUI`'s callback mechanism, temporary values from the working thread
cannot be accessed directly.

.. literalinclude:: examples/gui-buttons-2.cpp
  :language: c++
  :linenos:


As we can see, only a few changes were to be made here. First of all,
we use a global result image instance here that is protected by a
:icl:`Mutex` instance.

.. literalinclude:: examples/gui-buttons-2.cpp
  :language: c++
  :lines: 10,11


Furthermore, we add an extra **saveImage** function, that is linked as
a callback to the button click event. The function uses the simple to
used :icl:`qt::saveFileDialog()` function, that simply wraps a Qt file
dialog. Its call is set into a **try-catch** block to react to the
case if the user presses the *cancel* or the *window-close* button in
this dialog. If a valid image file name was provided, the result image
mutex is locked (here using a scoped lock of type
:icl:`Mutex::Locker`) and the current result image can be saved just
like in the former example. Moving the scoped lock on top of the call
to :icl:`qt::saveFileDialog` would suspend the working thread while
the user selects a file name, which is usually more practical.

.. literalinclude:: examples/gui-buttons-2.cpp
  :language: c++
  :lines: 13-19

In the **init** method, we only need to register an extra callback:

.. literalinclude:: examples/gui-buttons-2.cpp
  :language: c++
  :lines: 36

In **run**, we no longer need to handle the **save** button
explicitly.  Instead, we have to use the global **result**-image, to
make it accessible from the GUI-thread as well. While the new result
image is created, we lock the corresponding :icl:`Mutex` variable in
order to avoid that the image is saved while it is updated by the
working thread. Without the locking mechanism, the image could either
be reallocated by the morphological operator, which would lead to a
possible *segmentation fault* in the **saveImage** callback
function. But also if the image data is not reallocated, **saveImage**
would not be synchronized with the working thread and therefore
possibly save have-adapted images.

.. literalinclude:: examples/gui-buttons-2.cpp
  :language: c++
  :lines: 48-50









