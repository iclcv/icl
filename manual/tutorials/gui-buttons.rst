.. include:: ../js.rst

.. _tut.buttons:

#################
Using GUI Buttons
#################


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

Lets just implement a simple image processing application, that applies
a filter in real-time on a given input image. The application has
three buttons:

1. use next filter
2. save current result image
3. show an extra property GUI

For the the buttons 1. and 2. we will use the :icl:`ButtonHandle`
and its :icl:`wasTriggered<ButtonHandle::wasTriggered>`. Since button 3.
affects a Qt-GUI component, we will use the callback mechanism for this.

