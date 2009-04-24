#ifndef ICLITERATOR_H
#define ICLITERATOR_H

#include <iclCore.h>
#include <iclMatrixSubRectIterator.h>

namespace icl{
  /// Iterator class used to iterate through an Images ROI-pixels \ingroup IMAGE
  /** 
  The ImgIterator is a utility to iterate line by line through
  all ROI-pixels of an image. The following ASCII image 
  shows an images ROI.
  <pre>
    1st pixel
      |
  ....|.................... 
  ....+->Xoooooooo......... ---
  .......ooooooooo.........  |
  .......ooooooooo......... iRoiH
  .......ooooooooo.........  |
  .......ooooooooo......... ---
  ......................... 
         |-iRoiW-|
  |---------iImageW-------|
  
  </pre>
  
  For image operation like thresholding or filters,
  it is necessary perform calculation for each ROI-
  pixel. To achieve that, the programmer needs to
  Take care about:
     - xoffset
     - yoffset
     - step to jump if right border of the roi 
       is reached (imageW-roiW). current x must be reset
       to the xoffset, and y must be increased by 1
     - check of the last valid pixel position

  The following code examples demonstrate how to
  handle image ROIs using the ImgIterator drawing on the 
  example of a "find-the-min-pixel"-function.
  The example can be found in 
  "ICLCore/examples/img-iterator-benchmark.cpp"
  
  \subsection IPP IPP-Performance
      Ok, not very meaningful, but state of the art!
      If ICL is compiled without IPP the builtin-getMin()-
      function uses std::min_element instead. By the way, 
      IPP-performance is ... legendary -- it's about
      <b>20-times</b> faster than the C++ fallback!!
\code
icl8u find_min_ipp(const Img8u &i){
  return i.getMin();
}
\endcode

  \subsection STL1 std::min_element without ROI-iterator
      Just for comparison, without roi-handling   
\code
icl8u find_min_pointer_stl(const Img8u &i){
  return  *std::min_element(i.begin(0),i.end(0));
}
\endcode

  \subsection STL2 std::min_element
      Quite easy to use, but surprisingly not slower than the
      version above, that does not care about ROIs.
icl8u find_min_iterator_stl(const Img8u &i){
  return *std::min_element(i.beginROI(0),i.endROI(0));
}
      
  \subsection CPP1 C++ pointer version 
      This is just a reimplementation of std::min_element,
      so it's performance is comparable
\code
icl8u find_min_pointer_cpp(const Img8u &i){
  const icl8u *p = i.begin(0);
  const icl8u *end = p+i.getDim();
  icl8u minVal = *p++;
  for(;p!=end;++p){
     if(*p < minVal) minVal = *p;
  }
  return minVal;
}
\endcode

  \subsection CPP2 C++ Iterator Version
      Why ever -- this is a bit slower than the 
      implementation of std::min_element?
\code
icl8u find_min_iterator_cpp(const Img8u &i){
  Img8u::roi_iterator p = i.beginROI(0);
  Img8u::const_roi_iterator end = i.endROI(0);
  Img8u::roi_iterator minIt = p;
  while(++p != end){
    if(*p < *minIt) minIt = p;
  }
  return *minIt;
}
\endcode

  \subsection CC2 C++ Iterator Version using inRegionSubROI() (OLD-Style)
      To compare performance with older iterator use, this 
      function version is also listed here.
\code
icl8u find_min_iterator_cpp_inRegion(const Img8u &i){
  Img8u::roi_iterator p = i.beginROI(0);
  icl8u minVal = *p++;
  for(;p.inRegionSubROI();++p){
    if(*p<minVal) minVal = *p;
  }
  return minVal;
}
\endcode
      

  \section PERF Performance
  Ok, now let's have a look on the numbers: Here are the
  results of the demo example icl-img-iterator-benchmark for
  a Core-2-Duo with 2GHz and an input image of 1000x1000 pixels:

  - <b>STL functions</b>: 1.4ms (iterator is just as fast as the pointer)
  - <b>using icl8u* directly</b> also 1.4ms (this is no surprise)
  - <b>using iterator directly</b> 1.8ms a little bit slower
  - <b>using (old) inRegion()</b> 2ms another little bit slower
  - <b>using IPP</b> 0.073ms (applause!)

  The ImgIterator<Type> is defined in the Img<Type> as roi_iterator.
  This offers an intuitive "stdlib-like" use.

  <h3>Using the ImgIterator as ROW-iterator</h3>
  The ImgIterator can be used as ROW-iterator too. Just 
  call incRow() to move the iterator in y-direction
  
      
  <h3> Using Nested ImgIterators for Neighborhood operations </h3>

  In addition to the above functionalities, ImgIterators can be used for
  arbitrary image neighborhood operations like convolution, median or
  erosion. The following example explains how to create so called sub-region
  iterators, that work on a symmetrical neighborhood around a higher level
  ImgIterator.

\code
    template<class KernelType, class SrcType, class DstType>
    void generic_cpp_convolution(const Img<SrcType> &src, Img<DstType> &dst,const KernelType *k, ConvolutionOp &op, int c){
      const ImgIterator<SrcType> s(const_cast<SrcType*>(src.getData(c)), src.getWidth(),Rect(op.getROIOffset(), dst.getROISize()));
      const ImgIterator<SrcType> sEnd = ImgIterator<SrcType>::create_end_roi_iterator(&src,c, Rect(op.getROIOffset(), dst.getROISize()));
      ImgIterator<DstType>      d = dst.beginROI(c);
      Point an = op.getAnchor();
      Size si = op.getMaskSize();
      int factor = op.getKernel().getFactor();
      for(; s != sEnd; ++s){
        const KernelType *m = k; 
        KernelType buffer = 0;
        for(const ImgIterator<SrcType> sR (s,si,an);sR.inRegionSubROI(); ++sR, ++m){
          buffer += (*m) * (KernelType)(*sR);
        }
        *d++ = clipped_cast<KernelType, DstType>(buffer / factor);
      }
    }
\endcode

  This code implements a single channel image convolution operation. 


  <h2>Performance:Efficiency</h2>
  There are 3 major ways to access the pixel data of an image.
  - using the (x,y,channel) -operator
  - using the ImgIterator
  - working directly with the channel data

  Each method has its on advantages and disadvantages:
  - the (x,y,channel) operator is very intuitive and it can be used
    to write code whiches functionality is very transparent to 
    other programmers. The disadvantages are:
    - no implicit ROI - support
    - <b>very slow</b>
  - the ImgIterator moves pixel-by-pixel, line-by-line over
    a single image channel. It is highly optimized for processing
    each pixel of an images ROI without respect to the particular
    pixel position in in the image.
    Its advantages are:
     - internal optimized ROI handling
     - direct access to sub-ROIS
     - fast (nearly 10 times faster then the (x,y,channel)-operator
    <b>if used correctly (e.g as input for STL-functions) iterators
       are just as fast as using simple pointers!</b>
  - the fastest way to process the image data is work directly
    with the data pointer received from image.getData(channel).
    In this case the programmer himself needs to take care about
    The images ROI. This is only recommended, if no ROI-support
    should be provided.

      \section CONST const-ness
      Please note that the const-ness of an ImgIterator instance does
      not say anything about the sturcture itselft. Hence also const 
      ImgIterators can be 'moved' using ++-operators or incRow()
      method.\n
      Instead, const-ness relates to the underlying image, which data
      is referenced by the iterator instance. A const image provides
      only const ImgIterators which do not allow to change the image data.

  */
  template <typename Type>
  class ImgIterator : public MatrixSubRectIterator<Type>{
    public:
    
