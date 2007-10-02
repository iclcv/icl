#ifndef ICLCORE_H
#define ICLCORE_H
#include <iclMacros.h>
#include <iclTypes.h>
#include <iclImgParams.h>
#include <string>


/** 
    \defgroup TYPES Common Data Type Definitions
    \defgroup GENERAL General Utility and Support Functions
    \defgroup IMAGE Image Classes and Support Functions
    \defgroup STRUTILS Utiltiy functions for std::string conversions
    \defgroup MATH Mathematical Utiltiy functions

\mainpage ICL (Image-Component-Library) : ICLCore 
\section TODO


\section SEC_OVERVIEW Overview

The ICL is a C++ Image-Library, designed for Computer-Vision tasks. It
supports a multi-channel image class (class ImgBase and template class Img).
All channels within the image share a common size, region of interest (ROI) and
depth. This allows to handle color images as 3-channel images for example.

Despite of the different image depth, most methods of an image class have
common code. Hence, the different pixel data types are implemented by the
template class <b>Img<imagedepth></b>. Methods which are independent on the
image depth are provided by a common interface class, named <b>ImgBase</b>. This
allows easy and type-clean wrapping of the template classes Img<T> within frameworks
such as Neo/NST or TDI.

\subsection Modules
    If you like to explore the ICLCore documentation by your own, take a 
    look a the following modules:\n
    -# \ref TYPES 
    -# \ref GENERAL
    -# \ref IMAGE 
    -# \ref STRUTILS
    -# \ref MATH 

\subsection SUB_SEC_IMGBASE class ImgBase (abstract interface) 
    This class provides common, but depth-independent information about 
    the image structure:
  - size (in pixels)
  - channel count  (see <b>channel concept</b>)
  - type of pixels (see <b>data types</b>)
  - color format (see <b>color formats</b>)
  - raw image data access
  - Region of Interest (see <b>Region of Interests</b> (ROI))

  It has no public constructors so it has to be used as interface
  class for the derived template classes Img<Type>.
  Most of the functions in the ImgBase class are purely virtual which
  implies, that they are implemented in the derived classes Img<T>.

\subsection SUB_SEC_IMG template class Img<PixelType>
  Internally each Img<T> object holds a std::vector of so called smart 
  pointers (class SmartPtr in the ICLUtils package) to the channel
  data. The Img class provides some additional image information and access
  functions:
  - type-save image data access ( using the functions 
    getData(int channel) and getROIData(int channel)
  - access to single pixel values using the ()-operator
  - access to all Image/all ROI pixels using the ImgIterator 
  (see <b>ImgIterator</b> class reference)
    
In addition to the classes Img and ImgBase, the ICLCore package
provides some utility functions, that facilitates working with 
these classes (see icl namespace for more details).
  

@see ImgBase, Img

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
Currently the Img provides 5 different data types:
- <b>icl8u</b> 8bit unsigned char
- <b>icl16s</b> 16bit signed integer (short)
- <b>icl32s</b> 32bit signed integer (int)
- <b>icl32f</b> 32bit single precision float
- <b>icl64f</b> 64bit double precision float

Img-classes are predefined for these two types:
- Img<icl8u> : public ImgBase <b>typedef'd to Img8u</b>
- Img<icl16s> : public ImgBase <b>typedef'd to Img16s</b>
- Img<icl32s> : public ImgBase <b>typedef'd to Img32s</b>
- Img<icl32f> : public ImgBase <b>typedef'd to Img32f</b>
- Img<icl64f> : public ImgBase <b>typedef'd to Img64f</b>


Each of these data types has several advantages/disadvantages. The greatest
disadvantage of the integer types, is their bounded range (e.g. 0-255 for icl8u),
which has the effect, that all information has to be scaled to this
range, and all image processing functions must take care that
no range-overflow occurs during calculation. Furthermore
the limited range may cause loss of information - particular in 
complex systems.
The advantage of integer types is, that computation is faster
than using floats.
 
@see Depth, icl8u, icl32f

\section Color Formats
An ImgBase image provides some information about the (color) format, that
is associated with the image data represented by the images channels. Color
is written in brackets, as not all available formats imply color-information.
The most known color space is probably the RGB color space. 
If an ImgBase image has the format <i>formatRGB</i>, than this implies the 
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



/// The ICL-namespace
/**
This namespace is dedicated for ICLCore- and all additional Computer-Vision
packages, that are based on the ICLCore classes.
**/
namespace icl {

/* {{{ clip function*/

