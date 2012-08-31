GUI-Applications
================

One of ICL's main features is it's tight integration with the GUI
creation framework, which allows for intuitive and simple creation of
GUI-applications. In this section we will develop a simple viewer
application that grabs images from arbitrary image sources and
displays the image.

.. literalinclude:: examples/gui-apps-1.cpp
   :language: c++                         
   :linenos:                              


A basic ICL GUI-application can be structured into three major
functions: **main**, **init** and **run**

**main**
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


  
  
  
