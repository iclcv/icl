/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/CCFunctions.h                      **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter, Andre Justus                      **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Types.h>
#include <ICLUtils/Size.h>
#include <string>
#include <ICLUtils/Macros.h>

namespace icl{
  namespace core{

    /// Color conversion from source to destination image
    /** All color conversions of the ICL are tackled by this function.
        The destination images size is adapted to the source images size,
        as cc does not provide implicit scaling; Use the Converter class,
        also located in this package, for color conversion with implicit
        scaling instead.

        Currently ICL supports the following color formats:
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

        The SSE-implementation uses the original floating point calculation,
        that is extended to match the value range [0,255]:
        <pre>
        Y = 0.299f*R + 0.587f*G + 0.114*B
        U = 0.492f*(B-Y) + 128.0f
        V = 0.877f*(R-Y) + 128.0f
        if(V < 0.0f) V = 0.0f;
        else if(V > 255.0f) V = 255.0f;

        R = Y + 1.140*(V - 128)
        G = Y - 0.394*(U - 128) - 0.581*(V - 128)
        B = Y + 2.032*(U - 128)
        R = clip(R, 0, 255);
        G = clip(G, 0, 255);
        B = clip(B, 0, 255);
        </pre>



        \subsection IPPCOMPA_XYZ IPP Compatibility

        In order to achieve compatibility with the yuv color conversion provided by
        intel IPP (which is used if IPP is available), also ICL's color conversion
        methods were slightly adapted. We again used fixed point approximations for
        the algorithms described in the IPP manual:

        <pre>
        rgb-to-yuv:

        y = ( 1254097*r + 2462056*g + 478151*b ) >> 22;
        u = (2063598*(b-y) >> 22) + 128;
        v = (3678405*(r-y) >> 22) + 128;

        if(v<0) v=0;
        else if(v > 255) v = 255;
        </pre>

        <pre>
        yuv-to-rgb:

        icl32s u2 = u-128;
        icl32s v2 = v-128;
        icl32s y2 = y<<22;

        r = (y2 + 4781506 * v2 ) >> 22;
        g = (y2 - 1652556 * u2 - 2436891 *v2 ) >> 22;
        b = (y2 + 8522826 * u2 ) >> 22;
        </pre>
        <b>please note</b>
        Due to the clipping process of 'v' in rgb_to_yuv,
        this method cannot restore an original rgb value completetly. Since we lost some information
        in v, the resulting r and g values are differ as follows: r-r' in [-32,35], and g-g' in [-17,18]


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
        wX = 0.950455;
        wY = 1.0;
        wZ = 1.088753;
        _13 = 1.0/3.0;

        XXn = X / wX;
        YYn = Y / wY;
        ZZn = Z / wZ;

        L = (YYn > 0.008856) ? ((116 * pow (YYn, _13))-16) : (903.3 * YYn);

        fX = (XXn > 0.008856) ? pow (XXn, _13) : 7.787 * XXn + (16 / 116);
        fY = (YYn > 0.008856) ? pow (YYn, _13) : 7.787 * YYn + (16 / 116);
        fZ = (ZZn > 0.008856) ? pow (ZZn, _13) : 7.787 * ZZn + (16 / 116);

        L = 116 * fY - 16;
        a = 500.0 * (fX - fY);
        b = 200.0 * (fY - fZ);

        LABToXYZ
        n = 16.0/116.0;

        wX = 0.950455;
        wY = 1.0;
        wZ = 1.088754f;

        fy = (l+16)/116;
        fx = fy+a/500;
        fz = fy-b/200;

        X = (fx>0.206893f) ?  wX*pow(fx,3) : wX*(fx-n)/7.787f;
        Y = (fy>0.206893f) ?  wY*pow(fy,3) : wY*(fy-n)/7.787f;
        Z = (fz>0.206893f) ?  wZ*pow(fz,3) : wZ*(fz-n)/7.787f;
        </pre>



        \section secGray Gray Scale Conversion

        The gray scale conversion is optimized for speed performance. Although,
        L of Lab is not equal to the Y of YUV, color formats, that have an
        brightness-like component are converted to gray scale by picking this
        channel. RGB is converted to gray by the simple channel mean (r+g+b)/3.



        \section matrix formatMatrix Conversion

        As the matrix image format offers no color information, matrix image data is just copied
        from the source image channels to the destination image channels. If the source image has
        more channels, the remaining channels are left unregarded. If otherwise the destination
        image has more channels, this channels are left  unchanged



        \section secChroma r-g-Chromaticity Color Space Conversion

        The chromaticity space r,g,b divides the R,G,B components by the city
        block norm of the according color r=R/(R+G+B), g=G/(R+G+B), b= B/(R+G+B).
        By this means, the b-component becomes redundant, and can be left out,
        which leads to the r-g-chromaticity space.
        Conversions form the r-g-Chromaticity space to other formats are not
        possible, as to much information is lost.



        \section secBench Benchmarks

        The following tables, show the cross format conversion times:
        <b>TODO: re-run this test --> which machine / which image size ?</b>

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


        \section ROI ROI-Support

        A new feature is the ROI-Support of the "cc" function. If the "roiOnly"-flag given to cc function is
        set to true, the source images ROI is converted into the destination images ROI. In this case, the
        destination image is not adapted to the source image. Instead, a single test is performed to ensure,
        that the source images ROI has the same size as the destination images ROI. If the test fails, an
        error occurs and the function returns immediately.\n
        <b>Note:</b> internally all functions are optimized for running without ROI support (in this case, the
        images data arrays are 1D). Thus, the ROI-Support mode (roiOnly = true) runs approx. 20% (2%-50%) slower
        depended on the specific source and destination format.


        \section IPP IPP Acceleration

        Most functions are not IPP accelerated even if IPP support is available because IPP supports most conversions
        for interleaved formats only. However a few very common conversions make use of available IPP acceleration:
        - RGB -> YUV
        - YUV -> RGB
        - RGB -> HLS
        - HLS -> RGB

        The IPP accelerated times are not part of the benchmark above. Usually, as a rule of thumb, IPP is supposed to
        be at least twice as fast as the C++-fallback implementation.
        \section IPPComp Important note regarding IPP compatibility</b>
        Currently, IPP and C++ fallback version are not 100% compatible as IPP uses
        additional optimizations that causes, that the U- and V- channel range are not full 8bit range [0-255].
        We aim to fix that in future
    */
    ICLCore_API void cc(const ImgBase *src, ImgBase *dst, bool roiOnly=false);