  /// clips a value into the range [tMin,tMax] \ingroup GENERAL
  template <class T>
  inline T clip(T tX, T tMin, T tMax){ return tX < tMin ? tMin : tX > tMax ? tMax : tX; }
 
  /* }}} */

/* {{{ Cast class */
  
  /// Generic Casting operator \ingroup GENERAL
  /** Use Cast<srcT, dstT>::cast (value) to cast values safely from
      one srcT type to dstT. If destination type is e.g. icl8u, the source
      value is clipped to the range [0..255].
  */
  template<typename srcT, typename dstT> struct Cast {
     static dstT cast (srcT v) {return static_cast<dstT>(v);}
  };

  /** \cond */
  /// casting class from any to icl8u type
  template<class srcT> struct Cast<srcT,icl8u>{
    static icl8u cast (srcT v) { return static_cast<icl8u>(clip<srcT>(v,0,255)); }
  };
  
  /// casting class form any to icl16s type
  template<class srcT> struct Cast<srcT,icl16s>{
    static icl16s cast (srcT v) { return static_cast<icl16s>(clip<srcT>(v,-32767,32768)); }
  };
  /// casting class specialized form icl8u to icl16s type
  template<> struct Cast<icl8u,icl16s>{
    static icl16s cast (icl8u v) { return static_cast<icl16s>(v); }
  };
   
  /// casting class form any to icl32s type
  template<class srcT> struct Cast<srcT,icl32s>{
    static icl32s cast (srcT v) { return static_cast<icl32s>(clip<srcT>(v,-2147483647,2147483647)); }
  };
  /// casting class specialized form icl8u to icl32s type
  template<> struct Cast<icl8u,icl32s>{
    static icl32s cast (icl8u v) { return static_cast<icl32s>(v); }
  };
  /// casting class specialized form icl16s to icl32s type
  template<> struct Cast<icl16s,icl32s>{
    static icl32s cast (icl16s v) { return static_cast<icl32s>(v); }
  };

  /// casting class form T to T type (just returning the param itself)
  template<typename T> struct Cast<T, T> {
    static T cast (T v) {return v;}
  };

  /// casing class form icl8u to icl8u type
  template<> struct Cast<icl8u, icl8u> {
     static icl8u cast (icl8u v) {return v;}
  };
  /// casing class form icl16s to icl16s type
  template<> struct Cast<icl16s, icl16s> {
     static icl16s cast (icl16s v) {return v;}
  };
  /// casing class form icl32s to icl32s type
  template<> struct Cast<icl32s, icl32s> {
     static icl32s cast (icl32s v) {return v;}
  };
  /// casing class form icl32f to icl32f type
  template<> struct Cast<icl32f, icl32f> {
     static icl32f cast (icl32f v) {return v;}
  };
  /// casing class form icl64f to icl64f type
  template<> struct Cast<icl64f, icl64f> {
     static icl64f cast (icl64f v) {return v;}
  };
  /** \endcond */

/* }}} */
 
/* {{{ Global functions */

/// create a new image instance of the given depth type and with given parameters \ingroup IMAGE
/** This function provides a common interface to instantiate images of 
    arbitrary depth, selecting the appropriate constructor of the derived class
    Img<T>. 

    Instead of selecting the correct constructor by yourself, you can simply
    call this common constructor function. To illustrate the advantage, look
    at the following example:
    <pre>
      class Foo{
         public:
         Foo(...,Depth eDepth,...):
             poImage(eDepth==depth8u ? new Img8u(...) : new Img32f(...)){
         }
         private:
         ImgBase *poImage;         
      };
    </pre>
    This will work, but the "?:"-statement makes the code hardly readable.
    The following code extract will show the advantage of using the imgNew instantiator:
    <pre>
      class Foo{
         public:
         Foo(...,Depth eDepth,...):
             poImage(imgNew(eDepth,...)){
         }
         private:
         ImgBase *poImage;         
      };
    </pre>
    The readability of the code is much better.
  
    @param d depth of the image that should be created
    @param params struct containing all neccessary parameters like:
      size: size of the new image
      fmt:  format of the new image
      channels: number of channels (of an formatMatrix-type image)
      roi:  ROI rectangle of the new image
    @return the new ImgBase* with underlying Img<Type>, where
            Type is depending on the first parameter eDepth
  **/
  ImgBase *imgNew(depth d=depth8u, const ImgParams &params = ImgParams::null);
  
