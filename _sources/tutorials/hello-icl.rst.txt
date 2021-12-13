.. include:: ../js.rst

.. _hello-icl:

#########
Hello ICL
#########

Let's start with a very simple example, which visualizes a demo image
only. The basic idea for this chapter is to help you to set up all
your environment variables to easily compile and run your example code.

+--------------------------------------------+----------------------------------+
| .. literalinclude:: examples/hello-icl.cpp | .. figure:: images/hello-icl.png |
|    :language: c++                          |    :scale: 60%                   |
|    :linenos:                               |    :alt: shadow                  |
+--------------------------------------------+----------------------------------+   

In order to make this demo work, you have to apply the following steps (linux/mac):


* ensure you have all necessary build tools like g++, make and
  pkg-config installed and available on your system
* adapt your **PKG_CONFIG_PATH** environment variable by adding
  **$ICL_PREFIX/lib/pkgconfig**
* now, typing **pkg-config --libs --cflags icl-8.0.0** in your bash should
  provide you some useful compiler and linker flags add ICL's
  bin-directory to your PATH environment variable. This is necessary
  for the show-function, which starts an external process to for image
  visualization.

  Addittionally e.g. **which icl-xv** should print you the
  complete path to ICL's image-viewer application
* furthermore you can now compile every foo.cpp file into an
  executable that links against ICL by using the script **icl-make**
* simply save the example above as example.cpp in your current
  directory and type **icl-make example**
* this should invoke c++ with all necessary compiler and linker flags
* once your demo application is built, you can run it by typing 
  **./example**

The example code contains actually two nested function calls: The
inner :icl:`create` function and the outer :icl:`show`
function. Actually, you will find these functions in the namespace
:icl:`icl::qt`, however these are not needed here because the header
:icl:`ICLQt/Quick.h<Quick.h>` is included. This header is meant for
rapid prototyping and therefore it will automatically use all
ICL-namespaces and the **std**-namespace

* :icl:`qt::create` creates a demo image that is specified by
  the given string value. 
* :icl:`qt::show` is a very special function. It saves the given
  image to a temporary file, and starts the application **icl-xv** in
  order to display this image. **icl-xv** can be set up to delete the
  visualized image-file. This helps to avoid having to delete all
  temporary files manually. Furthermore, by using the external
  application **icl-xv**, the current application does not have to
  implement GUI- and event handling itself. (Note: it is also possible
  to set up the show function to use a custom image-viewer
  application, see :icl:`showSetup`).


