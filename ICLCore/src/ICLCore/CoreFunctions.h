/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/CoreFunctions.h                    **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter, Michael Goetting, Robert          **
**          Haschke, Andre Justus, Sergius Gaulik                  **
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

#include <ICLUtils/Macros.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/ClippedCast.h>
#include <ICLUtils/PThreadFix.h>
#include <ICLUtils/SSEUtils.h>

#include <ICLCore/Types.h>
#include <ICLCore/ImgParams.h>

#include <string>
#include <cstring>
#include <iostream>
#include <vector>

/// The ICL-namespace
/** This namespace is dedicated for ICLCore- and all additional Computer-Vision
    packages, that are based on the ICLCore classes.
**/ 
namespace icl {
  namespace core{
   
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
    ICLCore_API ImgBase *imgNew(depth d = depth8u, const ImgParams &params = ImgParams::null);
    
    /// creates a new Img (see the above function for more details) \ingroup IMAGE
    inline ImgBase *imgNew(depth d, const utils::Size& size, format fmt, 
                           const utils::Rect &roi=utils::Rect::null){
      return imgNew(d,ImgParams(size,fmt,roi));
    }
  
    /// creates a new Img (see the above function for more details) \ingroup IMAGE
    inline ImgBase *imgNew(depth d, const utils::Size& size, int channels=1, 
                           const utils::Rect &roi=utils::Rect::null){
      return imgNew(d,ImgParams(size,channels,roi));
    }
   
