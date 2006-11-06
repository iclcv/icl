#ifndef IMG_BORDER_H
#define IMG_BORDER_H

#include "Img.h"

namespace icl{

  /// Class to setup an images border pixels
  /** Border pixels are all pixels, that are not within the images ROI.
      The ImgBorder class provides three different strategies to setup
      these pixels with defined values:
  
      <h3>fixed</h3>
      This strategy sets all pixels to a fixed value, that is given by
      a 2nd argument to the "fixed"-method. To optimize performace,
      this function does only allow to set up <em>Img<T>-borders</em>
      with <em>T-values</em>. Hence it is realized as a template function.
      <pre>
      ..................        vvvvvvvvvvvvvvvvvv
      ..+-------------+.        vv...............v
      ..|.............|.        vv...............v
      ..|.............|.   -->  vv...............v
      ..|.............|.        vv...............v
      ..+-------------+.        vv...............v
      ..................        vvvvvvvvvvvvvvvvvv
      </pre>

      <h3>copy</h3>
      The copy stategy is an analogue to the IPPs "ippCopyReplicateBorder_.."
      function. It will extrude the images ROI edges towards the images borders.
      
      <pre>
             ROI                     Img                  Result
      ..................      .......++++++++##.     .....++++++++#####
      ..+-------------+.      .....++++++++####.     .....++++++++#####
      ..|.............|.      ....+++++++######.     ....+++++++#######
      ..|.............|. -->  ....-------######. --> ....-------#######
      ..|.............|.      ....-------######.     ....-------#######
      ..+-------------+.      ....-------####...     ....-------####...
      ..................      ....-------##.....     ....-------####...
      </pre>
      
      <h3>fromOther</h3>
      The last stategy is to copy the border pixels from another given
      Img. 
  
  */
  class ImgBorder{
    public:

    /// sets up an images border with a fixed value
    /** @param poImage destination image (with channel count C)
        @param ptVal destination value (for each of the C channels)
    */
    template<class T> 
    static void fixed(Img<T> *poImage, T* ptVal);
    
    /// sets up an images border by extruding the images ROI pixels towards the image border
    /** @param poImage destination image */
    static void copy(ImgBase *poImage);
    
    /// copies an images border from another image
    /** @param poImage destination image
        @param poOtherImage source image for the border data. This image must have
                            the same size and channel count as poImage. The function
                            will print a warning and return immediately without any 
                            changings otherwise.*/
    static void fromOther(ImgBase *poImage, ImgBase* poOther);

  };
}

#endif
