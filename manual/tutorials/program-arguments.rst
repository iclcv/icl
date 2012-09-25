.. include:: ../js.rst

.. _progarg-tutorial:

############################
Evaluating Program Arguments
############################


ICL's :ref:`ICLUtils<utils>` package contains the *program argument
evaluation framework*, providing the following functionalities:


* a simple and intuitive string-based description of allowed program
  arguments

* an easy to use way to describe program arguments

* a efficient parser for program arguments, that provides
  human-readable error messages

* an automatically supported set of common default program arguments,
  such as **-version** or **-help**

* a concise method :icl:`pa` that can be use to query whether a
  specific program argument has been given and what it’s sub arguments
  where



Program Arguments and Sub-Arguments
***********************************

In general, we distinguish between the term *program argument*, *sub
argument* and *dangling arguments*. E.g. an image type conversion
application **my-convert** could expect an input image file name, an
output image file name and an argument **-size** that always gets an
extra sub-argument which defines the target image size. A valid call
to the program would then be something like::
 
  my-convert -size 320x240 image.png image.jpg

In this case, we classify the set of all given program arguments as
follows:

* **-size** as a *defined* argument (expecting one sub-argument)
* **320x240** as a *sub*-argument
* **image.png** and **image.jpg** as *dangling* arguments 

All program arguments are either *defined*, expecting 1,n or an
arbitrary number of sub-arguments, *dandling* or *sub-arguments* of
defined ones. Furthermore, arguments can be mandatory or optional.


.. note::
   
   The support for dangling arguments must be enabled explicitly by
   adding a boolean flag to the call to the program argument
   initialization function :icl:`pa_init`. Since it turned out that
   usually applications have no *dangling* arguments, this feature is
   deactivated by default in order to avoid that erroneously given
   program arguments are not detected because they are interpreted as
   dangling arguments.

Defining allowed program arguments
**********************************

Program arguments are defined by using the :icl:`pa_init`
function. Usually, this function must not be called explicitly because
it is called indirectly by the :icl:`ICLApplication` (or short:
:icl:`ICLApp`) constructor. Actually the parameters are simply passed
to :icl:`pa_init` if an :icl:`ICLApp` is used.

:icl:`pa_init` always receives the main function's program argument and
count and a single definition string::

  int main(int n, char **args){
    pa_init(n,args,definition string);
    ..
  }


The definition string consists of space separated tokens of single
argument definitions. If real space characters are needed, they must
be escaped using a trailing backslash character (please note, that one
has to use a double backslash within a c++-string to create a
backslash character). Each single argument definition token consists
of a pipe-symbol separated list of argument name alternatives,
followed by an optional parameter list in round braces, e.g.::

  pa_init(n,args,"-input|-i(filename) -size(outputsize)");

Each *defined* argument may have 0, n, or an arbitrary number of
sub-arguments. If the argument has no arguments -- in this case the
empty brace can also be left out -- it becomes a *flag*, that is
internally represented by a boolean value that defines whether it was
given (then the value is **true**) or not (the value is **false**).
An arbitrary sub-argument count can be achieved by using the intuitive
string "..." as parameter list::

  pa_init(n,ppc,"-enable-preprocessing|-pp -input|-i(...) -output(filename)");

In general, each parameter-list is a comma separated list of tokens that
can be a type name, a type name followed by "=" and a default value or
an integer token, or the special token "...".  Here are some examples:

two integer sub-arguments::

 "(int,int)"

five sub-arguments with unspecified types::

 "(5)"

four sub-arguments, first of type float, second and third of any type
and last of type int::

  "(float,2,int)"

two sub-arguments first of type int with default value "4" and second
of type **Size** with default argument "VGA"::
 
  "(int=4,Size=VGA)"


.. note::

   1. in case of using just a number of sub-arguments, no default value
      can be given

   2. It is either possible to provide default values for all arguments
      or for non of them. So far there is no C++-like mechanism for this.