    /// creates a new Img (see the above function for more details) \ingroup IMAGE
    inline ImgBase *imgNew(depth d, const utils::Size& size, int channels, format fmt, 
                           const utils::Rect &roi=utils::Rect::null){
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
    ICLCore_API ImgBase *ensureDepth(ImgBase **ppoImage, depth eDepth);
  
    /// ensures that an image has given depth and parameters \ingroup IMAGE
    ICLCore_API ImgBase *ensureCompatible(ImgBase **dst, depth d, const ImgParams &params);
  
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
    inline ImgBase *ensureCompatible(ImgBase **dst, depth d,const utils::Size& size,int channels, const utils::Rect &roi=utils::Rect::null)
       { return ensureCompatible(dst,d,ImgParams(size,channels,roi)); }
  
    /// ensures that an image has given depth, size, format and ROI \ingroup IMAGE
    inline ImgBase *ensureCompatible(ImgBase **dst, depth d,const utils::Size& size, format fmt, const utils::Rect &roi=utils::Rect::null)
       { return ensureCompatible(dst,d,ImgParams(size,fmt,roi)); }
    
    /// ensures that an image has given parameters  \ingroup IMAGE
    /** The given format must be compatible to the given channel count.
        <b>If not:</b> The format is set to "formatMatrix" and an exception is thrown.
    */
    ICLCore_API ImgBase *ensureCompatible(ImgBase **dst, depth d, const utils::Size &size, int channels, format fmt, const utils::Rect &roi = utils::Rect::null);
    
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
    ICLCore_API ImgBase *ensureCompatible(ImgBase **dst, const ImgBase *src);
  
    /// determines the count of channels, for each color format \ingroup GENERAL
    /** @param fmt source format which channel count should be returned
        @return channel count of format eFormat
    **/
    ICLCore_API int getChannelsOfFormat(format fmt);
  
    /// getDepth<T> returns to depth enum associated to type T \ingroup GENERAL
    template<class T> inline depth getDepth();
  
  
    /// puts a string representation of format into the given stream
    ICLCore_API std::ostream &operator<<(std::ostream &s, const format &f);
    
    /// puts a string representation of depth into the given stream
    ICLCore_API std::ostream &operator<<(std::ostream &s, const depth &d);
  
    /// puts a string representation of format into the given stream
    ICLCore_API std::istream &operator>>(std::istream &s, format &f);
    
    /// puts a string representation of depth into the given stream
    ICLCore_API std::istream &operator>>(std::istream &s, depth &d);
  
    
    /** \cond */ 
  #define ICL_INSTANTIATE_DEPTH(T)                                        \
    template<> inline depth getDepth<icl ## T>() { return depth ## T; }
  ICL_INSTANTIATE_ALL_DEPTHS  
  #undef ICL_INSTANTIATE_DEPTH
    /** \endcond  */
   
    /// return sizeof value for the given depth type \ingroup GENERAL
    ICLCore_API unsigned int getSizeOf(depth eDepth);
  
    /// moves data from source to destination array (no casting possible) \ingroup GENERAL
    template <class T>
    inline void copy(const T *src, const T *srcEnd, T *dst){
      //std::copy<T>(src,srcEnd,dst);
      memcpy(dst,src,(srcEnd-src)*sizeof(T));
    } 
  
  
  #ifdef HAVE_IPP
    /** \cond */ 
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
    /** \endcond */
  #endif
  
  
  
  
    /// moves value from source to destination array (with casting on demand) \ingroup GENERAL
    template <class srcT,class dstT>
    inline void convert(const srcT *poSrcStart,const srcT *poSrcEnd, dstT *poDst){
      std::transform(poSrcStart,poSrcEnd,poDst,utils::clipped_cast<srcT,dstT>);
    }
    
  #ifdef HAVE_IPP 
    /** \cond */ 
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
    /** \endcond */
  #elif defined __ICL_SSE2__
    /** \cond */

    /// from icl8u functions
    template<> ICLCore_API void convert<icl8u,icl32f>(const icl8u *poSrcStart,const icl8u *poSrcEnd, icl32f *poDst);

    /// from icl16s functions
    template<> ICLCore_API void convert<icl16s,icl32s>(const icl16s *poSrcStart,const icl16s *poSrcEnd, icl32s *poDst);
    template<> ICLCore_API void convert<icl16s,icl32f>(const icl16s *poSrcStart,const icl16s *poSrcEnd, icl32f *poDst);
    template<> ICLCore_API void convert<icl16s,icl64f>(const icl16s *poSrcStart,const icl16s *poSrcEnd, icl64f *poDst);
    
    // from icl32s functions
    template<> ICLCore_API void convert<icl32s,icl16s>(const icl32s *poSrcStart,const icl32s *poSrcEnd, icl16s *poDst);
    template<> ICLCore_API void convert<icl32s,icl32f>(const icl32s *poSrcStart,const icl32s *poSrcEnd, icl32f *poDst);
    template<> ICLCore_API void convert<icl32s,icl64f>(const icl32s *poSrcStart,const icl32s *poSrcEnd, icl64f *poDst);
  
    // from icl32f functions
    template <> ICLCore_API void convert<icl32f,icl8u>(const icl32f *poSrcStart, const icl32f *poSrcEnd, icl8u *poDst);
    template <> ICLCore_API void convert<icl32f,icl16s>(const icl32f *poSrcStart, const icl32f *poSrcEnd, icl16s *poDst);
    template <> ICLCore_API void convert<icl32f,icl32s>(const icl32f *poSrcStart, const icl32f *poSrcEnd, icl32s *poDst);
    template <> ICLCore_API void convert<icl32f,icl64f>(const icl32f *poSrcStart, const icl32f *poSrcEnd, icl64f *poDst);
  
    // from icl64f functions 
    template<> ICLCore_API void convert<icl64f,icl32f>(const icl64f *poSrcStart,const icl64f *poSrcEnd, icl32f *poDst);
    template <> ICLCore_API void convert<icl64f,icl32s>(const icl64f *poSrcStart,const icl64f *poSrcEnd, icl32s *poDst);

    /** \endcond */
  #endif 
  
   
    /// function, that calculates the mininum and the maximum value of three value \ingroup GENERAL
    /** @param a first input value 
        @param b second input value 
        @param c third input value 
        @param minVal return value for the minimum
        @param maxVal return value for the maximum
    **/
    template<class T>
    ICLCore_API inline void getMinAndMax(T a, T b, T c, T &minVal, T &maxVal){
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

    /// Computes the mean value of a ImgBase* ingroup MATH
    /** IPP-Optimized for icl32f and icl64f
        @param poImg input image
        @param iChannel channel index (-1 for all channels)
	@param roiOnly
        @return mean value of image or image channel (optionally: roi)
    */
    ICLCore_API std::vector<double> mean(const ImgBase *poImg, int iChannel = -1, bool roiOnly = false);

    /// Compute the variance value of an image a with given mean \ingroup MATH
    /** @param poImg input imge
        @param mean vector with channel means
        @param empiricMean if true, sum of square distances is devided by n-1 else by n
        @param iChannel channel index (-1 for all channels)
	@param roiOnly
        @return The variance value form the vector
    */
    ICLCore_API std::vector<double> variance(const ImgBase *poImg, const std::vector<double> &mean, bool empiricMean = true, int iChannel = -1, bool roiOnly = false);
    
    /// Compute the variance value of an image a \ingroup MATH
    /** @param poImg input imge
        @param iChannel channel index (-1 for all channels)
	@param roiOnly 
        @return The variance value form the vector
        */
    ICLCore_API std::vector<double> variance(const ImgBase *poImg, int iChannel = -1, bool roiOnly = false);

    /// Compute the std::deviation of an image
    /** @param poImage input image
        @param iChannel channel index (all channels if -1)
        @param roiOnly
        */
    ICLCore_API std::vector<double> stdDeviation(const ImgBase *poImage, int iChannel = -1, bool roiOnly = false);
    
    /// Compute the std::deviation of an image with given channel means
    /** @param poImage input image
	@param mean vector with channel means
	@param empiricMean if true, sum of square distances is devided by n-1 else by n
        @param iChannel channel index (all channels if -1)
	@param roiOnly
    */
    ICLCore_API std::vector<double> stdDeviation(const ImgBase *poImage, const std::vector<double> mean, bool empiricMean = true, int iChannel = -1, bool roiOnly = false);
  
    /// Calculates mean and standard deviation of given image simultanously
    /** @param image input image
        @param iChannel image channel if < 0 all channels are used
	@param roiOnly
        @return vector v of pairs p with p.first = mean and p.second = stdDev v[i] containing i-th channel's results
    */
    ICLCore_API std::vector< std::pair<double, double> > meanAndStdDev(const ImgBase *image, int iChannel = -1, bool roiOnly = false);
    
    
    /// computes the color histogramm of given image channel                               
    ICLCore_API std::vector<int> channelHisto(const ImgBase *image, int channel, int levels = 256, bool roiOnly = false);
    
    /// computes the color histogramm of given image
    ICLCore_API std::vector<std::vector<int> > hist(const ImgBase *image, int levels = 256, bool roiOnly = false);
  
  } // namespace core
} // namespace icl
