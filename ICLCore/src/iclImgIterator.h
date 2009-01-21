#ifndef ICLITERATOR_H
#define ICLITERATOR_H

#include <iclCore.h>
#include <iterator>

namespace icl{
  /// Iterator class used to iterate though an Images ROI-pixels \ingroup IMAGE
  /** 
  The ImgIterator is a utility to iterate line by line though
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
  class ImgIterator : public std::iterator<std::forward_iterator_tag,Type>{
    private:
    inline void init () {
       m_iLineStep = m_iImageWidth - m_ROISize.width + 1;
       m_ptDataEnd = m_ptDataCurr;
       if (m_ROISize.width > 0)
          m_ptDataEnd += m_ROISize.width + (m_ROISize.height-1) * m_iImageWidth;
       m_ptCurrLineEnd = m_ptDataCurr + m_ROISize.width - 1;
    }

    public:
    
    static inline const ImgIterator<Type> create_end_roi_iterator(const Img<Type> *src, 
                                                                  int srcC, 
                                                                  const Rect &roi){
      
      ImgIterator<Type> i(const_cast<Type*>(src->getData(srcC)),src->getWidth(),roi);
      i.m_ptDataCurr = i.m_ptDataEnd - roi.width + src->getWidth();
      i.m_ptCurrLineEnd = i.m_ptDataCurr + roi.width;
      return i;
    }

    /** Creates an ImgIterator object */
    /// Default Constructor
    inline ImgIterator():
       m_iImageWidth(0),
       m_ROISize(Size::null), 
       m_ptDataOrigin(0),
       m_ptDataCurr(0) {init();}

     /** 2nd Constructor creates an ImgIterator object with Type "Type"
         @param ptData pointer to the corresponding channel data
         @param iImageWidth width of the corresponding image
         @param roROI ROI rect for the iterator
     */
    inline ImgIterator(Type *ptData,int iImageWidth,const Rect &roROI):
       m_iImageWidth(iImageWidth),
       m_ROISize(roROI.size()), 
       m_ptDataOrigin(ptData),
       m_ptDataCurr(ptData+roROI.x+roROI.y*iImageWidth) {init();}

    /// 3rd Constructor to create sub-regions of an Img-image
    /** This 3rd constructor creates a sub-region iterator, which may be
        used e.g. for arbitrary neighborhood operations like 
        linear filters, medians, ...
        See the ImgIterator description for more detail.        
        @param roOrigin reference to source Iterator Object
        @param s mask size
        @param a mask anchor
    */

    inline ImgIterator(const ImgIterator<Type> &roOrigin, const Size &s, const Point &a):
       m_iImageWidth(roOrigin.m_iImageWidth),
       m_ROISize(s), 
       m_ptDataOrigin(roOrigin.m_ptDataOrigin),
       m_ptDataCurr(roOrigin.m_ptDataCurr - a.x - a.y*m_iImageWidth) {init();}
    
    inline ImgIterator &operator=(const ImgIterator &other){
      m_iImageWidth = other.m_iImageWidth;
      m_ROISize = other.m_ROISize;
      m_iLineStep = other.m_iLineStep;
      m_ptDataOrigin = other.m_ptDataOrigin;
      m_ptDataCurr = other.m_ptDataCurr;
      m_ptDataEnd = other.m_ptDataEnd;
      m_ptCurrLineEnd = other.m_ptCurrLineEnd;
      return *this;
    }

    inline const ImgIterator& operator=(const ImgIterator &other) const{
      return (*const_cast<ImgIterator*>(this)) = other;
    }

    /// retuns a reference of the current pixel value (const)
    /** changes on *p (p is of type ImgIterator) will effect
        the image data       
    */
    inline const Type &operator*() const { return *m_ptDataCurr;  }

    /// retuns a reference of the current pixel value
    /** changes on *p (p is of type ImgIterator) will effect
        the image data       
    */
    inline Type &operator*(){  return *m_ptDataCurr;  }
    
    /// moves to the next iterator position (Prefix ++it)
    /** The image ROI will be scanned line by line
        beginning on the bottom left iterator.
       <pre>

           +-- begin here (index 0)
           |  
    .......|.................
    .......V.................
    .......012+-->+8<---------- first line wrap after 
    .......9++++++++.........   this pixel (index 8)
    .......+++++++++.........
    .......+++++++++.........
    .......++++++++X<---------- last valid pixel
    ....+->I.................
        |  
    'I' is the first invalid iterator
    (p.inRegion() will become false)
  

       </pre>
       
       In most cases The ++ operator will just increase the
       current x position and update the reference to the
       current pixel data. If the end of a line is reached, then
       the position is set to the beginning of the next line.
    */
    inline ImgIterator& operator++(){
      if ( ICL_UNLIKELY(m_ptDataCurr == m_ptCurrLineEnd) ){
        m_ptDataCurr += m_iLineStep;
        m_ptCurrLineEnd += m_iImageWidth;
      }else{
        m_ptDataCurr++;
      }
      return *this;
    }

