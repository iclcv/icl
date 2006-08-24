#ifndef ICLCORE_H
#define ICLCORE_H

/** 
\mainpage ICL (Image-Component-Library) : ICLCore 
\section Overview

The ICL is a C++ Image-Library, designed for Computer-Vision tasks. It
supports multi-channel images (class ImgI/Img) with a depth of 8bit integer or 32bit floats.
All channels within the image share a common size and region of 
interest (ROI). This allows to handle color images as 3-channel images for example.

Despite of the different image depth, most methods of an image class have
common code. Hence, the two different pixel formats are implemented by a
template class <b>Img<imagedepth></b>. Methods which are independent on the
image depth are provided by a common interface class, named <b>ImgI</b>. This
allows easy and type-clean wrapping of both template classes within frameworks
such as Neo/NST or TDI.

Hence, the Library provides two basic image classes: 
- <b>ImgI</b>: The <b>abstract interface</b> class providing common, 
  but depth-independent information about the image structure:
  - size (in pixels)
  - channel count  (see <b>channel concept</b>)
  - type of pixels (see <b>data types</b>)
  - color format (see <b>color formats</b>)
  - raw image data access
  - Region of Interest (see <b>Region of Interests</b> (ROI))

  It has no public constructors so it has to be used as interface
  class for the derived template classes Img<Type>.
  Most of the functions in the ImgI class are purely virtual which
  implies, that they are implemented in the derived classes Img<T>.

- <b>Img</b>: The <i>proper</i> image class is implemented as a template,
  where the datatype of each pixel value is the template parameter.
  Internally each Img<T> object holds a std::vector of so called smart 
  pointers (class SmartPtr in the ICLUtils package) to the channel
  data. The Img class provides some additional image information and access
  functions:
  - type-save image data access ( using the functions 
    getData(int channel) and getROIData(int channel)
  - access to single pixel values using the ()-operator
  - access to all Image/all ROI pixels using the ImgIterator 
  (see <b>ImgIterator</b> class reference)
    
In addition to the classes Img and ImgI, the ICLCore package
provides some utility functions, that facilitates working with 
these classes (see icl namespace for more details).
  

@see ImgI, Img

\section SEC_DATA_ORIGN Data Origin
Common image formats like png or bmp are following the convention,
that the image origin is the <b>upper left</b> corner of the image.
As many other frameworks like <b>IPP</b> (see section IPP-Optimization)
or the Qt-framework (www.trolltech.com) are following this convention too,
the data origin of ICL images should always be associated with the
upper left corner of the image.
Although most image operations like scaling, filters or thresholding do
not care about the associations the user has with the images content,
this standard will be very useful for I/O-routines or image visualization
and - not least - whenever discussing about ICL images.

\section Channel-Concept
The Img treats images as a stack of image slices -- <b>channels</b>.  Channels
can be shared by multiple Img images, which is especially important for fast
shallow images copies. Actually, it is possible to freely compose existing
channels (within several "parent images") to another new image.  

Attention: The newly <i>composed</i> image shares its channel data with
the original images, such that modifications will effect all images equally.
In order to get an independent image a deepCopy as well as a detach method
are provided. The latter replaces the "shared" image channel(s) with new 
independent ones. Shared channel data are stored using the boost-like 
shared pointer class SmartPtr, which realizes a garbage collector
automatically releasing <i>unused</i> image channels.

@see Img, ImgChannel

\section Data-Types
Currently the Img provides two different data types:
- <b>icl8u</b> 8bit unsigned char
- <b>icl32f</b> 32bit float

Img-classes are predefined for these two types:
- Img<icl32f> : public ImgI <b>typedef'd to Img32f</b>
- Img<icl8u> : public ImgI <b>typedef'd to Img8u</b>

Each of these data types has several advantages/disadvantages. The greatest
disadvantage of the icl8u, is its bounded range (0,1,...,255),
which has the effect, that all information has to be scaled to this
range, and all image processing functions must take care that
no range-overflow occurs during calculation. Furthermore
the limited range may cause loss of information - particular in 
complex systems.
The advantage of integer values is, that computation is faster
than using float values, not at least because of the 4-Times 
larger memory usage.
 
@see Depth, icl8u, icl32f

\section Color Formats
An ImgI image provides some information about the (color) format, that
is associated with the image data represented by the images channels. Color
is written in brackets, as not all available formats imply color-information.
The most known color space is probably the RGB color space. 
If an ImgI image has the format <i>formatRGB</i>, than this implies the 
following:
- the image has exactly 3 channels
- the first channel contains RED-Data in range [0,255]
- the second channel contains GREEN-Data in range [0,255]
- the third channel contains BLUE-Data in range [0,255]

All additional implemented Img-Packages may/should use this information. 
The currently available Img formats are member of the enum Format.
A special format: formatMatrix may be used for arbitrary purpose.

@see Format

\section IPP-Optimization
The IPP Intel Performance Primitives is a c-library that contains highly 
optimized and hardware accelerated functions for image processing, and 
other numerical problems for all processors providing the SSE command set, 
i.e. most new Intel and AMD processors.
To provide access to IPP/IPPI functionality, the ImgCore library can be 
compiled with <b>WITH_IPP_OPTIMIZATIONS</b> defined. In this case, the 
following adaptions are performed:
- the icl data types icl32f and icl8u are defined as the ipp compatible
  type Ipp32f and Ipp8u.
- the classes Size, Point and Rect are derived from the corresponding ipp-
  structs IppiSize, IppiPoint and IppiRect. Hence the the programme will
  be able to uses the ICL-stucts Size, Point and Rect everywhere whare 
  the ipp-structs are needed
- some of the builtin Img functions, like scaling or converting to another type
  are accelerated using equivalent ipp-function calls.

@see Img, ImgChannel

\section _DEBUG_MACROS_ How to use LOG-Macros in the Img
The ICLUtils package contains the Macros.h header file,
which provides the common debug macros for ICL classes. The debug system
knows 6 different debug levels (0-5). Level depended debug messages
can be written to std::out using the <b>DEBUG_LOG\<LEVEL\></b>-macro.
The set debug level (0 by default) regulates the verboseness of the 
ICL library. The debug levels (0-5) are characterized as follows:

<h3>Img debug levels</h3>
      <table>
         <tr>  
            <td><b>level</b></td>
            <td><b>macro</b></td>
            <td><b>description</b></td>
         </tr><tr>  
            <td><b>level 0</b></td>    
            <td>ERROR_LOG(x)</td>
            <td> <b>(default)</b> Only critical error messages 
                 will be written to std::err. If an exception 
                 that occurred may cause a program crash, then 
                 an error message should be written. 
            </td>
         </tr><tr> 
            <td><b>level 1</b></td>    
            <td>WARNING_LOG(x)</td>
            <td> Also non critical waring will be written (now to 
                 std::out. Exceptions, that are notified with the
                 WARNING_LOG template may not cause a program crash.
            </td>
         </tr><tr>  
            <td><b>level 2</b></td>    
            <td>FUNCTION_LOG(x)</td>
            <td> The next debug level will notify each function call
                 by printing an according message to std::out. Take
                 care to use the FUNCTION_LOG template in each function
                 you implement for the Img
            </td>
         </tr><tr>  
            <td><b>level 3</b></td>    
            <td>SECTION_LOG(x)</td>
            <td> Sections of functions (like initialization-section,
                 working-section or end-section) should be commented 
                 using the SECTION_LOG macro
            </td>
         </tr><tr>  
            <td><b>level 4</b></td>    
            <td>SUBSECTION_LOG(x)</td>
            <td> Subsections of functions may be commented 
                 using the SUBSECTION_LOG macro
            </td>
         </tr><tr>  
            <td><b>level 5</b></td>    
            <td>LOOP_LOG(x)</td>
            <td> Debug messages that occur in long loops, e.g. while
                 iteration over all image pixels, may be written using the
                 LOOP_LOG macro. Note, that these messages can slow down 
                 the iteration time to less then 0.1%.
            </td>
         </tr>
      </table>

The following example will show how to use the DEBUG-Macros 
provided in ImgMacros.h
  <pre>
  int sum_vec(int *piVec, int len){
     FUNCTION_LOG("int *, int");
     ImgASSERT(piVec); // calls ERROR_LOG

     SECTION_LOG("temp. variale allocation");
     int sum = 0;

     SECTION_LOG("starting loop");
     for(int i=0;i<len;i++){
        LOOP_LOG("addition loop, index: " << i << "curr. value: " << sum);
        sum += piVec[i];
        // WARNING_LOG e.g. for range overflow for the int accumulator "sum"
      }
     SECTION_LOG("return sum");
     return sum;
  }
  </pre>
@see ImgMacros.h
*/

