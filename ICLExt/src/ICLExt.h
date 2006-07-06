#ifndef ICL_EXT_H
#define ICL_EXT_H

#include "ICLcc.h"
#include "ICLConverter.h"
#include "ICLConvolution.h"

/**
\mainpage Image Component Library Extensions (ICLExt)

\section _INTO_ Introduction
The <b>ICLExt</b> package provides some common extensions to the ICLCore
library:
  - <b>Color Conversion:</b> see iclcc and ICLConverter
  - <b>Arbitrary Image Conversion:</b> see ICLConverter
  - <b>Image Convolution</b> see ICLConvolution
  - <b>Philips Webcam Grabber (PWC)</b>

\section _ICLCC_ Color Conversion (iclcc)
The color conversion routines of the ICL are packaged in the iclcc function,
which is directly accessible in the ICL namespace when the ICLcc.h header is 
included. Most of the color conversion routines are not supported yet.
<b>TODO: put a up-to-date list of supported conversions here</b>

\section _CONVERSION Arbitrary Image Conversion (ICLConverter)
In addition to the color conversions provided by iclcc, the ICLConverter goes 
even further, and offers a so called "arbitrary conversion mechanism".
To conceive the operation of the ICLConverter, it can be seen as a
<b>black box</b>, with arbitrary image in- and output. If source and 
destination images are plugged in, this box can be triggered, which will
involve a conversion of the image data from the input- to the output image
-- in some <b><i>reasonable</i></b> modality:

<h3>Order of Operations</h3>
During Conversion process, <b>4</b> different conversion can
be necessary:
- <b>(1st) adapt channel count:</b> the destination images
  channel count will be adapted.
- <b>(2nd) convert depth:</b> convert the depth if it differs 
  between source and destination. (Use an internal buffer, if other
  conversions (step 3 or 4) will follow.
- <b>(3rd) scale the image:</b> scale the image data from source to
  the destination image. (Use an extra internal buffer, if a 
  color conversion (step 4) will follow,
- <b>(4th) color conversion:</b> Convert the color format of the
  source into the destination image, depending on the source
  and destination images format.
  
<b>Note:</b> if no operation must be performed, then the data of
the source image will be copied <i>deeply</i> to the destination 
image.

\section _CONV_ Image Convolution (ICLConvolution)
The image convolution package (accessible if ICLConvolution.h
is included), provides functions for general image convolution,
by wrapping corresponding IPP-functions. Fallback functions
in <i>pure</i> C++ have to be integrated later.
<b>TODO:</b> a following step will be to integrate some
default filters like sobel-X/Y,Laplace,Previt,Gauss,...

\section _PWC_ PWCGrabber (ICLPWCGrabber)
The Phillips Webcam grabber, provided by the header file
ICLPWCGrabber.h offers a compact Grabber class, to
receive image data from Webcams, that are using the
PWC-Kernel module (<b>P</b>hillps-<b>W</b>eb<b>C</b>am).
For more information about this module, visit the following 
internet site: 
<A href="http://saillard.org/linux/pwc/">www.saillard.org/linux/pwc</A>


<b>TODO: Spell checking!</b>
**/

#endif
