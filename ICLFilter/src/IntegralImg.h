#ifndef INTEGRALIMG_H
#define INTEGRALIMG_H

#include <Img.h>
#include <vector>

namespace icl{
  /// class for creating integral images
  /** 
  The integral image A of an image a (introduced by Viola & Jones (2001) in the
  paper : "Rapid Object Detection using a Boosted Cascade of Simple Features" )
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
  Because there is no stable implementation of an Img32s avilable yet, the integral image comes with
  its own fallback implementation of this calls. <b>TODO:</b> replace this class lateron
  by the Img32s/Img32f class when it is available (...). 
  The provided functions offer the ability for the definition of an optional 
  <em>borderSize</em> parameter, which is 0 by default. If it is set, The result image
  is enfolded with a border. The border pixels are filled with values as if the result
  image would be 0 there. More precisely, upper and left border pixels are set to zero
  and lower and right border pixels are a copy of the nearest <em>real</em> pixel
  like a <em>replicated border</em>.
  An application of the border-mechanism is the local threshold algorithm.

  <h1>Supported Type-Combinations</h1>
  The templates are instantiated for:
  - Img8u -> int (IPP-OPTIMIZED)
  - Img8u -> icl32f (IPP-OPTIMIZED)
  - Img32f -> icl32f
  
  <h1>Performance</h1>
  The calculation is very fast! The IPP is about twice as fast as the c++ code.
  As Example: Img8u 640x480, 1-channel, borderSize = 10, 1.4MHz Centrino: 
  - no ipp: approx. 4.4ms (inclusive memory allocation!!)
  - with ipp: approx. 2.8ms (inclusive memory allocation!!)

  The fallback implementations performance could be optimized by using the algorithm
  proposed by Viola & Jones:
  first step: percalculate a row-sum image  
  \f[
  s(x,y) = s(x-1,y) + a(x,y) \;\;with\; s(-1,y)=0
  \f]
  second step: calculate the integral image
  \f[
  A(x,y) = s(x-1,y) + A(x,y-1) \;\;with\; A(x,-1)=0
  \f]
  But since it is a fallback implementation, this optimization is not yet implemented 
  
  */
  class IntegralImg{
    public:
    
    template<class T>
    class IntImg{
      public:
      IntImg();
      IntImg(const Size &s, int channels);
      ~IntImg();
      
      T *getData(int channel);
      const T *getData(int channel) const;
      const Size &getSize() const;
      void setSize(const Size &size);
      void setChannels(int c);
      int getChannels() const;
      int getDim() const;
      
      private:
      Size m_oSize;
      std::vector<T*> m_oData;
    };

    typedef IntImg<icl32f> IntImg32f;
    typedef IntImg<int> IntImg32s;
    
    /// creates a set of integral image channels from a given image
    template<class T,class  I>
    static IntImg<I> *create(Img<T> *image,unsigned int borderSize=0, IntImg<I> *intImage=0);

  };
}

#endif
