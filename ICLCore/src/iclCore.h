#ifndef ICL_CORE_H
#define ICL_CORE_H

#include <iclMacros.h>
#include <iclTypes.h>
#include <iclImgParams.h>
#include <string>
#include <cstring>
#include <iostream>
#include <iclPoint32f.h>
#include <iclClippedCast.h>

/** 
    \defgroup TYPES Common Data Type Definitions
    \defgroup GENERAL General Utility and Support Functions
    \defgroup IMAGE Image Classes and Support Functions
    \defgroup STRUTILS Utiltiy functions for std::string conversions
    \defgroup MATH Mathematical Utiltiy functions

\mainpage Image Component Library (ICL) 

At its core, ICL is a C++ computer vision library. During the design and development process, the following main goals took center stage:
 - Optimal Performace
 - Simple and easy to use C++-interface
 - Platform-Independence
 - No compulsory software dependencies

ICL tutorials can be build independently from the ICL/tutorial folder, or downloaded from <b>TODO</b>

\section PACKAGES Packages

ICL consists of currently 11 packages that are listed in the main menu at the left.

 - <b>ICLUtils</b> Contains general purpose functions and classes that are currently not part of the C++-STL (e.g. threads or matrices).
 - <b>ICLCore</b> basically provides class definitions for ICL's image classes Img and ImgBase and related global functions.
 - <b>ICLCC</b> provides functions and classes for color conversion.
 - <b>ICLIO</b> extends the range of functions by input and output classes. Camera grabbers different camera types (e.g. IEEE-1394 or Video-4-Linux) can be found here as well a video file grabber or a file writer class.
 - <b>ICLBlob</b> contains classes for blob detection and tracking and for connected component analysis.
 - <b>ICLFilter</b> provides classes for most common image filters like linear filters and morphological operators.
 - <b>ICLQuick</b> provides almost 100 functions and functors for rapid prototyping
 - <b>ICLGeom</b> contains classes for 3D-modelling and camera calibration. 
 - <b>ICLQt*</b> contains a Qt-4 based GUI-API that facilitates creation of simple and complex GUI applications significantly. And of course a powerful image visualisation widget called ICLWidget is provided.
 - <b>ICLAlgorithms</b> contains high level classes like a hough-transformation-based line detector or generic self organizing map (SOM) implementation. 
 - <b>ICLOpenCV*</b> offers functions for shallow and deep copies from ICL-images types into OpenCV's images types and v.v.
    
    *) The packages ICLQt and ICLOpenCV depend compulsorily on the corresponding external software dependencies Qt4 and OpenCV. Consequently these packages are not available if these dependencies are missing.

\section THE_IMAGE The Image Classes

We use inheritance and class templates for ICL image representation:
The ImgBase class defines an abstract interface, that manages all image information except image pixel data. These abstract image features are:

 - size (in pixels)
 - channel count  (see <b>channel concept</b>)
 - type of pixels (see <b>data types</b>)
 - color format (see <b>color formats</b>)
 - raw image data access
 - Region of Interest (see <b>Region of Interests</b> \ref ROI (ROI)) 
 - a time stamp 

The ImgBase interfaces is implemented by the template class Img<T> which implements all abstract ImgBase-functions and aggregates a vector of planar image channel data pointers. Internally, these channel data pointers use reference counting to allow shallow image copies. \n
<b>Img's copy-constructor and assignment operator use shallow copy on default!</b>

The Img<T> template also adds functions for type-safe data access:

 - access to channel data pointers (using getData(channel) or begin(channel))
 - extraction of single image channels (using operator []) 
 - extraction of single image pixels (using operator()(x,y,channel-index) for single values or operator()(x,y) to obtain a pixel vector)

 
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

\section ROI region of Interest (ROI)
Each image can be set up with a rectangular region of interest. Algorithms 
work only on the pixels within the ROI. If a function does not support
ROI handling it will be noticed in the documentation. There are several ways
to realize ROI handling in functions. The most common way is to use the
ImgIterator with can be accessed using the STL-style functions beginROI(channel) and
endROI(channel). 

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

\section CONST_SEC Const-Concept
ICL Images use the const concept of C++ to ensure pixel data of
const Images (of type const ImgBase or more precisely const Img<T>)
is not changed, i.e. it is only accessible for reading. Unfortunately
this leads to a conflict with the "shallow-copy" concept of ICL images.
\code
void func(const Img8u &image){
    // given image is const -> data must not be changed
    Img8u x = image;
    // x is a shallow copy of image (data is shared)
    x.clear();
    // this affects also the data of image (which shall not
    // be permitted
}
\endcode
To avoid this conflict, we tried to forbid creating un-const shallow
copies of const images by implementing no default copy constructor:
\code
Img<T>(const Img<T> &other) {... }
\endcode
but an un-const version of this:
\code
Img<T>(Img<T> &other) {... }
\endcode
Here we face some GCC related problem, because gcc is not able
for an implicit cast of an Img<T> to an Img<T>& in constructor
calls:
\code
template<class T> class Img<T>{ ... };

Img8u create_image(){ 
    return Img8u(); 
}

int main(){
    Img8u a = create_image();
    return 0;
}
\endcode
Here, the compiler gives error: "Can't find constructor Img<T>(Img<T>)".
In fact, this constructor can not exist: it must have the following
syntax: Img<T>(Img<T>&)

Probably further gcc versions will fix this problem!

<b>Until then, we accept the const leak at constructor and assignment
operator and reimplemented them as ..(const Img<T> &other) </b>


\section IPP-Optimization
The IPP Intel Performance Primitives is a c-library that contains highly 
optimized and hardware accelerated functions for image processing, and 
other numerical problems for all processors providing the SSE command set, 
i.e. most new Intel and AMD processors.
To provide access to IPP/IPPI functionality, the ImgCore library can be 
compiled with <b>HAVE_IPPS</b> defined. In this case, the 
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

\section ICLCore Modules
    If you like to explore the ICLCore documentation by your own, take a 
    look a the following modules:\n
    -# \ref TYPES 
    -# \ref GENERAL
    -# \ref IMAGE 
    -# \ref STRUTILS
    -# \ref MATH 

/// The ICL-namespace
/** This namespace is dedicated for ICLCore- and all additional Computer-Vision
    packages, that are based on the ICLCore classes.
**/ namespace icl {
 
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

  /// getDepth<T> returns to depth enum associated to type T \ingroup GENERAL
  template<class T> inline depth getDepth();


  /// puts a string representation of format into the given stream
  std::ostream &operator<<(std::ostream &s,const format &f);
  
  /// puts a string representation of depth into the given stream
  std::ostream &operator<<(std::ostream &s,const depth &d);

  /// puts a string representation of format into the given stream
  std::istream &operator>>(std::istream &s, format &f);
  
  /// puts a string representation of depth into the given stream
  std::istream &operator>>(std::istream &s, depth &d);

  
  /** \cond */ #define ICL_INSTANTIATE_DEPTH(T) \
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

  /** \cond */ #ifdef HAVE_IPP
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
    std::transform(poSrcStart,poSrcEnd,poDst,clipped_cast<srcT,dstT>);
  }
  
#ifdef HAVE_IPP 
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
