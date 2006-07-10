#ifndef ICLCONVOLUTION_H
#define ICLCONVOLUTION_H

#include "ICL.h"

namespace icl{
  //@{ @name Image Convolution
  
  /// Convolution with source image, destination image and mask (IPP-OPTIMIZED)
  /** The iclConv function provides functionality for generic image filter procedures.
      To guarantee compability to the IPP optimized functions, also the C++ fallback
      implementations (which are not yet implemented!) should not break with the
      IPP-conventions:
      
      <h3>Conventions</h3>
      Let the source image image size be (W,H) and the mask size (M,N), then
      the convolution operation can only be performed on some inner rect of
      the source image. This inner rect is defined by the source images ROI-size.
      To ease using of iclConv, the ROI-size of the source image is made smaller
      before the operation and reset to its former size afterwards.
      The amount of pixels, that must be <i>eroded</i> before applying the convolution
      on the source image depends on the size of the mask, that is used.
      As the mask is anchored at the center-point, the ROI size is reduced by 
      (M/2,N/2) and enlaged by the same values.
      The destination image must be of the same size as this temporary ROI, and
      is resized if it's not. Furthermore the channel count and the depth of source and 
      destination image must be equal. In case of different channel count, 
      the channel count of the destination image ist adapted to the channel count of 
      the source image. If the depth of source and destination do not match, 
      an error message is shown, and the programm will be aborted with exit(-1).
      
      <h3>Efficiency (IPP-Optimized)</h3>
      The internally used IPP-functions do not provide iclbyte filter masks. So
      <b>note</b> that iclbyte convolution kernels must be converted internally
      into an Ipp32s of Ipp32f pointer, which involves some memory- 
      allocation/deallocation in O(M*N). This will have a small effect for 3x3 or
      5x5 filter kernels, but it may slow down the performace by more the 50%
      when dealing with huge masks.
      @param poSrc source image
      @param poDst destination image
      @param poMask convolution kernel
  */
  void iclConv(ICLBase *poSrc, ICLBase *poDst, ICLBase *poMask);
   
  /// Convolution with source image, destination image and iclfloat mask (IPP-OPTIMIZED)
  /** This function works essentially like the above function
      @param poSrc source image
      @param poDst destination image
      @param pfMask convolution kernel as iclfloat* (dimension iMaskW*iMaskH)
      @param iMaskW width of the convolution kernel
      @param iMaskH height of the convolution kernel
  */
  void iclConv(ICLBase *poSrc, ICLBase *poDst, iclfloat *pfMask, int iMaskW, int iMaskH);
  
  /// Convolution with source image, destination image and iclbyte mask (IPP-OPTIMIZED)
  /** This function works essentially like the above function
      @param poSrc source image
      @param poDst destination image
      @param pucMask convolution kernel as iclbyte* (dimension iMaskW*iMaskH)
      @param iMaskW width of the convolution kernel
      @param iMaskH height of the convolution kernel
  */
  void iclConv(ICLBase *poSrc, ICLBase *poDst, iclbyte *pucMask, int iMaskW, int iMaskH);
  
  //@}
}


#endif