Types
"""""

**Question:**
  
   What types are allowed/supported? 

**Answer:**

   All types!

Actually, this sounds more advanced than it actually is. Internally,
all sub-arguments are managed as strings. The sub-argument is intended
as a type-hint for the program user, and it is used for the generation
the program usage output, that is created automatically, if the
program is called with a **-help** argument, or if the given set of
program arguments int not compatible with the defined set. Therefore,
we strongly recommend to use explicit type names such as "filename" or
"format-string" rather than just an integer parameter count.


Mandatory Arguments
"""""""""""""""""""

Some of your program arguments might be mandatory for you application.
If, e.g., no image source is defined, your application cannot work
properly. If you access program arguments or sub-arguments, that have
no default value and are not given, an exception is thrown. Since in
this case, the retrieved error message could be difficult to analyze,
certain arguments can be set up to be mandatory. This is done by
adding an "[m]"-token (for *mandatory*) in front of the program argument
definition token::

  pa_init(n,ppc,"-size(Size=VGA) -pp [m]-input|-i(2)");

Here, the application will not start unless the "-input" argument is
given with 2 sub-argument. Those arguments that have default values,
cannot be set to mandatory because the default value would never be
used in this case. And logically, also flags -- arguments without
sub-arguments -- cannot be set to mandatory.


Accessing Program Arguments
***************************

Given program arguments can be retrieved using the :icl:`pa` function
that returns an instance of :icl:`ProgArg`, however :icl:`ProgArg`
instances is most of the time not used explicitly. Instead, the
:icl:`ProgArg` type provides many useful operators that allows for
using it in a very convenient manner.  The following source code shows
some usage examples:

.. literalinclude:: examples/pa.cpp
   :language: c++
   :linenos:

Calling Programs with given Arguments
*************************************

Let's call the resulting program **example**. Obviously, the program
accesses all program arguments, so calling::

  ./example

fails, giving the error message in line 10 that sub-argument 0 of the 
defined argument "-input" was tried to access, but is was neither given
nor defined by a default argument. In real-life examples, those defined
arguments should be declared mandatory.

First of all, let us try to get a list of the allowed program
arguments.  The framework always provides the program arguments
**-help** and **-version**. In turn, these cannot be used for own
purposes. **-help** gives a formatted *usage* output, in this case::

  usage:
	example [ARGS]
  -size|-s      {optional}
                (Size=VGA)

  -index        {optional}
                (int)

  -input|-i     {optional}
                (*,*)

  -files        {optional}
                (...) [arbitrary subargument count possible]

  -help         shows this help text
  -version      shows version and copyright
                information

**-version** provides current version and license information. If
an application has some special license, the license text can be 
set using the :icl:`pa_set_license` function.
Optionally the usage information can be explained using an extra help
text that can be set using the :icl:`pa_set_help_text` function.
A valid (and quite complex) program call could look like::

  ./example a b c -size SVGA d e -index 7 -i dc 0 -files *.cpp

Here, the tokens "a", "b", "c", "d" and "e" are dangling arguments,
because they are neither defined nor associated with sub-arguments to
other defined ones. The defined argument "-size" uses "SVGA" as
sub-argument, "-index" uses "7" as sub-argument and "-i" gets two
sub-arguments "dc" and "0". Finally "-files" gets a list of
sub-arguments expanded by the shell (if there are any .cpp-files in
the current directory).



Adding more Descriptions
************************

In some cases, some or all given program arguments need an extra
description. This can be given using the :icl:`pa_explain` function.

.. note::

   It is very important that :icl:`pa_explain` is called **before**
   :icl:`pa_init` is invoked. Later calls to  :icl:`pa_explain` have
   no effect.

Calling the program

.. literalinclude:: examples/pa2.cpp
   :language: c++
   :linenos:

With::

   ./example-2 --help

Shows the following usage output::   
  
  usage:
  	pa2 [ARGS]
  
  	This is an example program that has no function
  	but it demonstrates how to add some program
  	argument and program description
  
  -size|-s      {optional}
                (Size=VGA)
                image size use for something
  
  -index        {optional}
                (int)
                some index used for something else
  
  -input|-i     {optional}
                (*,*)
                input definition, the first first arg defines
                the input grabber backend, second arg selects
                a certain device from this backend
  
  -help         shows this help text
  -version      shows version and copyright
                information
  
