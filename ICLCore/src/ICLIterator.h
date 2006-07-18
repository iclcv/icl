#ifndef ICLITERATOR_H
#define ICLITERATOR_H

#include "ICLCore.h"

namespace icl{
  /// used to determine the upper right corner images roi
  typedef int ICLEndIterator;
  

  /// Iterator class used to iterate over an ICLs ROI pixels
  /**
  The ICLIterator is a utility to iterate line by line over
  all pixels of an ICLs ROI. The following ascii image 
  shows an images ROI.
  <pre>
    1st pixel
      |
  ....|.................... 
  ....|..ooooooooo......... ---
  ....|..ooooooooo.........  |
  ....|..ooooooooo......... iRoiH
  ....|..ooooooooo.........  |
  ....+->xoooooooo......... ---
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
  handle image ROIs using the ICLIterator image iterator
  
  <pre>
  void channel_threshold_inplace(ICL8u &im, int iTetta, int iChannel)
  {
      for(ICL8u::iterator p=im.begin(c)  ; p.inRegion() ; p++)
      {
          *p = *p > tetta ? 255 : 0;
      }
     
  }
  </pre>
  The ICLIterator<Type> is defined in the ICL<Type> as iterator.
  This offers an intuitive "stdlib-like" use.

  <h3>Using the ICLIterator as ROW-iterator</h3>
  The ICLIterator can be used as ROW-iterator too. The following example
  will explain usage:
  
  <pre>
  void copy_channel_roi_row_by_row(ICL8u &src, ICL8u &dst, int iChannel)
  {
     for(ICL8u::iterator s=src.begin(iChannel),d=dst.begin(iChannel) ; s.inRegion() ; d.incLine(), s.incLine())
     {
        memcpy(&*d,&*s,s.getROIWidth()*sizeof(iclbyte));
     }
  }
  </pre>

  <h3> Using Nested ICLIterators for Neighbourhood operations </h3>

  void channel_convolution_3x3(ICL8u &src, ICL8u &dst,iclbyte *pucMask, int iChannel)
  {
  ICL8u::iterator s=src.begin(iChannel);
     ICL8u::iterator d(s, 3, 3);
     while(s.inRegion())
     {
        iclbyte *m = pucMask;
        iclbyte *ucBuf = 0;
        for( d.reinit(s) ; d.inRegion(); d++)
        {
           ucBuf += (*d) * (*m);
        }
        *s = ucBuf;
        s++;
     }
    
  
  }

  

  void channel_convolution_3x3(ICL8u &src, ICL8u &dst,iclbyte *pucMask, int iChannel)
  {
     ICL8u::iterator s=src.begin(iChannel);
     ICL8u::iterator d(s, 3, 3);
     while(s.inRegion())
     {
        iclbyte *m = pucMask;
        iclbyte *ucBuf = 0;
        for( d.reinit(s) ; d.inRegion(); d++)
        {
           ucBuf += (*d) * (*m);
        }
        *s = ucBuf;
        s++;
     }
    
  
  }

  <h2>Performance:Efficiency</h2>
  There are 3 major ways to access the pixel data of an image.
  - using the (x,y,channel) -operator
  - using the ICLIterator
  - working directly with the channel data

  Each method has its on advantages and disadvatages:
  - the (x,y,channel) operator is very intuitive and it can be used
    to write code that functionality is very transparent to 
    other programmers. The disadvantages are, the fact, that it
    does not take care about the images ROI, and it is 
    <b>very slow</b>.
  - the ICLIterator works on single channels, so a single iterator
    provides only <b>linear</b>(line by line) access to each pixel.
    It major advantage is - that it handels the ROI internally, and
    that it is up to 5 times faster then working with the 
    (x,y,channel)-operator
  - the fastes way to process the image data is work directly
    with the data pointer received from image.getData(channel).
    In this case the programmer himself needs to take care about
    The images ROI.

  <h2>Performace:In Values</h2>
  The following example shows use of the different techniques
  to set image data of a single channel image to the value 42.
  (Times: 1.4Mhz Pentium-M machine with 512 MB-Ram, SuSe-Linux 9.3)
  <pre>
  // create a VERY large image
  ICL8u im(10000,10000,1);
  
  // 1st working with the image data (time: ~280ms)
  iclbyte *pucData = im.getData(0);
  for(int i=0;i<im.getWidth()*im.getHeight();i++){
     pucData[i]=42;
  }

  // 2nd working with the iterator (time: ~650ms)
  for(ICL8u::iterator it=im.begin(0);it!=im.end(0);it++){
    *it = 42;
  }

  // 3rd working with the (x,y,channel)-operator (time: ~2280)
  for(int x=0;x<im.getWidth();x++){
    for(int y=0;y<im.getHeight();y++){
      im(x,y,0) = 42;
    }
  }

  // for coparison:memset (time: ~150ms)
  memset(pucData,42,im.getWidth()*im.getHeight());
  
  </pre>
  <b>Note</b> Working directly on the image data, is fast in this case,
  as the implemented algorithm does not use the pixels position (x,y)
  
  
  */
  template <class Type>
    class ICLIterator{
    public:
     /// Default Constructor
     /** Creates an ICLIterator object with Type "Type"
         @param ptData pointer to the corresponding channel data
         @param iXPos x offset of the images ROI
         @param iYPos y offset of the images ROI
         @param iImageWidth width of the corresponding image
         @param iRoiWidth width of the images ROI
         @param iRoiHeight width of the images ROI
     */
    ICLIterator(Type *ptData, int iXPos,int iYPos,int iImageWidth, int iROIWidth, int iROIHeight):
       m_iImageWidth(iImageWidth),
       m_iROIWidth(iROIWidth), 
       m_iROIHeight(iROIHeight), 
       m_iLineStep(m_iImageWidth - m_iROIWidth),
       m_ptDataOrigin(ptData),
       m_ptDataCurr(ptData),
       m_ptDataEnd(ptData+iROIWidth+iROIHeight*iImageWidth),
       m_ptCurrLineEnd(ptData+iRoiWidth){}

