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
  show an image roi.
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
  it is necessary perform calculation for each roi
  pixel. To achieve that the programmer needs to
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
  "typedef'ed" to ICLEndIterator. This in has the value of the first
  not-valid y-value for a pixel: If the last line is finished,
  the ++-operator will increase y to this value, and
  <pre>
  p!=im.end(c)
  </pre>
  will return false, which will force the loop to be finished.

  The ICLIterator<Type> is defined in the ICL<Type> as iterator.
  This offers an intuitive "std-lib-like" use.

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