  /// creates a new Img (see the above function for more details) \ingroup IMAGE
  inline ImgBase *imgNew(depth d, const Size& size, format fmt, const Rect &roi=Rect::null){
    return imgNew(d,ImgParams(size,fmt,roi));
  }

  /// creates a new Img (see the above function for more details) \ingroup IMAGE
  inline ImgBase *imgNew(depth d, const Size& size, int channels=1, const Rect &roi=Rect::null){
    return imgNew(d,ImgParams(size,channels,roi));
  }
 
  /// creates a new Img (see the above function for more details) \ingroup IMAGE
  inline ImgBase *imgNew(depth d, const Size& size, int channels, format fmt, const Rect &roi=Rect::null){
    return imgNew(d,ImgParams(size,channels,fmt,roi));
  }

  
  /// ensures that an image has the specified depth \ingroup IMAGE
  /** This function will delete the original image pointed by (*ppoImage)
      and create a new one with identical parameters, if the given depth
      parameter is not the images depth. If the given image pointer
      (*ppoImage) is NULL, then an empty image of specified depth es created
      at *ppoImage.
      If ppoImage is NULL a new Image is created and returned.
      call:
      <pre>
      void func(ImgBase **ppoDst){
         ImgBase *poDst = ensureDepth(ppoDst,anyDepth);
      }
      </pre>
      to ensure that an image is valid.
      @param ppoImage pointer to the image-pointer
      @param eDepth destination depth of the image
      @return the new image (this can be used e.g. if ppoImage is NULL)
  **/
  ImgBase *ensureDepth(ImgBase **ppoImage, depth eDepth);

  /// ensures that an image has given depth and parameters \ingroup IMAGE
  ImgBase *ensureCompatible(ImgBase **dst, depth d,const ImgParams &params);

  /// ensures that an image has given depth, size, number of channels and ROI \ingroup IMAGE
  /** If the given pointer to the destination image is 0, a new image with appropriate
      properties is created. Else the image properties are checked and adapted to the new
      values if neccessary.
      @param dst points the destination ImgBase*. If the images depth hasa to be
                 converted, then a new Img<T>* is created at (*dst).
      @param d desired image depth
      @param size desired image size
      @param channels desired number of channels, if eFormat == formatMatrix
                      for other format, the number of channels is determined by the format
      @param roi desired ROI rectangle. If the ROI parameters are not given, 
                 the ROI will comprise the whole image.
  **/
  inline ImgBase *ensureCompatible(ImgBase **dst, depth d,const Size& size,int channels, const Rect &roi=Rect::null)
     { return ensureCompatible(dst,d,ImgParams(size,channels,roi)); }

  /// ensures that an image has given depth, size, format and ROI \ingroup IMAGE
  inline ImgBase *ensureCompatible(ImgBase **dst, depth d,const Size& size, format fmt, const Rect &roi=Rect::null)
     { return ensureCompatible(dst,d,ImgParams(size,fmt,roi)); }
  
  /// ensures that an image has given parameters  \ingroup IMAGE
  /** The given format must be compatible to the given channel count.
      <b>If not:</b> The format is set to "formatMatrix" and an exception is thrown.
  */
  ImgBase *ensureCompatible(ImgBase **dst, depth d, const Size &size, int channels, format fmt, const Rect &roi=Rect::null);
  
  /// ensures that the destination image gets same depth, size, channel count, depth, format and ROI as source image \ingroup IMAGE
  /** If the given pointer to the destination image is 0, a new image is created as a deep copy of poSrc.
      Else the image properties are checked and adapted to the new values if neccessary.
      <b>Note:</b> If the destination images depth differs from the source images depth, it is adapted by
      deleting the <em>old</em> destination pointer by calling <em>delete *ppoDst</em> and creating a
      <em>brand new</em> Img<T> where T is the destination images depth.
      @param dst points the destination ImgBase*. If the images depth has to be
                    converted, then a new Img<T>* is created, at (*ppoDst).
      @param src  source image. All params of this image are extracted to define
                    the destination parameters for *ppoDst.  
  **/
  ImgBase *ensureCompatible(ImgBase **dst, const ImgBase *src);

