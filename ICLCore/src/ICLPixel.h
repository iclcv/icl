#ifndef ICLPIXEL_H
#define ICLPIXEL_H

#include "ICLCore.h"

namespace icl{
  /// used to determine the upper right corner images roi
  typedef int ICLEndPixel;
  

  /// Pixel class used to iterate over an ICLs ROI pixels
  /**
  The ICLPixel is a utility to iterate line by line over
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
  handle image ROIs using the ICLPixel image iterator
  
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
  "typedef'ed" to ICLEndPixel. This in has the value of the first
  not-valid y-value for a pixel: If the last line is finished,
  the ++-operator will increase y to this value, and
  <pre>
  p!=im.end(c)
  </pre>
  will return false, which will force the loop to be finished.

  The ICLPixel<Type> is defined in the ICL<Type> as iterator.
  This offers an intuitive "std-lib-like" use.
  */
  template <class Type>
    class ICLPixel{
      public:
    /// Default Constructor
    /** Creates an ICLPixel object with Type "Type"
        @param iXStart x offset of the images ROI
        @param iYStart y offset of the images ROI
        @param ptData pointer to the corresponding channel data
        @param iImageW width of the corresponding image
        @param iRoiW width of the images ROI
    */
    ICLPixel(int iXStart,int iYStart,Type *ptData, int iImageW, int iRoiW):
      x(iXStart),y(iYStart),val(ptData[iXStart+iImageW*iYStart]),ptData(ptData),
      iImageW(iImageW),iXEnd(iXStart+iRoiW),iDX(iImageW-iRoiW){}
    
    /// current x location of the pixel (with image origin)
    int x;

    /// current y location of the pixel (with image origin)
    int y;

    /// retuns a reference of the current pixel value
    /** changes on *p (p is of type ICLPixel) will effect
        the image data       
    */
    inline Type &operator*()
      {
        return val;
      }
    
    /// moves to the next pixel position
    /** The image ROI will be scanned line by line
        beginning on the bottom left pixel.
       <pre>

           +--'I' is the first invalid pixel
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
    inline void operator++()
      {
        x++;
        if(x>iXEnd){
          x-=iDX;
          y++;
        }
        val = ptData[x+iImageW*y];
      }
    /// to check if pixel is still inside the ROI
    /** @see operator++ */
    inline int operator!=(ICLEndPixel end)
      {
        return y!=end;
      }
    
      private:
    
    /// internal reference for the current pixel value
    Type &val;
    Type *ptData;
    int iImageW,iXEnd,iDX;
  };
}
#endif