    /// const version of pre increment operator
    inline const ImgIterator& operator++() const{
      return ++(*const_cast<ImgIterator*>(this));
    }

    /** postfix operator++ (used -O3 to avoid
        loss of performace when using the "it++"-operator
        In most cases the "++it"-operator will ensure
        best performace.
    **/
    inline ImgIterator operator++(int){
      ImgIterator current (*this);
      ++(*this); // call prefix operator
      return current; // return previous
    }

    /// const version of post increment operator
    inline const ImgIterator operator++(int) const{
      return (*const_cast<ImgIterator*>(this))++;
    }
    

    /// to check if iterator is still inside the ROI
    /** This function was replaced by STL-like begin(), end() logic
        Although in some cases it might be quite useful, so
        we renamed it rather than deleting it
        @see operator++ */
    inline bool inRegionSubROI() const
    {
      return m_ptDataCurr < m_ptDataEnd;          
    }



    /// compare two iterators
    inline bool operator!=(const ImgIterator<Type> &it) const{
      return m_ptDataCurr != it.m_ptDataCurr;
    }
    /// compare two iterators
    inline bool operator==(const ImgIterator<Type> &it) const{
      return m_ptDataCurr == it.m_ptDataCurr;
    }
    /// compare two iterators
    inline bool operator<(const ImgIterator<Type> &it) const{
      return m_ptDataCurr < it.m_ptDataCurr;
    }
    /// compare two iterators
    inline bool operator>(const ImgIterator<Type> &it) const{
      return m_ptDataCurr > it.m_ptDataCurr;
    }
    /// compare two iterators
    inline bool operator<=(const ImgIterator<Type> &it) const{
      return m_ptDataCurr <= it.m_ptDataCurr;
    }
    /// compare two iterators
    inline bool operator>=(const ImgIterator<Type> &it) const{
      return m_ptDataCurr >= it.m_ptDataCurr;
    }


    /// returns the length of each row processed by this iterator
    /** @return row length 
     */
    inline int getROIWidth() const
       {
          return m_ROISize.width;
       }
    
    inline int getROIHeight() const
       {
          return m_ROISize.height;
       }
    
    /// move the pixel vertically forward
    /** current x value is hold, the current y-value is
        incremented by iLines
        @param iLines amount of lines to jump over
    */
    inline void incRow(int iLines=1) const {
      m_ptDataCurr += iLines * m_iImageWidth;
      m_ptCurrLineEnd += iLines * m_iImageWidth;
    }

    /// returns the current x position of the iterator (image-coordinates)
    /** @return current x position*/
    inline int x(){
      return (m_ptDataCurr-m_ptDataOrigin) % m_iImageWidth;
    }

    /// returns the current y position of the iterator (image-coordinates)
    /** @return current y position*/
    inline int y(){
      return (m_ptDataCurr-m_ptDataOrigin) / m_iImageWidth;
    }       
    
    private:
    /// corresponding images width
    int m_iImageWidth;
    
    /// ROI size of the iterator
    Size m_ROISize;

    /// result of m_iImageWidth - m_iROIWidth
    int m_iLineStep;

    /// pointer to the image data pointer (bottom left pixel)
    Type *m_ptDataOrigin;

    /// pointer to the current data element
    mutable Type *m_ptDataCurr;

    /// pointer to the first invalid pixel of ptDataOrigin
    Type *m_ptDataEnd;

    /// pointer to the first invalid pixel of the current line
    mutable Type *m_ptCurrLineEnd;
    
  };
  /**
      template <typename Type>
      class ConstImgIterator : public ImgIterator<const Type> {
      public:
      /// Default Constructor: creates an empty ConstImgIterator object
      ConstImgIterator() : ImgIterator<const Type>() {}
      
      /// 2nd Constructor creates an ImgIterator object with type "Type"
      ConstImgIterator(const Type *ptData,int iImageWidth,const Rect &roROI) :
      ImgIterator<const Type>(ptData, iImageWidth, roROI) {}
      
      /// 3rd Constructor to create sub-regions of an image
      ConstImgIterator(const ConstImgIterator<Type> &roOrigin, const Size &s, const Point &a) :
      ImgIterator<const Type>(roOrigin, s, a) {}
      };
   **/
}
#endif
