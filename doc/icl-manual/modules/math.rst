.. include:: ../js.rst

.. _math:

###################################
Linear Algebra and Machine Learning
###################################

.. image:: /icons/185px/math.png



The Math module provides support classes for linear algebra as well as
some basic machine learning tools. Like the Utils module, Math is
independent from the Core module and therefore not directly correlated
to computer vision only.


Table of Contents
"""""""""""""""""

* :ref:`math.dyn`
* :ref:`math.fixed`
* :ref:`math.lma`
* :ref:`math.fft`
* :ref:`math.ransac`
* :ref:`math.som`
* :ref:`math.llm`
* :ref:`math.simplex`
* :ref:`math.stochastic`
* :ref:`math.vq`
* :ref:`math.model`
* :ref:`math.polyreg`
* :ref:`math.trees`


.. _math.dyn:

The **DynMatrix** class
"""""""""""""""""""""""

The :icl:`DynMatrix` template is one of ICL' fundamental utlity
classes for linear algebra. It provides an intuitive object-based
interface for handling 2D matrices with a *templated* element type.
:icl:`DynMatrix` instances provide 1D and 2D element access (using the
index or 2D-function operator) and also the ability to shallowly
wrapping around existing data-pointers.

.. note:: 
 
  It is very important that all ICL-Matrix classes use image-like
  2D indexing, which is exactly the opposite of the common math-style
  indexing, which would mean **Matrix(row,column)** i.e. (y,x).
  In ICL, matrix 2D indices are (x,y), i.e. **Matrix(column,row)**

In addition to the standard mathematical operators, such as +,-,*=,
\... we support a large set of higher level functions:

* QR and RQ decomposition (:icl:`DynMatrix::decompose_QR` and :icl:`DynMatrix::decompose_RQ`) 
* LU decomposition (:icl:`DynMatrix::decompose_LU`)
* matrix inverse (:icl:`DynMatrix::inv`)
* matrix pseudo-inverse (:icl:`DynMatrix::pinv`)
* eigenvalue decomposition (:icl:`DynMatrix::eigen`)
* singular value decomposition (SVD) (:icl:`DynMatrix::svd`)
* linear equation solving (:icl:`DynMatrix::solve`)

  * LU decomposition  based
  * SVD based
  * matrix inverse based
  * QR decomposition  based

* matrix trace (:icl:`DynMatrix::trace`)
* matrix condition (:icl:`DynMatrix::cond`)
* matrix determinant (:icl:`DynMatrix::det`)

:icl:`DynRowVector`

  due to the row-major data layout of the :icl:`DynMatrix` class, the
  :icl:`DynRowVector` was realized by a simple type-def, to 
  :icl:`DynMatrix` instances, that shallowly wraps the matrix's row
  data pointer

:icl:`DynColVector`

  extends the :icl:`DynMatrix` class by restricting instances to one column

.. _math.fixed:

The **FixedMatrix** class
"""""""""""""""""""""""""

The :icl:`FixedMatrix` template shows the same
behavior as the :icl:`DynMatrix<T>` template, except for the fact, that
it uses a fixed data-array instead of a dynamic on. By these means,
:icl:`FixedMatrix` instances can be created on the stack without the need
for allocating dynamic data from the heap. Even though C++ allows for
implementing abstract matrix classes that use template parameters to
switch between static and dynamic data handling (as e.g. done in the
Eigen matrix library), we decided to keep things simpler for the user
by providing two separate matrix classes.

Again, also vector classes are derived from this matrix class.
:icl:`FixedColVector` and :icl:`FixedRowVector` are defined in
**ICLUtils/FixedVector.h**

The :icl:`FixedMatrix` class is used for *typedefs* in several other
packages. I.e.:

* :icl:`core::Color` is a *typedef* to **FixedColVector<icl8u,3>**
* :icl:`geom::Vec` is a *typedef* to **FixedColVector<float,4>**
* :icl:`geom::Mat` is a *typedef* to **FixedMatrix<float,4,4>**


.. _math.lma:

Levenberg Marquard Optimizer
""""""""""""""""""""""""""""

The :icl:`LevenbergMarquardtFitter` is a generic implementation
of the Levenberg Marquardt algorithm for non-linear parameter fitting.
The implementation can either use an analytic Jacobian or can be told
to derive a numerical Jacobinan automatically. The class documentation
provides several useful examples for different kinds of target
functions.


.. _math.fft:

Fast Fourier Transform (FFT)
""""""""""""""""""""""""""""

The FFT package provides a huge set of 1D and 2D functions for Fast
Fourier Transformation and several support functions for vectors and
matrices with real and even complex data types. Internally, the
FFT-framework uses Intel IPP and the Intel MKL if available for a
significant speed up. All FFT support functions and classes are 
in the inner namespace :icl:`math::fft`.

.. note:: 
   
   For image-FFT, a less general, but easier to use FFT-Filter is
   provided in the ICLFilter module: :icl:`FFTOp`





.. _math.ransac:

Generic RANSAC Optimization
"""""""""""""""""""""""""""

The :icl:`RansacFitter` can be used for RANSAC based function
fitting and optimization. It is implemented with template parameters
for the vector-types for sample points and for the model parameter
set. Here is an example

.. literalinclude:: examples/ransac.cpp
  :language: c++
  :linenos: 


.. _math.som:

Generic Self Organizing (SOM)
"""""""""""""""""""""""""""""

The **math::SOM** and it's derived class **math::SOM2D** are generic
self organizing map implementations. An algorithm overview is given
in http://en.wikipedia.org/wiki/Self-organizing_map


.. _math.llm:

Local Linear Maps Network (LLM)
"""""""""""""""""""""""""""""""

The Local Linear Map algorithm can be used for general regression
tasks.  ICL provides two sample applications that use the icl::`LLM`
network for 1D to 1D and from 2D to 3D regression tasks.


.. _math.simplex:

Simplex Optimizer
"""""""""""""""""

The Simplex (Downhill) algorithm is a very concise, yet powerful
search algorithm, that can be used to minimize arbitrary well shaped
functions. Our implementation, the :icl:`SimplexOptimizer` is
implemented as a generic template that abstracts from the used scalar
and vector types. Except for some optimization parameters, it can be
instantiated by just passing an arbitrary error-function and an
initial position in the search space.

.. _math.stochastic:

Stochastic Optimizer
""""""""""""""""""""

The :icl:`StochasticOptimizer` is a rather old implementation for 
stochastic search processes. It defines a virtual class interface, that
can be implemented for specific optimization tasks.


.. _math.vq:

Vector Quantisation
"""""""""""""""""""

This algorithm is implemented by the :icl:`KMeans` class template.
The class is implemented as a generic template, and can therefore
be used for different Scalar and Vector types.

.. _math.model:

Least-Square based Model Fitting
""""""""""""""""""""""""""""""""

The :icl:`LeastSquareModelFitting` is a
generic implementation of the direct-least-square fitting approach
presented in the paper *Direct Least Square Fitting of Ellipses* by
*Andrew W. Fitzgibbon et. al.*.


.. _math.polyreg:

Generic Polynomial Regression
"""""""""""""""""""""""""""""

The :icl:`PolynomialRegression` is a
generic template-based implementation for a polynomial regression
network. It provides a string-based interface to define regression
parameters.

.. _math.trees:

QuadTree and Octree classes
"""""""""""""""""""""""""""

The :icl:`QuadTree` and the :icl:`Octree` class templates provides an
extremly fast interface for inserting points, nearest-neighbor search
and aproximate nearest-neighbor search. In benchmarks, the octree was
magnitudes faster then a comparable pcl-octree implementation. However,
we buy this speedup by the loss of the ability to check whether a newly
added point is already contained in the octree. 

The classes are implemented as a template for **float** and **double**
point types and it uses an extra template parameter **CAPACITY** that
defines the number of points, that can be stored in each node of the
tree. It basically provides optimized functions for

* point insertion
* nearest neighbor search
* querying a rectangular sub-region