    static inline const ImgIterator<Type> create_end_roi_iterator(const Type *data,
                                                                  int width,
                                                                  const Rect &roi){
      ImgIterator<Type> i(const_cast<Type*>(data),width,roi);
      i.m_dataCurr = i.m_dataEnd - roi.width + width;
      i.m_currLineEnd = i.m_dataCurr + roi.width;
      return i;
    }

    /** Creates an ImgIterator object */
    /// Default Constructor
    inline ImgIterator(){}

     /** 2nd Constructor creates an ImgIterator object with Type "Type"
         @param ptData pointer to the corresponding channel data
         @param iImageWidth width of the corresponding image
         @param roROI ROI rect for the iterator
     */
    inline ImgIterator(Type *data,int imageWidth,const Rect &roi):
    MatrixSubRectIterator<Type>(data,imageWidth,roi.x,roi.y,roi.width,roi.height){}

    /// 3rd Constructor to create sub-regions of an Img-image
    /** This 3rd constructor creates a sub-region iterator, which may be
        used e.g. for arbitrary neighborhood operations like 
        linear filters, medians, ...
        See the ImgIterator description for more detail.        
        @param roOrigin reference to source Iterator Object
        @param s mask size
        @param a mask anchor
    */

    inline ImgIterator(const ImgIterator<Type> &origin, const Size &s, const Point &a){
      *this = origin;
      ImgIterator<Type>::m_dataCurr = origin.m_dataCurr - a.x - a.y * origin.m_matrixWidth;
      ImgIterator<Type>::init();
    }

    /// to check if iterator is still inside the ROI
    /** This function was replaced by STL-like begin(), end() logic
        Although in some cases it might be quite useful, so
        we renamed it rather than deleting it
        @see operator++ */
    inline bool inRegionSubROI() const{
      return ImgIterator<Type>::inSubRect();
    }

    /// Allows to assign const instances
    inline ImgIterator<Type> &operator=(const MatrixSubRectIterator<Type> &other){
      MatrixSubRectIterator<Type>::operator=(other);
      return *this;
    }
    
    /// Allows to assign const instances
    inline const ImgIterator<Type> &operator=(const MatrixSubRectIterator<Type> &other) const{
      MatrixSubRectIterator<Type>::operator=(other);
      return *this;
    }


  };
}
#endif
