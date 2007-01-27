#include "ICLTypes.h"
#include <Size.h>
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
Obviously, the RGB Color Model (or its absolute Version 
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

\section secYUV YUV Color Space Conversion
The literature yuv color model conversion is mostly a bit
confusing and far away from a kind of pseudocode, that can 
easily be converted to (fast) c++ code.
The YUV Color model divides an incoming RGB signal into its
luminance component (Y) and two chrominance components (U and V).
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
The formulas are adapted for using ranges [0,255]:

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
This approximation produces errors less 3/255, and runs up to 20% faster.
A further optimization can be implemented using lookup tables.


\section secHLS HLS Color Space Conversion
The HLS color space, also known as the HSI color space with another channel
order describes colors in more independent components. The <b>H</b>ue
component complies an angle and is as well as all other color information
scaled to the range [0,254]. The value H=255 is identical to H=0 (red).
Independent from the colors hue, its <b>L</b>ightness is estimated by the
second component. Basic colors as red (r=255,g=0,b=0) have a lightness of
127; lighter colors have a hight L value; darker colors have a lower 
one. The last component is the <b>S</b>aturation of the color. 
The color model can be drawn as a double (hex)cone . Note that the HLS
color spaces resolution is higher in its center (L near 127).
The following formulas describe the conversion from and to the RGB format:
<pre>
(H,L,S) RGBToHLS(R,G,B)
(r,g,b) = (R,G,B)/255;
    
m = min(r,g,b)
v = max(r,g,b)
    
l = (m+v)/2
if(l <= 0){
  (H,L,S) = (0,0,0)
  return
}

vm = v-m
if ( vm > 0 ) {
  if(l<=0.5){
     s=vm/(v+m)
  }else{
     s=vm/(2.0-v-m)
  }
}else{
  (H,L,S)=(0,l*255,0)
  return
}

r2 = (v - r) / vm
g2 = (v - g) / vm
b2 = (v - b) / vm
    
if (r == v)
  h = (g == m ? 5.0 + b2 : 1.0 - g2)
else if (g == v)
  h = (b == m ? 1.0 + r2 : 3.0 - b2)
else
  h = (r == m ? 3.0 + g2 : 5.0 - r2)
if(h == 255) h = 0
(H,L,S) = (h*255/6,l*255,s*255)
</pre> 
An optimization, that allows conversion directly with (r,g,b) values in
range [0,255] is not yet implemented.

<pre>
(R,G,B) HLSToRGB(H,L,S)

(h,l,s) = (H,L,S)/255;
v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl)
if (v <= 0 ) {
  R = G = B = 0;
  return;
} 
    
m = l + l - v;
sv = (v - m ) / v;
h *= 6.0;
 
int sextant = (int)h;	
 
fract = h - sextant;
vsf = v * sv * fract;
mid1 = m + vsf;
mid2 = v - vsf;

switch (sextant) {
  case 0: r = v;    g = mid1; b = m;    break;
  case 1: r = mid2; g = v;    b = m;    break;
  case 2: r = m;    g = v;    b = mid1; break;
  case 3: r = m;    g = mid2; b = v;    break;
  case 4: r = mid1; g = m;    b = v;    break;
  case 5: r = v;    g = m;    b = mid2; break;
}
    
(R,G,B) = (r,g,b)*255;
</pre>
An additional optimization can be implemented using lookup tables.

\section secLAB Lab Color Space Conversion
The LAB color space (strictly CIE L*a*b*), was designed to describe the complete
range of colors, that can be seen by the human eye. It must not be mixed up with
the "Hunter Lab" color space, that is sure related to the CIE L*a*b*, but in detail
much different.
"The three parameters in the model represent the lightness of the color 
(L*, L*=0 yields black and L*=100 indicates white), its position between magenta 
and green (a*, negative values indicate green while positive values indicate magenta) 
and its position between yellow and blue (b*, negative values indicate blue and positive 
values indicate yellow)"(wikipedia).
"CIE 1976 L*a*b* is based directly on the CIE 1931 XYZ color space as an attempt 
to linearize the perceptibility of color differences, using the color difference 
metric described by the MacAdam ellipse"(wikipedia). So an euclidian (linear) 
color difference metric can be used here. 
The following code show the formulas LabToXYZ, XYZToLab, RGBToXYZ and XYZToRGB.
<pre>
RGBToXYZ
static icl32f m[3][3] = {{ 0.412453, 0.35758 , 0.180423},
                         { 0.212671, 0.71516 , 0.072169},
                         { 0.019334, 0.119193, 0.950227}};

X = m[0][0] * R + m[0][1] * G + m[0][2] * B;
Y = m[1][0] * R + m[1][1] * G + m[1][2] * B;
Z = m[2][0] * R + m[2][1] * G + m[2][2] * B;


