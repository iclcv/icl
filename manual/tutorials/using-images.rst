.. include:: ../js.rst

.. _tut.using-images:

#####################
Using the Image Class
#####################

In this exercise, we will learn how to use images. ICL provides two
different types of image classes: :icl:`ImgBase` is an abstract base
class for images. It provides all image information except for the
actual image pixels. :icl:`ImgBase` is derived by the
:icl:`Img`-template class, whose template parameter determines the
pixel data type. Most ICL classes are able to handle arbitrary
pixel-types, i.e. images of type :icl:`ImgBase`.

In this example we'll give only a very simple overview that allows us
to use the image classes in the next steps of the tutorial. As soon as
we need more advanced features of the image class, we will introduce
them.

+-------------------------------------------------+----------------------------------------+
|                                                 |                                        |
| .. literalinclude:: examples/using-images.cpp   | .. image:: images/using-images.png     |
|    :language: c++                               |      :scale: 80%                       |
|    :linenos:                                    |      :alt: shadow                      |
+-------------------------------------------------+----------------------------------------+


.. note::
 
   the image is still in RGB colorspace. The value domain of
   each channel is restricted to {0,255}.
