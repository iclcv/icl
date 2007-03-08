#include <ICLGeneralModel.h>
#include <ICLQuadraticModel.h>
#ifndef ICLMEX_H
#define ICLMEX_H


/** \mainpage ICLMEx (ICL Model Extraction package)

The ICLMex library currently provides functions to extract parameterized
models from given sets of x and y coordinates. The algorithms base on 
the paper <em><b>Direct Least Square Fitting of Ellipses</b></em> of 
<em>Andrew W. Fitzgibbon</em>, <em>Maurizio Milu</em> and <em>Robert B.
Fischer</em>.
The basic structure is the <em>GeneralModel</em> class. It formalizes all
information needed to extract a model from a given set of x and y positions.
<b>NOTE: A further todo will be to generalize the algorithms to an abitrary
input feature dimension.</b> Yet, no generalization is implemented, so
the model extraction functions and data structures are restricted to 2D 
problems.
The following will give a small insight of the used algorithm:
<h1>The direct least square fitting algorithm</h1>
In the following, models are formalized by \f$ F=AX=0 \f$, 
where the vector \f$\vec{X}=(f_1(x,y),f_2(x,y),...)^T\f$ contains combined 
features of the input data \f$x \;and \;y\f$, and the components \f$a_i\f$ of 
\f$\vec{A}=(a_1,a_2,a_3,...)^T\f$ are coefficient to the features. So models 
are expressed as the intersection of polynoms over x and y and 
the \f$F=0\f$ plane. The following examples shall illustrate this:

<h2>model examples</h2>
In the following five examples are given for the primitives: 
<em>ellipse</em>, <em>so called restricted ellipse</em>, <em>circle</em>,
<em>straight line</em> and <em>straight lines visiting the origin</em>

<h3>ellipse models</h3>
an elliptic model for example can be parameterized as follows:
\f[
ax^2 + bxy + cy^2 + dx + ey + f = 0;
\f]
this can be rewritten using the above notation:
\f[
F(A,X)=0 \; with \; A=(a,b,c,d,e,f)^T \; and \; X=(x^2,xy,y^2,x,y,1)^T
\f]

<h3>restricted ellipse models</h3>
If we leave out the ellipse models mixed term (b), the major axis of
the ellipse remain one of the x- and the y-axis. We restate the formula
as follows:
\f[
F(A,X)=0 \; with \; A=(a,b,c,d,e)^T \; and \; X=(x^2,y^2,x,y,1)^T
\f]

<h3>circle models</h3>
An additional restriction of the ellipse model will forbid the 
<em>ellipse</em> to have different stretch factors along the two major
axis, which complies a circle model:
\f[
F(A,X)=0 \; with \; A=(a,b,c,d)^T \; and \; X=(x^2+y^2,x,y,1)^T
\f]

<h3>straight line models</h3>
If we leave out the quadratic Terms of the above model, we get a
model for straight lines:
\f[
F(A,X)=0 \; with \; A=(a,b,c)^T \; and \; X=(x,y,1)^T
\f]
in this case the vector \f$(a,b)^T\f$ complies the normal vector of the
straight line, and the distance to the origin is defined by c

<h3>straight line through the origin models</h3>
The above models coefficient c can be left out, to get a model
which restricts the lines to visit the origin.
\f[
F(A,X)=0 \; with \; A=(a,b)^T \; and \; X=(x,y)^T
\f]

<h1>Direct least square fitting</h1>
The following shall give a coarse insight in the implemented algorithms; further
information can be found in the above mentioned paper.
We have a set of data points coordinates \f$\vec{x}\f$ and \f$\vec{y}\f$, and
want to minimize the function:
\f[
\|Da\|^2
\f]
where D is the so called design matrix, where row i complies \f$X(x_i,y_i)\f$. So 
D is in case of searching ellipses:
\f[
D=\left(\begin{array}{cccccc}
x_1^2 & x_1y_1 & y_1^2 & x_1 & y_1 & 1\\
x_2^2 & x_2y_2 & y_2^2 & x_2 & y_2 & 2\\
x_3^2 & x_3y_3 & y_3^2 & x_3 & y_3 & 3\\
& & ... & & & \\
x_N^2 & x_Ny_N & y_N^2 & x_N & y_N & N\\
\end{array}\right)
\f]

To forbid the minimization procedure to find the trivial solution \f$A=0\f$
we have to introduce a so called constraint matrix \f$C\f$, which is in the
simplest case just the \f$dim(A)\times dim(A)\f$ identity matrix. The minimization
can be tackled as a constraint problem introducing Lagrange multipliers: Minimize
\f$E=\|Da\|^2\f$ subject to \f$a^TCa=1\f$. In case of \f$C=Id_{dim(A)}\f$ this
constraint is equivalent to the constraint \f$\|a\|^2=1\f$. The above mentioned 
paper discusses also other choices for \f$C\f$. Introducing Lagrange multipliers
\f$\lambda\f$ we arrive the System:

\f[
\begin{array}{r}
2D^TDa-2\lambda Ca = 0 \\
a^TCa = 1
\end{array}
\f]

By further introducing the so called <em>scatter matrix</em> \f$S=D^TD\f$ this 
can be restated as follows:
\f[
\begin{array}{r}
Sa = \lambda Ca = 0 \\
a^TCa = 1
\end{array}
\f]
So we have to solve a generalized eigenvector problem. More details should not be
discussed here.

<h1>Implemented algorithm</h1>
The paper gives a short summary of the algorithm to simplify implementation in 
terms of a 6-line MatLab implementation, which is sketched in the following:
-# create the Design matrix \f$D\f$
-# calculate \f$S=D^TD\f$
-# create the constraint matrix \f$C\f$ (here \f$Id\f$)
-# find eigenvectors and eigenvalues of \f$S^{-1}C\f$
-# find the largest valid eigenvalue \f$\lambda_{max}\f$
-# optimal parameters are the components of the eigenvector \f$\hat{\nu}_{max}\f$ 
*/
namespace icl{

  /// fits a model to given xs and ys coordinate vector 
  /** after the call, the given model reference has the found parameters.
      This parameters can easily read out with the models "[]" operator.
      @see GeneralizedModel
  */
  template<class T>
  void fitModel(T *xs, T *ys, unsigned int nPoints, GeneralModel<T> &model);


  /// this fitModel function implements a metha algorithm to get a better robustness against non gaussian outliers
  /** if subSetCoutn == -1 it is set to the dim of the model  
      A simple idea to become more robust against non gaussian outliers, this
      implementation takes subSetCount subsets of the given xs, and ys sets, of size subSetSize,
      and calculates the seached model params by using the above function for this subset. Single outliers
      will effect only a small percentage of all of the evaluated subsets. The rule of thumb is to 
      estimate the median of each parameter for the result.
      (@see Paper: Further Five Point Fit Ellipse Fitting by Paul L. Rosin)
  */
  template<class T>
  void fitModel(T *xs, T *ys,unsigned int nPoints, GeneralModel<T> &model, int subSetSize, int subSetCount=-1);
  
  /// draws a model with calculated params 
  /** The model must be fitted with the above function before it can be drawed
      into an image. 
  */
  template<class T, class X>
  void drawModel(GeneralModel<T> &model, Img<X> *image, X *color);
  
  /// draws a model with calculated params 
  /** The model must be fitted with the above function before it can be drawed
      into an image. 
  */
  template<class T, class X>
  void drawQuadraticModel(QuadraticModel<T> &model, Img<X> *image, X *color);
}


#endif