XYZToRGB
static icl32f m[3][3] = {{ 3.2405, -1.5372,-0.4985},
                         {-0.9693,  1.8760, 0.0416},
                         { 0.0556, -0.2040, 1.0573}};
R = m[0][0] * x + m[0][1] * y + m[0][2] * z;
G = m[1][0] * x + m[1][1] * y + m[1][2] * z;
B = m[2][0] * x + m[2][1] * y + m[2][2] * z;

XYZToLAB
wX = 95.0456;
wY = 100.0;
wZ = 108.8754;
_13 = 1.0/3.0;
    
XXn = X / wX;
YYn = Y / wY;
ZZn = Z / wZ;

L = (YYn > 0.008856) ? ((116 * pow (YYn, _13))-16) : (903.3 * YYn);
    
fX = (XXn > 0.008856) ? pow (XXn, _13) : 7.787 * XXn + (16 / 116);
fY = (YYn > 0.008856) ? pow (YYn, _13) : 7.787 * YYn + (16 / 116); 
fZ = (ZZn > 0.008856) ? pow (ZZn, _13) : 7.787 * ZZn + (16 / 116);
    
a = 500.0 * (fX - fY);
b = 200.0 * (fY - fZ);

LABToXYZ
d = 6.0/29.0;
n = 16.0/116.0;
f = 3*d*d;

wX = 95.0456;
wY = 100.0;
wZ = 108.8754;

fy = (l+16)/116;
fx = fy+a/500;
fz = fy-b/200;

X = (fx>d) ?  wX*pow(fx,3) : (fx-n)*f*wX;
Y = (fy>d) ?  wY*pow(fy,3) : (fy-n)*f*wY;
Z = (fz>d) ?  wZ*pow(fz,3) : (fz-n)*f*wZ;
</pre>
 
\section secGray Gray Scale Conversion
The gray scale conversion is optimized for speed performance. Although,
L of Lab is not equal to the Y of YUV, color formats, that have an
brightness-like component are converted to gray scale by picking this
channel. RGB is converted to gray by the simple channel mean (r+g+b)/3.

\section secChroma r-g-Chromaticity Color Space Conversion
The chromaticity space r,g,b divides the R,G,B components by the city
block norm of the according color r=R/(R+G+B), g=G/(R+G+B), b= B/(R+G+B).
By this means, the b-component becomes redundant, and can be left out,
which leads to the r-g-chromaticity space. 
Conversions form the r-g-Chromaticity space to other formats are not 
possible, as to much information is lost.
 
\section secBench Benchmarks
The following tables, show the cross format conversion times:

<b><em>depth32f</em></b>
<table>
<tr>  <td>src\\dst</td><td>rgb</td> <td>yuv</td> <td>lab</td> <td>hls</td> <td>gray</td> <td>chroma</td>  </tr>
<tr>  <td>rgb</td>     <td>10*</td> <td>26</td>  <td>47</td>  <td>30</td>  <td>7</td>    <td>21</td>      </tr>
<tr>  <td>yuv</td>     <td>23</td>  <td>8</td>   <td>387?</td><td>64</td>  <td>2</td>    <td>45</td>      </tr>
<tr>  <td>lab</td>     <td>45</td>  <td>63</td>  <td>8</td>   <td>75</td>  <td>2</td>    <td>63</td>      </tr>
<tr>  <td>hls</td>     <td>23</td>  <td>43</td>  <td>67</td>  <td>8</td>   <td>2</td>    <td>44</td>      </tr>
<tr>  <td>gray</td>    <td>11</td>  <td>12</td>  <td>11</td>  <td>13</td>  <td>3</td>    <td>**</td>       </tr>
<tr>  <td>chroma</td>  <td>**</td>  <td>**</td>  <td>**</td>  <td>**</td>  <td>**</td>   <td>6</td>       </tr>
</table>
<em>*  times in ms</em>
<em>** this does not make sense!</em>

<b><em>depth8u</em></b>
<table>
<tr>  <td>src\\dst</td><td>rgb</td> <td>yuv</td> <td>lab</td> <td>hls</td> <td>gray</td> <td>chroma</td>  </tr>
<tr>  <td>rgb</td>     <td>3</td>   <td>9</td>   <td>31</td>  <td>22</td>  <td>3</td>    <td>22</td>      </tr>
<tr>  <td>yuv</td>     <td>13</td>  <td>2</td>   <td>487?</td><td>56</td>  <td>0.2</td>  <td>33</td>      </tr>
<tr>  <td>lab</td>     <td>31</td>  <td>40</td>  <td>2</td>   <td>54</td>  <td>0.2</td>  <td>52</td>      </tr>
<tr>  <td>hls</td>     <td>12</td>  <td>21</td>  <td>43</td>  <td>2</td>   <td>0.2</td>  <td>33</td>      </tr>
<tr>  <td>gray</td>    <td>3</td>   <td>4</td>   <td>4</td>   <td>4</td>   <td>0.2</td>  <td>**</td>      </tr>
<tr>  <td>chroma</td>  <td>**</td>  <td>**</td>  <td>**</td>  <td>**</td>  <td>**</td>   <td>6</td>       </tr>
</table>