    /// 2nd Constructor to create sub-regions of an ICL-image
    /** This 2nd constructor creates a sub-region iterator, which may be
        used e.g. for arbitrary neighbourhood operations like 
        lineare filters, medians, ...
        See the ICLIterator description for more detail.        
        @param roOrigin reference to source Iterator Object
        @param iRoiWidth width of the images ROI
        @param iRoiHeight width of the images ROI
    */

    ICLIterator(const ICLIterator<Type> &roOrigin,int iROIWidth, int iROIHeight):
       m_iImageWidth(roOrigin.m_iImageWidth),
       m_iROIWidth(iROIWidth),
       m_iROIHeight(iROIHeight),
       m_iLineStep(m_iImageWidth - m_iROIWidth)
       m_ptDataOrigin(roOrigin.m_ptDataOrigin),
       m_ptDataCurr(roOrigin.m_ptDataCurr-(iROIWidth/2)-(iROIHeight/2)*m_iImageWidth),
       m_ptDataEnd(ptData+iROIWidth+iROIHeight*m_iImageWidth),
       m_ptCurrLineEnd(ptData+iROIWidth){}
    
    /// moves the origin of the Iterator to given position
    /** @param x new x position
        @param x new y position
    */
    inline void reinit(const ICLIterator<Type> &roOrigin)
       {
          // EVALUATE !!!
          m_ptDataCurr = roOrigin.m_ptDataCurr-(m_iROIWidth/2)-(m_iROIHeight/2)*m_iImageWidth;
          m_ptDataEnd =  m_ptData+m_iROIWidth+iROIHeight*m_iImageWidth;
          m_ptCurrLineEnd = m_ptData+m_iROIWidth;
       }

    /// retuns a reference of the current pixel value
    /** changes on *p (p is of type ICLIterator) will effect
        the image data       
    */
    inline Type &operator*() const
       {
          return *m_ptDataCurr;
       }
    
    /// moves to the next iterator position
    /** The image ROI will be scanned line by line
        beginning on the bottom left iterator.
       <pre>

           +--'I' is the first invalid iterator
           |  (p.inRegion() will become false)
    .......V.................
    .......I.................
    .......++++++++X<---------- last valid pixel
    .......+++++++++.........
    .......+++++++++.........
    .......9++++++++.........
    ....+->0++-->++8.........
    ....|....................
        |
       begin here

       </pre>
       
       In most cases The ++ operator will just increase the
       current x position and update the reference to the
       current pixel data. If the end of a line is reached, then
       the position is set to the beginning of the next line.
    */
    inline operator ++(int i)
       {
          if ( m_ptDataCurr == m_ptDataLineEnd)
             {
                m_ptData += m_iLineStep;
                m_ptDataLineEnd += m_iImageWidth;
             }
          else
             {
                m_ptData++;
             }
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
          return m_iROIWidth;
       }
    
    inline int getROIHeight() const
       {
          return m_iROIHeight();
       }
    
    /// moved the pixel vertically forward
    /** current x value is hold, the current y-value is
        incremented by iLines
        @param iLines amount of lines to jump over
    */
    inline void incRow(int iLines=1) 
       {
          m_ptDataCurr += iLines * m_iImageWidth;
          m_ptDataLineEnd += iLines * m_iImageWidth;
       }

    /// returns the current x position of the iterator (image-coordinates)
    /** @return current x postion*/
    inline int x()
       {
          return (m_ptDataCurr-m_ptDataOrigin) % m_iImageWidth;
       }

    /// returns the current y position of the iterator (image-coordinates)
    /** @return current y postion*/
    inline int y()
       {
          return (m_ptDataCurr-m_ptDataOrigin) / m_iImageWidth;
       }       
    private:
    /// corresponding images width
    int m_iImageWidth;
    
    /// corresponding images ROI width
    int m_iROIWidth;

    /// corresponding images ROI height
    int m_iROIHeight;

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
}
#endif
