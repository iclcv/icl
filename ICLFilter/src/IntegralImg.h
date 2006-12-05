#ifndef INTEGRALIMG_H
#define INTEGRALIMG_H

#include <Img.h>
#include <vector>

namespace icl{
  /// class for creating integral images
  /** 
  The integral image A of an image a (introduced by viola and jones ...I think) is
  defined as follows:
  \f[
  A(i,j) = \sum\limits_{x=0}^i \sum\limits_{y=0}^j  a(i,j)
  \f]
  It can be calculated incrementally using the following equation:
  \f[
  A(i,j) = a(i,j)+A(i-1,j)+A(i,j-1)-A(i-1,j-1)
  \f]
  <h3>illustration:</h3>
  <pre>
    +++++..
    +++CA..
    +++BX..
    .......
    X = src(x) + A + B - C
  </pre>

  <h1>Implementation<h1>
  <h3>Other definitions</h3>
  There are alternative definitions (like the ipp): 
   \f[
  A(i,j) = \sum\limits_{x=0}^{i-1} \sum\limits_{y=0}^{j-1}  a(i,j)
  \f]
  From this it follows, that the first integral image row and colum is filled with
  zeros and the internal image grows by one pixel in each dimension, with the benefit
  of some better usability of the result image. We use the above definition, which
  is a bit more convenient for the most common applications in our sight.
  
  <h3>functions and borders</h3>
  The provided functions offer the ability for the definition of an optional 
  <em>borderSize</em> parameter, which is 0 by default. If it is set, The result image
  is enfolded with a border. The border pixels are filled with values as if the result
  image would be 0 there. More precisely, upper and left border pixels are set to zero
  and lower and right border pixels are a copy of the nearest <em>real</em> pixel
  like a <em>replicated border</em>.
  An application of the border-mechanism is the local threshold algorithm.

  <h1>Supported Type-Combinations</h2>
  The templates are instantiated for:
  - Img8u -> int (IPP-OPTIMIZED)
  - Img8u -> icl32f (IPP-OPTIMIZED)
  - Img32f -> icl32f
  
  <h1>Performance</h1>
  The calculation is very fast! The IPP is about twice as fast as the c++ code.
  As Example: Img8u 640x480, 1-channel, borderSize = 10, 1.4MHz Centrino: 
  - no ipp: approx. 4.4ms (inclusive memory allocation!!)
  - with ipp: approx. 2.8ms (inclusive memory allocation!!)
  */
  class IntegralImg{
    public:
    /// creates a set of integral image channels from a given image
    template<class T,class  I>
    static std::vector<I*> create(Img<T> *image,std::vector<I*> &integralImageChannels,unsigned int borderSize=0);

    /// creates a set of integral image channels from a given image
    template<class T,class  I>
    static std::vector<I*> create(Img<T> *image,unsigned int borderSize=0);
  };
}

#endif