    /// returns whether a lookup table was already created for src and dst format
    /** @param srcFmt source format
        @param dstFmt destination format
    **/
    ICLCore_API bool lut_available(format srcFmt, format dstFmt);

    /// Internally creates a lookup table to accelerate conversion between given formats
    /** Take care: <b>Each LUT uses up to 48MB of system memory</b>
        @param srcFmt source format
        @param dstFmt destination format
    **/
    ICLCore_API void createLUT(format srcFmt, format dstFmt);

    /// releases the internal lookup table created with createLUT
    /**  @param srcFmt source format
         @param dstFmt destination format
    **/
    ICLCore_API void releaseLUT(format srcFmt, format dstFmt);

    /// releases all lookup tables that were created with createLUT
    ICLCore_API void releaseAllLUTs();

    /// Internal used type, that describes an implementation type of a specific color conversion function
    enum ccimpl{
      ccAvailable   = 0, /**< conversion is supported natively/directly */
      ccEmulated    = 1, /**< conversion is supported using the bridge format RGB */
      ccAdapted     = 2, /**< conversion is actually not possible but although performed ( like XXX to matrix )*/
      ccUnavailable = 3, /**< conversion is not implemented yet, but possible */
      ccImpossible  = 4  /**< conversion does not make sense (like croma to RGB )*/
    };