  /// determines the count of channels, for each color format \ingroup GENERAL
  /** @param fmt source format which channel count should be returned
      @return channel count of format eFormat
  **/
  int getChannelsOfFormat(format fmt);


  /// returns a string representation of an Format enum \ingroup GENERAL
  /** @param eFormat Format enum which string repr. is asked 
      @return string representation of eFormat
  **/
  std::string translateFormat(format eFormat);  
  
  /// returns an Format enum, specified by a string \ingroup GENERAL
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
  format translateFormat(const std::string& sFormat);

  /// returns a string representation for a depth value \ingroup GENERAL
  std::string translateDepth(depth eDepth);

  /// creates a depth value form a depth string \ingroup GENERAL
  depth translateDepth(const std::string& sDepth);

  /// creates a size string like "640x480" \ingroup GENERAL
  std::string translateSize(const Size &size);
  
  /// translates a size string into a size variable \ingroup GENERAL
  /** if the string could not be parsed, the returned size is "0x0" */
  Size translateSize(const std::string &sSize);

  /// creates a rect string like "640x480@(5,10)" \ingroup GENERAL
  std::string translateRect(const Rect &rect);
  
  /// translates a rect string into a Rect variable \ingroup GENERAL
  /** if the string could not be parsed, the returned Rect is "0x0@(0,0)" */
  Rect translateRect(const std::string &sRect);

  /// creates a point string like "640x480@(5,10)" \ingroup GENERAL
  std::string translatePoint(const Point &p);
  
  /// translates a point string into a Point variable \ingroup GENERAL
  /** if the string could not be parsed, the returned Poin is "(0,0)" */
  Point translatePoint(const std::string &p);
  
  /// getDepth<T> returns to depth enum associated to type T \ingroup GENERAL
  template<class T> inline depth getDepth();

  /** \cond */
#define ICL_INSTANTIATE_DEPTH(T) \
  template<> inline depth getDepth<icl ## T>() { return depth ## T; }
ICL_INSTANTIATE_ALL_DEPTHS  
#undef ICL_INSTANTIATE_DEPTH
 
  /** \endcond  */
  /// return sizeof value for the given depth type \ingroup GENERAL
  unsigned int getSizeOf(depth eDepth);

  /// moves data from source to destination array (no casting possible) \ingroup GENERAL
  template <class T>
  inline void copy(const T *src, const T *srcEnd, T *dst){
    //std::copy<T>(src,srcEnd,dst);
    memcpy(dst,src,(srcEnd-src)*sizeof(T));
  } 

  /** \cond */
#ifdef WITH_IPP_OPTIMIZATION
  template <>
  inline void copy<icl8u>(const icl8u *poSrcStart, const icl8u *poSrcEnd, icl8u *poDst){
    ippsCopy_8u(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }
  template <>
  inline void copy<icl16s>(const icl16s *poSrcStart, const icl16s *poSrcEnd, icl16s *poDst){
    ippsCopy_16s(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }
  template <>
  inline void copy<icl32s>(const icl32s *poSrcStart, const icl32s *poSrcEnd, icl32s *poDst){
    ippsCopy_32s(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }
  template <>
  inline void copy<icl32f>(const icl32f *poSrcStart, const icl32f *poSrcEnd, icl32f *poDst){
    ippsCopy_32f(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }
  template <>
  inline void copy<icl64f>(const icl64f *poSrcStart, const icl64f *poSrcEnd, icl64f *poDst){
    ippsCopy_64f(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }
#endif
  /** \endcond */



  /// moves value from source to destination array (with casting on demand) \ingroup GENERAL
  template <class srcT,class dstT>
  inline void convert(const srcT *poSrcStart,const srcT *poSrcEnd, dstT *poDst){
    while(poSrcStart != poSrcEnd) *poDst++ = Cast<srcT,dstT>::cast(*poSrcStart++);
  }
  
  // Problems here! (if defined, the compiler gets problems when resolving the most-optimized function
  /** \cond */
  /**
      #define ICL_INSTANTIATE_DEPTH(D) template<> inline void convert<icl##D,icl##D>  \
      (const T* src, const T*srcEnd, T *dst){        \
      icl::copy<icl##D>(src,srcEnd,dst); }
      ICL_INSTATIATE_ALL_DEPTHS
      #undef ICL_INSTANTIATE_DEPTH
   **/ 
  
#ifdef WITH_IPP_OPTIMIZATION 
  /// from icl8u functions
  template<> inline void convert<icl8u,icl32f>(const icl8u *poSrcStart,const icl8u *poSrcEnd, icl32f *poDst){
    ippsConvert_8u32f(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }
  /// from icl16s functions
  template<> inline void convert<icl16s,icl32s>(const icl16s *poSrcStart,const icl16s *poSrcEnd, icl32s *poDst){
    ippsConvert_16s32s(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }
  template<> inline void convert<icl16s,icl32f>(const icl16s *poSrcStart,const icl16s *poSrcEnd, icl32f *poDst){
    ippsConvert_16s32f(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }
  template<> inline void convert<icl16s,icl64f>(const icl16s *poSrcStart,const icl16s *poSrcEnd, icl64f *poDst){
    ippsConvert_16s64f_Sfs(poSrcStart,poDst,(poSrcEnd-poSrcStart),0);
  }
  
  // from icl32s functions
  template<> inline void convert<icl32s,icl16s>(const icl32s *poSrcStart,const icl32s *poSrcEnd, icl16s *poDst){
    ippsConvert_32s16s(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }
  template<> inline void convert<icl32s,icl32f>(const icl32s *poSrcStart,const icl32s *poSrcEnd, icl32f *poDst){
    ippsConvert_32s32f(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }
  template<> inline void convert<icl32s,icl64f>(const icl32s *poSrcStart,const icl32s *poSrcEnd, icl64f *poDst){
    ippsConvert_32s64f(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }

  // from icl32f functions
  template <> inline void convert<icl32f,icl8u>(const icl32f *poSrcStart, const icl32f *poSrcEnd, icl8u *poDst){
    ippsConvert_32f8u_Sfs(poSrcStart,poDst,(poSrcEnd-poSrcStart),ippRndNear,0);
  } 
  template <> inline void convert<icl32f,icl16s>(const icl32f *poSrcStart, const icl32f *poSrcEnd, icl16s *poDst){
    ippsConvert_32f16s_Sfs(poSrcStart,poDst,(poSrcEnd-poSrcStart),ippRndNear,0);
  } 
  template <> inline void convert<icl32f,icl32s>(const icl32f *poSrcStart, const icl32f *poSrcEnd, icl32s *poDst){
    ippsConvert_32f32s_Sfs(poSrcStart,poDst,(poSrcEnd-poSrcStart),ippRndNear,0);
  } 
  template <> inline void convert<icl32f,icl64f>(const icl32f *poSrcStart, const icl32f *poSrcEnd, icl64f *poDst){
    ippsConvert_32f64f(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  } 

  // from icl64f functions 
  template<> inline void convert<icl64f,icl32f>(const icl64f *poSrcStart,const icl64f *poSrcEnd, icl32f *poDst){
    ippsConvert_64f32f(poSrcStart,poDst,(poSrcEnd-poSrcStart));
  }
  template <> inline void convert<icl64f,icl32s>(const icl64f *poSrcStart,const icl64f *poSrcEnd, icl32s *poDst){
    ippsConvert_64f32s_Sfs(poSrcStart,poDst,(poSrcEnd-poSrcStart),ippRndNear,0);
  }
#endif 
  /** \endcond */

 
  /// function, that calculates the mininum and the maximum value of three value \ingroup GENERAL
  /** @param a first input value 
      @param b second input value 
      @param c third input value 
      @param minVal return value for the minimum
      @param maxVal return value for the maximum
  **/
  template<class T>
  inline void getMinAndMax(T a, T b, T c, T &minVal, T &maxVal){
    if(a<b) {
      minVal=a;
      maxVal=b;
    }
    else {
      minVal=b;
      maxVal=a;	
    }
    if(c<minVal)
      minVal=c;
    else {
      maxVal = c>maxVal ? c : maxVal;
    }
  }  
                          
 

  /* }}} */

} // namespace icl
#endif