<b><em>cross depth conversion depth8u --> depth32f</em></b>
<table>
<tr>  <td>src\\dst</td><td>rgb</td> <td>yuv</td> <td>lab</td> <td>hls</td> <td>gray</td> <td>chroma</td>  </tr>
<tr>  <td>rgb</td>     <td>9</td>   <td>10</td>  <td>32</td>  <td>22</td>  <td>4</td>    <td>23</td>      </tr>
<tr>  <td>yuv</td>     <td>11</td>  <td>8</td>   <td>474?</td><td>54</td>  <td>3</td>    <td>34</td>      </tr>
<tr>  <td>lab</td>     <td>30</td>  <td>41</td>  <td>8</td>   <td>52</td>  <td>3</td>    <td>53</td>      </tr>
<tr>  <td>hls</td>     <td>14</td>  <td>22</td>  <td>43</td>  <td>8</td>   <td>3</td>    <td>34</td>      </tr>
<tr>  <td>gray</td>    <td>8</td>   <td>8</td>   <td>8</td>   <td>8</td>   <td>3</td>    <td>**</td>      </tr>
<tr>  <td>chroma</td>  <td>7</td>   <td>**</td>  <td>**</td>  <td>**</td>  <td>**</td>   <td>5</td>       </tr>
</table>
*/

namespace icl{
  
  /// Color conversion from src to the dst image
  /** All color conversions of the ICL are tackled by this function.
      see the ICLCC mainpage for its behavior, features and performance.
      The destination images size is adapted to the source images size,
      as cc does not provide implicit scaling; Use the Converter class,
      also located in the ICLCC package, for color conversion with implicit
      scaling instead.
  */
  void cc(const ImgBase *src, ImgBase *dst);
  
  
  /// Internal used type, that describes an implementation type of a specific color conversion function
  enum ccimpl{
    ccAvailable   = 0, /**< conversion is supported natively/directly */
    ccEmulated    = 1, /**< conversion is supported using the bridge format RGB */
    ccUnavailable = 2, /**< conversion is not implemented yet, but possible */
    ccImpossible  = 3  /**< conversion does not make sense (like formatMatrix to XXX) */
  };
  
  /// translates a ccimpl enum into a string representation
  /** The returned string for ccAvailable is "available" (...) */
  std::string translateCCImpl(ccimpl i);

  /// translates the string represenation of a
  ccimpl translateCCImlp(const std::string &s);

  /// returns the ccimpl state to a conversion from srcFmt to dstFmt
  ccimpl cc_available(format srcFmt, format dstFmt);


  /// Convert an image in YUV420-format to RGB8 format (ippi accelerated)
  /**
  @param poDst destination image
  @param pucSrc pointer to source data (data is in YUV420 format, which is planar and which's U- and V-channel
  has half X- and half Y-resolution. The data pointer has iW*iH*1.5 elements)
  @param s image size 
  */
  void convertYUV420ToRGB8(const unsigned char *pucSrc, const Size &s, Img8u* poDst);
  
  /// Convert an 4 channel Img8u into Qts ARGB32 interleaved format 
  /** @param pucDst destination data pointer of size
  poSrc->getDim()*4
  @param poSrc source image with 4 channels
  */ 
  //void convertToARGB32Interleaved(const Img8u *poSrc, unsigned char *pucDst);
  
  /// Convert an 4 channel Img32f into Qts ARGB32 interleaved format 
  /** This function will first convert the given Img32f poSrc into the 
  buffer image poBuffer. Then it will call the above method, to convert the
  buffer data into pucDst. If the buffer is not valid, the method will
  return immediately.
  @param pucDst destination data pointer of size
  poSrc->getDim()*4
  @param poSrc source image with 4 channels
  @param poBuffer buffer to use for internal depth conversion.
  */ 
  //void convertToARGB32Interleaved(const Img32f *poSrc, Img8u *poBuffer, unsigned char *pucDst);
  
  
  /// Converts a planar Img<S> image into its interleaved representations by mixing the channels
  /** This function is highly optimized, because it is needed whenever we need interleaved images
  */
  template<class S, class D>
  void planarToInterleaved(const Img<S> *src, D* dst);
  
  /// Converts interleaved image data into planar representation
  template<class S, class D>
  void interleavedToPlanar(const S *src,const Size &srcSize, int srcChannels,  Img<D> *dst);

}
