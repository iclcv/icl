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

\section secYUV YUV Color Conversion
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
<em>** this does not make sence!</em>

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
  */
  void cc(ImgBase *src, ImgBase *dst);
  
  
  /// Internal used type, that describes an implementation type of a specific color conversion function
  enum ccimpl{
    ccAvailable   = 0, /**< conversion is supported natively/directly */
    ccEmulated    = 1, /**< conversion is supported using the bridge format RGB */
    ccUnavailable = 2, /**< conversion is not implemented yet, but possible */
    ccImpossible  = 3  /**< conversion does not make sence (like formatMatrix to XXX) */
  };
  
  /// translates a ccimpl enum into a string representation
  /** The returned string for ccAvailable is "available" (...) */
  std::string translateCCImpl(ccimpl i);

  /// translates the string represenation of a
  ccimpl translateCCImlp(const std::string &s);

  /// returns the ccimpl state to a conversion from srcFmt to dstFmt
  ccimpl cc_available(format srcFmt, format dstFmt);
}