#include "Macros.h"
#include "Size.h"
#include "Point.h"
#include "Rect.h"


#include <string>
#include <vector>
#ifdef WITH_IPP_OPTIMIZATION
#include <ipp.h>
#endif

/// The ICL-namespace
/**
This namespace is dedicated for ICLCore- and all additional Computer-Vision
packages, that are based on the ICLCore classes.
**/
namespace icl {
  
#ifdef WITH_IPP_OPTIMIZATION
  /// 32Bit floating point type for the ICL 
  typedef Ipp32f icl32f;

  /// 8Bit unsigned integer type for the ICL
  typedef Ipp8u icl8u;

#else
  /// 32Bit floating point type for the ICL 
  typedef float icl32f;

  /// 8Bit unsigned integer type for the ICL 
  typedef unsigned char icl8u;
#endif

  //forward declaration for the Image interface
  class ImgI;

  /// forward declaration of the Img-class
  template<class T> class Img;

  /// typedef for 8bit integer images
  typedef Img<icl8u> Img8u;

  /// typedef for 32bit float images
  typedef Img<icl32f> Img32f;


  /// determines the pixel type of an image (8Bit-int or 32Bit-float) 
  enum depth{
    depth8u  = 0, /**< 8Bit unsigned integer values range {0,1,...255} */
    depth32f = 1 /**< 32Bit floating point values */
  };
  
