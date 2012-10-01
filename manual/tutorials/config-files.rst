.. include:: ../js.rst

.. _config-file-tutorial:

#############################
XML-based Configuration Files
#############################

The :icl:`ConfigFile` class provides an easy to use yet very powerful
building block for complex applications. :icl:`ConfigFile` instances
provide an intuitive interface for XML-based configuration files without
the need for any manual XML-parsing. However, :icl:`ConfigFile` can be
easily serialized into simple XML files that are still human readable.

The :icl:`ConfigFile` class provides *"one line of code solutions"* for:

* serialization into XML-files
* deserialization from XML-files
* type-safe insertion of entries
* type-safe retrieval of entries
* static custom type registration mechanism

The XML Structure
*****************

First of all, let's take a look at the XML-Structure that is used. The
:icl:`ConfigFile` class uses a restricted XML-format using only three
different types of tags:

1. a mandatory top level "<config>" tag
2. "section" tags with an "id" attribute: "<section id='something'>"
3. "data" tags with an "id" and a "type" attribute:
   "<data id='entry-id' type='int'>5</data>"

Here is an example xml-file:

.. literalinclude:: examples/config-file.xml
     :language: xml

As we can see, the file has a very simple structure. Sections may
contain data nodes and/or other sections. Each data node defines an
entry of the :icl:`ConfigFile` where the actual data entry is given by
the text-value of the data nodes.

Reading and Writing XML Files
*****************************

In C++, the file (here called
**config.xml** can be loaded by::

  ConfigFile cfg("config.xml");

This will parse the whole config-file and create an efficient STL-map
based lookup for all it's entries.

.. note::
   
   Internally, we use the 3rd party library pugi-XML as XML parser,
   which is included as a source file and directly linked into the
   ICLUtils library

The :icl:`ConfigFile` instance **cfg** can now also intuitively be
written to an xml-file (here named "config2.xml" using::

   cfg.save("config2.xml");

Adding Entries to the XML File
******************************

Lets start with an empty file and add all the entries of the 
example above.

.. literalinclude:: examples/config-file.cpp
   :language: c++
   :linenos:

As we can see, this is at least as transparent as writing the XML-file
manually. Sections are addressed by the '.'-delimited string list where
the last token always defines the actual data-entry's *id*. The example
also shows, that sections are automatically added on demand during the 
C++-based creation process. Furthermore, we demonstrate the automatic
type inference mechanism, that derives the data-tag's type entry from
the source type. Types that are not registered cannot be added to 
:icl:`ConfigFile` instances::

  struct MyType {};
  ConfigFile f;
  f["config.x"] = MyType(); // error type is not registered!
  

The Prefix
""""""""""
During the data insertion, we might have to use an identical prefix
several times leading to many long lines in your C++-code. This can
be bypassed by setting up the :icl:`ConfigFile` instance with a 
temporary path prefix. 

.. literalinclude:: examples/config-file-2.cpp
   :language: c++
   :linenos:

.. note::
 
  The prefix is just treated as an **std::string** variable, that
  is automatically prepended to the :icl::`ConfigFile`'s index
  operator's argument. Therefore, the prefix must actually end with
  a '.'-character so that the following relative paths make sense 
  then.


Extracting Entries for Config Files
***********************************

For the extraction of :icl:`ConfigFile` entries, a special C++-
technique is used in order to provide type-safe lvalue-based type
inference. Let's first consider some examples:

.. literalinclude:: examples/config-file-3.cpp
   :language: c++
   :linenos:

The example demonstrates the intuitive mechanism. Please note, that
extracting an entry as a wrong type leads to an exception, even
if the assignment seems trivial::

   float value2 = cfg["config.general.params.value"]; // error!

If a behavior like this is required, one can either first use a
correct assignment::

   double tmp = cfg["config.general.params.value"]; 
   float value2 = tmp;

Or use the :icl:`ConfigFile::Data::as` method, that implements
an explicit yet still type-safe cast into a given destination
type::

   float value2 = cfg["config.general.params.value"].as<double>();


Static Type Registration
************************

The :icl:`ConfigFile` class can also be set up to accept custom 
types. For this, C++'s ostream- and istream-operators must fist
be overloaded for the desired type. The new type can then be 
registered using the :icl:`ConfigFile::register_type` template,
which gets a template-based type and a string-based type-ID. Here is
an example:

.. literalinclude:: examples/config-file-4.cpp
   :language: c++
   :linenos:


Use the :icl:`ConfigFile` for (De)serialization
***********************************************

For complex types it is usually quite difficult to find a nice way to
implement C++-'s **istream**- and **ostream**-operators. In particular
the implementation if the **istream**-operator is usually very
difficult because it should usually also detect parsing errors. Using the
:icl:`ConfigFile` class for this has two advantages: first, it solves
all serialization and deserialization issues, and second, it also leads
to a human readable and well defined XML-based serialization string.

.. literalinclude:: examples/config-file-5.cpp
   :language: c++
   :linenos:
