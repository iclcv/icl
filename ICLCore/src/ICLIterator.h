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
  void channel_threshold_inplace(ICL8u &im, int tetta)
  {
    for(int c=0;c<3;c++)
    {
       for(ICL8u::iterator p=im.begin(c);p!=im.end(c);p++)
       {
         *p = *p > tetta ? 255 : 0;
       }
    }
  }
  </pre>

  The ROIs end(c) is represented by a single integer value
  "typedef'ed" to ICLEndIterator, which the value of the first
  invalid y-value for a ROI-pixel: If the last line is finished,
  the ++-operator will increase y to this value, and
  <pre>
  p!=im.end(c)
  </pre>
  will return false, which will end the loop.

  The ICLIterator<Type> is defined in the ICL<Type> as iterator.
  This offers an intuitive "stdlib-like" use.

  <h3>Using the ICLIterator as ROW-iterator</h3>
  The ICLIterator can be used as ROW-iterator too. The following example
  will explain usage:
  <pre>
  void copy_channel_roi_row_by_row(ICL8u &src, ICL8u &dst, int iChannel)
  {
     for(ICL8u::iterator s=src.begin(iChannel),d=dst.begin(iChannel) ;s!=src.end(iChannel);d.y++,s.y++)
     {
        memcpy(&*d,&*s,s.getRowLen()*sizeof(iclbyte));
     }
  }
  </pre>

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
        @param iXStart x offset of the images ROI
        @param iYStart y offset of the images ROI
        @param ptData pointer to the corresponding channel data
        @param iImageW width of the corresponding image
        @param iRoiW width of the images ROI
    */
    ICLIterator(int iXStart,int iYStart,Type *ptData, int iImageW, int iRoiW):
      x(iXStart),y(iYStart),ptData(ptData),
      iImageW(iImageW),iXEnd(iXStart+iRoiW),iXStart(iXStart){}
    
    /// current x location of the iterator (with image origin)
    int x;

    /// current y location of the iterator (with image origin)
    int y;

    /// retuns a reference of the current pixel value
    /** changes on *p (p is of type ICLIterator) will effect
        the image data       
    */
    inline Type &operator*()
      {
        return ptData[x+iImageW*y];
      }
    
    /// moves to the next iterator position
    /** The image ROI will be scanned line by line
        beginning on the bottom left iterator.
       <pre>

           +--'I' is the first invalid iterator
           |  (p!=image.end() will become false)
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
    inline void operator++(int i)
      {
        (void)i;
        x++;
        if(x>=iXEnd){
          x=iXStart;
          y++;
        }     
      }
    /// prefix as the above postfix
    inline void operator++()
      {
        x++;
        if(x>=iXEnd){
          x=iXStart;
          y++;
        }     
      }
    /// to check if iterator is still inside the ROI
    /** @see operator++ */
    inline int operator!=(ICLEndIterator end)
      {
        return y!=end;
      }    
    inline int getRowLen()
      {
        return iXEnd-iXStart;
      }
      private:
    
    /// internal reference for the current pixel value
    Type *ptData;
    int iImageW,iXEnd,iXStart;
  };
}
#endif