    /// translates a ccimpl enum into a string representation
    /** The returned string for ccAvailable is "available" (...) */
    ICLCore_API std::string translateCCImpl(ccimpl i);

    /// translates the string represenation of a
    ICLCore_API ccimpl translateCCImlp(const std::string &s);

    /// returns the ccimpl state to a conversion from srcFmt to dstFmt
    ICLCore_API ccimpl cc_available(format srcFmt, format dstFmt);


    /// Convert an image in YUV420-format to RGB8 format (ippi accelerated)
    /**
    @param poDst destination image
    @param pucSrc pointer to source data (data is in YUV420 format, which is planar and which's U- and V-channel
    has half X- and half Y-resolution. The data pointer has iW*iH*1.5 elements)
    @param s image size
    */
    ICLCore_API void convertYUV420ToRGB8(const unsigned char *pucSrc, const utils::Size &s, Img8u* poDst);

    /// Convert an 4 channel Img8u into Qts ARGB32 interleaved format
    /** \@param pucDst destination data pointer of size
    poSrc->getDim()*4
    \@param poSrc source image with 4 channels
    */
    //void convertToARGB32Interleaved(const Img8u *poSrc, unsigned char *pucDst);

    /// Convert an 4 channel Img32f into Qts ARGB32 interleaved format
    /** This function will first convert the given Img32f poSrc into the
    buffer image poBuffer. Then it will call the above method, to convert the
    buffer data into pucDst. If the buffer is not valid, the method will
    return immediately.
    \@param pucDst destination data pointer of size
    poSrc->getDim()*4
    \@param poSrc source image with 4 channels
    \@param poBuffer buffer to use for internal depth conversion.
    */
    //void convertToARGB32Interleaved(const Img32f *poSrc, Img8u *poBuffer, unsigned char *pucDst);


    /// Converts a planar Img<S> images ROI  into its interleaved representations by mixing the channels
    /** This function is highly optimized, because it is needed whenever we need interleaved images
        @param src source image image
        @param dst destination data pointer
        @param dstLineStep optinal linestep of the destination image. This must be given, if it differs from
                           the source images lineStep multiplied by the source images channel count
    */
    template<class S, class D> ICLCore_API
    void planarToInterleaved(const Img<S> *src, D* dst, int dstLineStep = -1);

    /// Converts interleaved image data into planar representation
    /** The source data is transformed into the destination images ROI
        @param src data pointer
        @param dst image pointer
        @param srcLineStep optionally given src linestep size
    */
    template<class S, class D> ICLCore_API
    void interleavedToPlanar(const S *src, Img<D> *dst, int srcLineStep = -1);


    /// converts given (r,g,b) pixel into the yuv format
    ICLCore_API void cc_util_rgb_to_yuv(const icl32s r, const icl32s g, const icl32s b, icl32s &y, icl32s &u, icl32s &v);

    /// converts given (y,u,v) pixel into the rgb format
    ICLCore_API void cc_util_yuv_to_rgb(const icl32s y, const icl32s u, const icl32s v, icl32s &r, icl32s &g, icl32s &b);

    /// converts given (r,g,b) pixel into the hls format
    ICLCore_API void cc_util_rgb_to_hls(const icl32f r255, const icl32f g255, const icl32f b255, icl32f &h, icl32f &l, icl32f &s);

    /// converts given (h,l,s) pixel into the rgb format
    ICLCore_API void cc_util_hls_to_rgb(const icl32f h255, const icl32f l255, const icl32f sl255, icl32f &r, icl32f &g, icl32f &b);

    /// converts given (r,g,b) pixel into the RG-chroma format
    ICLCore_API void cc_util_rgb_to_chroma(const icl32f r, const icl32f g, const icl32f b, icl32f &chromaR, icl32f &chromaG);

    /// converts given (r,g,b) pixel into the Lab format
    void cc_util_rgb_to_lab(const icl32f &r, const icl32f &g, const icl32f &b, icl32f &L, icl32f &A, icl32f &B);
  } // namespace core
}