  /// determines the color-format, that is associated with the images channels 
  enum format{
    formatRGB, /**< (red,green,blue) colors pace */
    formatHLS, /**< (hue,lightness,saturation) color space (also know as HSI) */
    formatLAB, /**< (lightness,a*,b*) color space */
    formatYUV, /**< (Y,u,v) color space */
    formatGray, /**< n-channel gray image range of values is [0,255] as default */
    formatMatrix /**< n-channel image without a specified color space. */
  };

#ifdef WITH_IPP_OPTIMIZATION
  enum scalemode{
    interpolateNN=IPPI_INTER_NN,      /**< nearest neighbor interpolation */
    interpolateLIN=IPPI_INTER_LINEAR, /**< bilinear interpolation */
    interpolateRA=IPPI_INTER_SUPER    /**< region-average interpolation */
  };
#else
  /// for scaling of Img images theses functions are provided
  enum scalemode{
    interpolateNN,  /**< nearest neighbor interpolation */
    interpolateLIN, /**< bilinear interpolation */
    interpolateRA   /**< region-average interpolation */
  };
#endif

  /// for flipping of images
  enum axis{
    axisHorz, /**> horizontal image axis */
    axisVert  /**> vertical image axis */
  };

/* {{{ Global classes */
  
  /// Casting operator
  /** Use Cast<srcT, dstT>::cast (value) to cast values safely from
      one srcT type to dstT. If destination type is icl8u, the source
      value is clipped to the range [0..255].
  */
  template<typename srcT, typename dstT> 
  struct Cast {
     static dstT cast (srcT v) {return static_cast<dstT>(v);}
  };
  template<typename srcT>
  struct Cast<srcT, icl8u> {
     static icl8u cast (srcT v) {
        return static_cast<icl8u>(std::min ((srcT) 255, std::max ((srcT) 0, v)));
     }
  };
  template<typename T>
  struct Cast<T, T> {
     static T cast (T v) {return v;}
  };
  template<>
  struct Cast<icl8u, icl8u> {
     static icl8u cast (icl8u v) {return v;}
  };

/* }}} */

/* {{{ Global functions */

  /// creates a new ImgI by abstacting from the depth parameter
  /** This function is essention for the abstaction mechanism about 
      Img image classes  underlying depth. In many cases you might have an
      ImgI*, wich must be initialized with parameters size,
      channel count and - which is the problem - the depth. The
      default solution is to insert an if statement. Look at the 
      following Example, that shows the implementation of a class
      constructor.
      <pre>
      class Foo{
         public:
         Foo(...,Depth eDepth,...):
             poImage(eDepth==depth8u ? new Img8u(...) : new Img32f(...)){
         }
         private:
         ImgI *poImage;         
      };
      </pre>
      This will work, but the "?:"-statement make the code hardly readable.
      The following code extract will show the advantage of using the iclNew
      function:
      <pre>
      class Foo{
         public:
         Foo(...,Depth eDepth,...):
             poImage(iclNew(eDepth,...)){
         }
         private:
         ImgI *poImage;         
      };
      The readability of the code is much better.
      </pre>
  
      @param eDepth depth of the image that should be created
      @param s size of the new image
      @param eFormat format of the new image
      @param iChannels channel count of the new image. If < 0,
                       then the channel count associated with 
                       eFormat is used
      @param oROI ROI rectangle of the new image
      @return the new ImgI* with underlying Img<Type>, where
              Type is depending on the first parameter eDepth
  **/
  ImgI *imgNew(depth eDepth=depth8u, 
               const Size& s=Size(1,1),
               format eFormat=formatMatrix,
               int iChannels = -1,
               const Rect &oROI=Rect());


  
  /// ensures that an image has the specified depth
  /** This function will delete the original image pointed by (*ppoImage)
      and create a new one with identical parameters, if the given depth
      parameter is not the images depth. If the given image pointer
      (*ppoImage) is NULL, then a new image is created with size 1x1,
      one channel and specified depth.
      @param ppoImage pointer to the image-pointer
      @param eDepth destination depth of the image
  **/
  void ensureDepth(ImgI **ppoImage, depth eDepth);

