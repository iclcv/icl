#include <ICLCore.h>
#ifndef ICLITERATOR_H
#define ICLITERATOR_H


namespace icl{
  /// Iterator class used to iterate over an Images (ROI-)pixels
  /**
  The ImgIterator is a utility to iterate line by line over
  all pixels of <a subrect/all pixles> of an image. The following ASCII image 
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

  the following code example shows how to
  handle image ROIs using the ImgIterator
  
  <pre>
  void channel_threshold_inplace(Img8u &im, int iTetta, int iChannel)
  {
      for(Img8u::iterator p=im.getROIIterator(c)  ; p.inRegion() ; p++)
      {
          *p = *p > tetta ? 255 : 0;
      }
     
  }
  </pre>
  The ImgIterator<Type> is defined in the Img<Type> as iterator.
  This offers an intuitive "stdlib-like" use.

  <h3>Using the ImgIterator as ROW-iterator</h3>
  The ImgIterator can be used as ROW-iterator too. The following example
  will explain usage:
  
  <pre>
  void copy_channel_roi_row_by_row(Img8u &src, Img8u &dst, int iChannel)
  {
     for(Img8u::iterator s=src.getROIIterator(iChannel),
                         d=dst.getROIIterator(iChannel) ;
         s.inRegion() ; 
         d.incLine(), s.incLine())
     {
        memcpy(&*d,&*s,s.getROIWidth()*sizeof(icl8u));
     }
  }
  </pre>

  <h3> Using Nested ImgIterators for Neighborhood operations </h3>

  In addition to the above functionalities, ImgIterators can be used for
  arbitrary image neighborhood operations like convolution, median or
  erosion. The following example explains how to create so called sub-region
  iterators, that work on a symmetrical neighborhood around a higher level
  ImgIterator.

  <pre>
  void channel_convolution_3x3(Img32f &src, Img32f &dst,icl32f *pfMask, int iChannel)
  {
     for(Img32f::iterator s=src.getROIIterator(iChannel) d=dst.getROIIterator() ; s.inRegion() ; s++,d++)
     {
        icl32f *m = pfMask;
        (*d) = 0;
        for(Img32f::iterator sR(s, 3, 3); sR.inRegion(); sR++,m++)
        {
           (*d) += (*sR) * (*m);
        }
     }  
  }
  </pre>

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
  - the fastest way to process the image data is work directly
    with the data pointer received from image.getData(channel).
    In this case the programmer himself needs to take care about
    The images ROI. This is only recommended, if no ROI-support
    should be provided.

  <h2>Performance:In Values</h2>
  The following example shows use of the different techniques
  to set image data of a single channel image to a static value.
  (Times: 1.4Mhz Pentium-M machine with 512 MB-Ram, SuSe-Linux 9.3)
  <pre>
  // create a VERY large image
  int iW = 10000, iH=10000;
  Img8u im(iW,iH,1);

  // 1st working with the image data (time: ~210/360ms)
  // pointer style (~210ms)
  for(icl8u *p= im.getData(0), *d=p+iW*iH ; p<d; ){
     *p++ = 5;
  }
  
  // index style (~360ms)
  icl8u *pucData = im.getData(0);
  for(int i=0;i<im.getWidth()*im.getHeight();i++){
     pucData[i]=42;
  }

  // 2nd working with the iterator (time: ~280ms) (further implementation ~650ms)
  for(Img8u::iterator it=im.begin(0) ; it.inRegion() ; it++){
    *it = 42;
  }

  // 3rd working with the (x,y,channel)-operator (time: ~2400)
  for(int x=0;x<im.getWidth();x++){
    for(int y=0;y<im.getHeight();y++){
      im(x,y,0) = 42;
    }
  }

  // for comparison: memset (time: ~140ms)
  memset(pucData,42,im.getWidth()*im.getHeight());
  
  </pre>
  <b>Note</b> Working directly on the image data, is fast for 
  algorithms that are not using the pixels position (x,y) or the
  images ROI.
  
  
  */
  template <typename Type>
  class ImgIterator {
    private:
    void init () {
       m_iLineStep = m_iImageWidth - m_ROISize.width + 1;
       m_ptDataEnd = m_ptDataCurr;
       if (m_ROISize.width > 0)
          m_ptDataEnd += m_ROISize.width + (m_ROISize.height-1) * m_iImageWidth;
       m_ptCurrLineEnd = m_ptDataCurr + m_ROISize.width - 1;
    }

    public:
    /** Creates an ImgIterator object */
    /// Default Constructor
    ImgIterator():
       m_iImageWidth(0),
       m_ROISize(Size::null), 
       m_ptDataOrigin(0),
       m_ptDataCurr(0) {init();}
    
     /** 2nd Constructor creates an ImgIterator object with Type "Type"
         @param ptData pointer to the corresponding channel data
         @param iImageWidth width of the corresponding image
         @param roROI ROI rect for the iterator
     */
    ImgIterator(Type *ptData,int iImageWidth,const Rect &roROI):
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

    ImgIterator(const ImgIterator<Type> &roOrigin, const Size &s, const Point &a):
       m_iImageWidth(roOrigin.m_iImageWidth),
       m_ROISize(s), 
       m_ptDataOrigin(roOrigin.m_ptDataOrigin),
       m_ptDataCurr(roOrigin.m_ptDataCurr - a.x - a.y*m_iImageWidth) {init();}
    
    /// retuns a reference of the current pixel value
    /** changes on *p (p is of type ImgIterator) will effect
        the image data       
    */
    inline Type &operator*() const
       {
          return *m_ptDataCurr;
       }
    
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
    inline ImgIterator& operator++()
       {
         if ( ICL_UNLIKELY(m_ptDataCurr == m_ptCurrLineEnd) )
           {
             m_ptDataCurr += m_iLineStep;
             m_ptCurrLineEnd += m_iImageWidth;
           }
         else
           {
             m_ptDataCurr++;
           }
         return *this;
       }
    /** postfix operator++ (used -O3 to avoid
        loss of performace when using the "it++"-operator
        In most cases the "++it"-operator will ensure
        best performace.
    **/
    inline const ImgIterator operator++(int)
       {
         ImgIterator current (*this);
         ++(*this); // call prefix operator
         return current; // return previous
       }

    /// to check if iterator is still inside the ROI
    /** @see operator++ */
    inline bool inRegion() const
       {
          return m_ptDataCurr < m_ptDataEnd;          
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
    inline void incRow(int iLines=1) 
       {
          m_ptDataCurr += iLines * m_iImageWidth;
          m_ptCurrLineEnd += iLines * m_iImageWidth;
       }

    /// returns the current x position of the iterator (image-coordinates)
    /** @return current x position*/
    inline int x()
       {
          return (m_ptDataCurr-m_ptDataOrigin) % m_iImageWidth;
       }

    /// returns the current y position of the iterator (image-coordinates)
    /** @return current y position*/
    inline int y()
       {
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
    Type *m_ptDataCurr;

    /// pointer to the first invalid pixel of ptDataOrigin
    Type *m_ptDataEnd;

    /// pointer to the first invalid pixel of the current line
    Type *m_ptCurrLineEnd;
    
  };

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
}
#endif
