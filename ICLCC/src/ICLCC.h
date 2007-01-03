#include "ICLTypes.h"
#include <string>


/** \mainpage ICLCC Color Conversion Package of the ICL 
The ICL provides the following color formats / models
- RGB (sRGB)
- HLS
- YUV
- LAB
- r-g-Chromaticity
- Grayscale

\section secRanges Color Ranges
As default, the all ICL color spaces are represented in
the full range of [0,255] in all depths. By this, we get
the advantage of being able to treat all color images in
the same way.
The disadvantage is, that already existing color conversion
routines must be adapted to scale each color component to
that range.

\section secDefFormat sRGB, most common and default
Obviously, the RGB Color Model (or its absolut Version 
sRGB -> standard RGB) is one of the best known one by most
of the users. Although many computer vision approaches are using
other color spaces as HLS or LAB, the sRGB color space
complies a kind of angle point for color conversion functions.
So in most cases, the sRGB format forms a <em>bridge format</em>
for conversion from one to another format 
(xxx->yyy ==> xxx->rgb followed by rgb->yyy ; Another bridge 
color space (XYZ) is not considered here). In term of
efficiency of the color conversion code: Converting from
or to RGB is fast, other conversion may be much slower.

\section secYUV YUV Color Conversion
The literature yuv colormodel conversion is mostly a bit
confusing and far away from a kind of pseudocode, that can 
easyly be converted to (fast) c++ code.
The YUV Color model devides an incoming RGB signal into its
luninance component (Y) and two crominance components (U and V).
The common yuv-color holds Y in the range [0,1], u in range
[-0.436,0.436] and v in range [-0.615,0,615]. These different
ranges compound implementing algorithms. Hence the
color conversion functions of the ICL are adapted to scale the
resulting values to the range [0,255] in all channels.
Outgoing from the basic equation for converting rgb to yuv and
back
<pre>
Y = 0.299*R + 0.587*G + 0.114*B
U = -0.147*R - 0.289*G + 0.436*B = 0.492*(B- Y)
V = 0.615*R - 0.515*G - 0.100*B = 0.877*(R- Y)

R = Y + 1.140*V
G = Y - 0.394*U - 0.581*V
B = Y + 2.032*U
</pre>
The fromulars are adapted for using ranges [0,255]:

<pre>
Y = (0.299*R + 0.587*G + 0.114*B);  
U = 0.56433408*(B-Y) + 127.5;
V = 0.71326676*(R-Y) + 127.5;

R = Y +               290.7   * v2;
G = Y - 100.47 * u2 - 148.155 * v2;   with: u2 = 0.0034196078*U - 0.436;
B = Y + 518.16 * u2;                   and  v2 = 0.0048235294*V - 0.615;

</pre>
To avoid <em>expensive</em> floating point operations, the 
conversions can be optimized by creating a so called <b>fixed 
point approximation</b> of the above code:
<pre>
Y = ( 1254097*R + 2462056*G + 478151*B ) >> 22;  
U = ( 2366989*(B-Y) + 534773760        ) >> 22;
V = ( 2991658*(R-Y) + 534773760        ) >> 22;

R = Y +  ( ( 290 * V2 ) >> 22 );
G = Y -  ( ( 100  * U2 + 148 * V2) >> 22 ); with: U2 = 14343*U - 1828717; 
B = Y +  ( ( 518 * U2 ) >> 22 );             and  V2 = 20231*v - 2579497; 
</pre>
This approximization produces errors up to 2, and runs up to 20 times 
faster.


*/
namespace icl{
  
  void cc(ImgBase *src, ImgBase *dst);
  
  
  enum ccimpl{
    ccAvailable   = 0,
    ccEmulated    = 1,
    ccUnavailable = 2, 
    ccImpossible  = 3
  };
  
  std::string translateCCImpl(ccimpl i);
  ccimpl translateCCImlp(const std::string &s);

  ccimpl cc_available(format srcFmt, format dstFmt);
}