  /// ensures that two images have the same size, channel count, depth, format and ROI
  /** If the given dst image image is 0 than it is created as a (deep copy) of
      of poSrc.
      @param ppoDst points the destination ImgI*. If the images depth has to be
                    converted, then a new ImgCore* is created, at (*ppoDst).
      @param poSrc source image. All params of this image are extracted to define
                   the destination parameters for *ppoDst.  
  **/
  void ensureCompatible(ImgI **ppoDst, ImgI *poSrc);

  /// ensures that two images have the same size, channel count, depth, format and ROI
  /** If the given dst image image is 0 than it is created as a (deep copy) of
      of poSrc.
      @param ppoDst points the destination ImgI*. If the images depth has to be
                    converted, then a new ImgCore* is created, at (*ppoDst).
      @param eDepth destination depth
      @param s destination image size
      @param eFormat destination format
      @param iChannelCount destination channel count. (If -1, then the channel count
                           is extracted from the given eFormat
      @param roROI destination images ROI rectangle. If the ROI parameters are not 
                   given, the ROI will comprise the whole image.
  **/
  void ensureCompatible(ImgI **ppoDst,
                        depth eDepth, 
                        const Size& s,
                        format eFormat, 
                        int iChannelCount=-1,
                        const Rect &roROI=Rect());
  
  /// determines the count of channels, for each color format
  /** @param eFormat source format which channel count should be returned
      @return channel count of format eFormat
  **/
  int getChannelsOfFormat(format eFormat);


  /// returns a string representation of an Format enum
  /** @param eFormat Format enum which string repr. is asked 
      @return string representation of eFormat
  **/
  string translateFormat(format eFormat);  
  
  /// returns an Format enum, specified by a string 
  /** This functions implements the opposite direction to the above function,
      which means, that:
      <pre>
      translateFormat(translateFormat(x)) == x
      </pre>
      If x is a string or an Format enum.
      @param sFormat string representation of the format 
                     which should be returned
      @return Format, that corresponds to sFormat
  **/
  format translateFormat(string sFormat);

  /// returns a string representation for a depth value
  inline string translateDepth(depth eDepth){
    return eDepth == depth8u ? "depth8u" : "depth32f";
  }
  
  /// creates a depth value form a depth string
  inline depth translateDepth(string sDepth){
    return sDepth == "depth8u" ? depth8u : depth32f;
  }

  /// getDepth<T> returns to depth enum associated to type T
  template<class T> 
  inline depth getDepth() { return depth8u; }

  /// getDepth<T> returns to depth enum associated to type T
  template<> 
  inline depth getDepth<icl8u>() { return depth8u; }
  
  /// getDepth<T> returns to depth enum associated to type T
  template<> 
  inline depth getDepth<icl32f>() { return depth32f; }

  /// return sizeof value for the given depth type
  int getSizeOf(depth eDepth);


  /// moves value from source to destination array (with casting on demand)
  template <class srcT,class dstT>
  inline void copy(srcT *poSrcStart, srcT *poSrcEnd, dstT *poDst){
    while(poSrcStart != poSrcEnd) *poDst++ = Cast<srcT,dstT>::cast(*poSrcStart++);
  }

#ifdef WITH_IPP_OPTIMIZATION
  template<>
  inline void copy<icl8u,icl32f>(icl8u *poSrcStart, icl8u *poSrcEnd, icl32f *poDst){
    ippsConvert_8u32f(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }
  template <>
  inline void copy<icl32f,icl8u>(icl32f *poSrcStart, icl32f *poSrcEnd, icl8u *poDst){
    ippsConvert_32f8u_Sfs(poSrcStart,poDst,(poSrcEnd-poSrcStart),ippRndNear,1);
  } 
  template <>
  inline void copy<icl8u,icl8u>(icl8u *poSrcStart, icl8u *poSrcEnd, icl8u *poDst){
    ippsCopy_8u(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }
  template <>
  inline void copy<icl32f,icl32f>(icl32f *poSrcStart, icl32f *poSrcEnd, icl32f *poDst){
    ippsCopy_32f(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }
#else
  template <typename T>
  inline void copy(T *poSrcStart, T *poSrcEnd, T *poDst){
    memcpy(poDst,poSrcStart,(poSrcEnd-poSrcStart)*sizeof(T));
  }
#endif

 
                          
 

  /* }}} */

} // namespace icl

#endif
