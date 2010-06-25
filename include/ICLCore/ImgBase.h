/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLCore/ImgBase.h                              **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter, Michael Götting, Robert Haschke,  **
**          Andre Justus                                           **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICLBASE_H
#define ICLBASE_H

#include <vector>
#include <ICLCore/CoreFunctions.h>
#include <ICLCore/ImgParams.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/Range.h>
    

namespace icl {
  
  /// ImgBase is the Image-Interface class that provides save access to underlying Img-template \ingroup IMAGE \ingroup TYPES
  /* {{{ ImgBase class documentation */

  /**
      \section SEC1 Class
      The ImgBase class provides access to the following basic image features:
      - image size 
      - channel count
      - depth:  currently one of the following:
        - depth8u
        - depth16s
        - depth32s
        - depth32f
        - depth64f
      - format: color-format associated with images channels 
        (see section "Img Color Formats")
      - raw data:  getDataPtr(int) returns image data form nth channel 
        as void pointer. The function is implemented in the 
        inherited classes Img<icl8u> and Img<icl32f>, 
        which also provide type-safe access functions, 
        e.g. getData (int).
      - time stamp: The time stamp of an images can be set using the
        setTime() and getTime() functions. All image sources should set
        the time stamp of the produced images.\n
        An images time stamp is associated with image images content (
        what scene is shown on the image). So it is not implicitly the images
        creation time, but it corresponds to the last time the images data content
        was set. E.g. a scaling operation would not change the images time stamp.
   
      \section SEC2 How to use the ImgBase class.
      As the ImgBase is an abstract class, no ImgBase objects can be instantiated
      It merely provides a common interface to methods provided by the 
      inherited classe Img<T>. It can be used in all functions the abstract
      about the underlying data type. E.g. consider a scaling operation on images:
      When regarding an image as a continuous 2D function, it is not necessary to know
      the images data type when scaling it. However the developer might say: "At 
      the latest if I want to implement some kind of interpolation I have to know the
      actual data type I'm working on".\n
      The ImgBase class implements exactly this kind of abstraction from the 
      underlying implementation. Although the implementation has indeed to tackle
      each of the different data types (E.g. floating point types vs. integer 
      types) in an appropriate way, it is possible to call such function directly on
      the abstract ImgBase object.\n
      Many other operations are also conceptually independent
      on the concrete pixel type, e.g. recombining or selecting channels 
      For these operations the ImgBase class provides abstract or implemented
      methods ensuring a common and type-independent interface.
      
      For example, to resize an image, one can easily write:
      \code
      void any_function(ImgBase *poBase){
          poBase->setSize(Size(640,480));
          ...
      }
      \endcode
      
      \section Examples
      The following example should explain how to work with ImgBase class.
  
      \code
      void special_function_8u(Img8u* poImage){...}
      void special_function_16s(Img16f* poImage){...}
      ...
      void special_function_64f(Img64f* poImage){...}
      
      void generic_function(ImgBase *poImage){
         switch(poImage->getDepth()){
            case depth8u:  special_function_8u(poImage->asImg<icl8u>());
            case depth16s: special_function_8u(poImage->asImg<icl16s>());
            ...
            case depth64f: special_function_(poImage->asImg<icl64f>());
        }
      }
      \endcode
      Template functions can be called in an analogous way:
      \code
      template<class T> void template_function(Img<T> *poImg){...}
      
      void generic_function(ImgBase *poImage){
         switch(poImage->getDepth()){
            case depth8u:  template_function(poImage->asImg<icl8u>());
            case depth16s: template_function(poImage->asImg<icl16s>());
            ...
            case depth64f: template_function(poImage->asImg<icl64f>());
        }
      }
      \endcode
  **/ 

  /* }}} */
  class ImgBase
    {   
      public:
      
      /* {{{ open */
    
      /// Destructor
      virtual ~ImgBase();
      
      /* }}} */ 
      
      /** @{ @name shallow copy */
      /* {{{ open */

      
      /** Create a shallow copy of an image with given 
          @param ppoDst destination image which is exploited if possible, or otherwise reallocated
          @param roi ROI of the new Image
          @param channelIndices indices to select from the source image. These channels are
                                shallow-copied into the destination image
          @param fmt format of the new image (the channel count that is associated with this format
                     must be equal to the channel count that is implicitely defined by the size of 
                     the vector channelIndices
          @param time new timestamp for the returned image
          @return shallow-copied image
      **/
      virtual ImgBase *shallowCopy(const Rect &roi, 
                                   const std::vector<int> &channelIndices,
                                   format fmt, 
                                   Time time=Time::null,
                                   ImgBase **ppoDst = NULL) = 0;
      
      /** Create a shallow copy of an image with given (const version)
          @see the above function
          @param roi ROI of the new Image
          @param channelIndices indices to select from the source image. These channels are
                                shallow-copied into the destination image (if size is null, all
                                channels are selected)
          @param fmt format of the new image (the channel count that is associated with this format
                     must be equal to the channel count that is implicitely defined by the size of 
                     the vector channelIndices
          @param time new timestamp for the returned image
          @return shallow-copied image
      **/
      const ImgBase *shallowCopy(const Rect &roi, 
                                 const std::vector<int> &channelIndices,
                                 format fmt, 
                                 Time time=Time::null) const{
        // casting constness away is safe, because we effectively return a const Img<Type>*
        return const_cast<ImgBase*>(this)->shallowCopy(roi,channelIndices,fmt,time,0);
      }
      

      /// Create a shallow copy of this image with a new format
      /** @param newFmt new format to choose. This must be compatible to the channel count 
                           of this image.
          @param ppoDst destination image (exploited as possible) 
          @return shallow copie with given format of NULL if an error occured 
      **/
      ImgBase *reinterpretChannels(format newFmt, ImgBase **ppoDst = NULL){
        return shallowCopy(getROI(),std::vector<int>(),newFmt,getTime(),ppoDst);
      }
      
      
      /// Create a shallow copy of this image with a new format (const version)
      /** @param newFmt new format to choose. This must be compatible to the channel count 
                           of this image. 
          @return shallow copie with given format of NULL if an error occured 
      **/
      const ImgBase *reinterpretChannels(format newFmt) const{
        // casting constness away is safe, because we effectively return a const Img<Type>*
        return const_cast<ImgBase*>(this)->shallowCopy(getROI(),std::vector<int>(),newFmt,getTime());
      }
      /// Create a shallow copy of the image
      /** It exploits the given destination image if possible,
          i.e. if the pixel depth matches. Else this image is released
          and a new one is created. Optionally a second argument can be
          specified to get a new image with the given ROI.
          @param ppoDst pointer to the destination image pointer If ppoDst is NULL,
                        a new image is created, if ppoDst points to NULL, a new 
                        image is created at *ppoDst;
          @param roi new ROI of the new image. If Rect::null, the source images roi
                     is used.
          @return shallow copy of this image
      **/
      ImgBase* shallowCopy(const Rect &roi, ImgBase** ppoDst = NULL){
        return shallowCopy(roi,std::vector<int>(),getFormat(),getTime(),ppoDst);
      }
      ImgBase* shallowCopy(ImgBase** ppoDst = NULL){
        return shallowCopy(getROI(),std::vector<int>(),getFormat(),getTime(),ppoDst);
      }

      /// Create a shallow copy of a const source image
      /** In contrast to the not const function shallowCopy, the const one does not provide
          to specify a destination image pointer, because this must neither be const nor not const.
          If it would be const, it would not be possible to adapt it to correct parameters, 
          otherwise it would violate the const concept as it could be used to change the const
          result.\n 
          This function can only be used to get const copy of a source image with a special ROI.
          @param roi ROI of the returned image (Rect::null is not allowed!)
          @return shallow copy of this image with specified ROI
      */
      const ImgBase* shallowCopy(const Rect& roi) const {
         // casting constness away is safe, because we effectively return a const Img<Type>*
         return const_cast<ImgBase*>(this)->shallowCopy(roi,0);
      }
     

      /// Create a shallow copy of selected channels of an image
      /** This function can be used if only one or some channels of a given const 
          image should be used in further processing steps. It helps to avoid the 
          necessity of "deepCopy" calls there.
          @param channelIndices vector containing channel indices to copy
          @param ppoDst destination image (if Null, a new one is created)
          @return image containing only the selected channels (as shallow copies)
                        format of that image becomes formatMatrix
          @see shallowCopy
      */
      ImgBase* selectChannels (const std::vector<int>& channelIndices, ImgBase** ppoDst=0){
        return shallowCopy(getROI(),channelIndices,formatMatrix,getTime(),ppoDst);
      }

      /// Create a shallow copy of a single image channel of an image
      /** This function is a shortcut to use 
          icl::ImgBase::selectChannels(const std::vector<int>&,icl::ImgBase**) to 
          select a single channel from an image
          @param channelIndex index of the channel to select (if invalid, NULL is returned)
          @param ppoDst destination image 
          @return image containing only the selected channel
      **/
      ImgBase* selectChannel(int channelIndex, ImgBase **ppoDst=0){
        ICLASSERT_RETURN_VAL(validChannel(channelIndex), 0);
        std::vector<int> v(1); v[0]= channelIndex; 
        return selectChannels(v,ppoDst);
      }
      /// Create a shallow copy of selected channels of a const image.
      /** @param channelIndices vector containing channel indices to copy
          @return const image containing only the selected channels
      */
      const ImgBase* selectChannels (const std::vector<int>& channelIndices) const {
         // casting constness away is safe, because we effectively return a const Img<Type>*
         return const_cast<ImgBase*>(this)->selectChannels(channelIndices, 0);
      }
      
      /// Create a shallow copy of a single image channel of a const image
      /** This function is a shortcut to use 
          icl::ImgBase::selectChannels(const std::vector<int>&)const to 
          select a single channel from a const image image
          @param channelIndex index of the channel to select (if invalid, NULL is returned)
          @return const image containing only the selected channel
      **/
      const ImgBase *selectChannel(int channelIndex) const{
        ICLASSERT_RETURN_VAL(validChannel(channelIndex), 0);
        std::vector<int> v(1); v[0]= channelIndex; return selectChannels(v);
      }

      
      /* }}} */
      
      /** @{ @name deep copy and depth conversion */
      /* {{{ open */
      /// Create a deep copy of a given image
      /** An optional destination image can be given via ppoDst. If ppoDst is NULL, a new image
          created and returned. If ppoDst points to NULL, the new image is created at *ppoDst.
          Otherwise, the given destination image (*ppoDst) is adapted to this images params
          including its depth. If the destination images depth differs from this images depth, 
          (*ppoDst) is first released and then created new on the heap
          @param ppoDst optionally given destination image
          @return deep copied image
      */
      virtual ImgBase *deepCopy(ImgBase **ppoDst=0) const=0;
      
      /// Create a deep copy of an images ROI
      /** This function creates copies this images ROI into an optional given 
          destination image. If ppoDst is NULL, a new image is created. If it points to 
          NULL, a new image is created at *ppoDst. Otherwise the destination image is
          adapted in size, channels and depth to this image (the size is set to this
          images ROI size). The copy operation is performed line-wise using <em>memcpy</em>,
          what makes deepCopyROI very fast.
          @param ppoDst optionally given destination image
          @return image containing a deep copy of the source images ROI
      */
      virtual ImgBase *deepCopyROI(ImgBase **ppoDst=0) const=0;

      /// returns an Img<T> instance of this image (type-conversion or deep copy)
      /** If the requested type differs from the actual image type, then a type
          conversion is performed to transfer the image data to poDst. Else
          deepCopy is called, to transfer the image data. If poDst is NULL, it
          is created with identical parameters, except for the images depth, which
          is given by the template parameter T.
          (For developers: The convert function builds the base function for
          other higher level functions like deepCopy. Internally it calls the
          icl namespace function deepCopyChannel, which decides if data has to
          be copied or converted.)
          @param poDst destination image. If NULL, then a deep copy of the current
                       image is returned
          @return converted image
          @see deepCopy
      */      
      template<class T>
      Img<T> *convert(Img<T> *poDst=NULL) const;

      /// returns a converted (or deep copied) instance of this image
      /** This function can be called using an explicit destination depth.
          The function switches the depth parameter and calls the associated
          convert<T> template function
          @param d new images depth
          @return converted image
      */
      ImgBase *convert(depth d) const;

      /// converts image data into the given destination image
      /** @param poDst destination image (exploited if not NULL else a deep copy of this is returned)
          @return converted image
      **/
      ImgBase *convert(ImgBase *poDst) const;
      
      /// returns a converted (or deep copied) instance of this images ROI
      /** This function behaves essentially like the above functions, except it
          is applied on the source image ROI only.
          @param poDst optionally given destination image pointer.
          @return converted image, containing the source images ROI
      */
      template<class T>
      Img<T> *convertROI(Img<T> *poDst=NULL) const;

      /// returns a converted (or deep copied) instance of this images ROI
      /** This function behaves essentially like the above functions, except it
          is applied on the source image ROI only.
          @param d new images depth 
          @return converted image, containing the source images ROI
      */
      ImgBase *convertROI(depth d) const;
      
      
      /// converts this images ROI into a given destination image
      /** The destination image is exploited if not NULL, else a deep copy of this
          is created and returned. 
          @param poDst destination image (its depth is hold, other parameters are adapted)
          @return converted image (poDst if it was not NULL) that was adapted to this images 
                            ROI size, channels, format and Time
      **/
      ImgBase *convertROI(ImgBase *poDst) const;
      
      /// converts this images ROI into the destination images ROI
      /** it exploits the destination image if it is not NULL. Otherwise a deep copy of this
          image is created and returned. This images ROI size must be equal to poDst ROISize,
          otherwise NULL is returned, and an assertion message is written to std::out.
          @param poDst destination image (adated in channel count, and format).
          @return conveted poDst 
      **/
      // DELETE template<class T>
      // DELETE Img<T> *convertROIToROI(Img<T> *poDst) const;
      
      /// converts this images ROI into the destination image ROI (ImgBase version)
      /** This function behaves like the above funtion 
          @param poDst destination image
          @return converted image (poDst)
      **/
      // DELETE ImgBase *convertROIToROI(ImgBase *poDst) const;
      
      
      /// @}
      /* }}} */ 

      /** @{ @name scaledCopy */
      /* {{{ open */
      
      /// Create a scaled copy with given size of an image
      /** @param newSize size of the new image
          @param eScaleMode interpolation method to use when scaling the image
          @return scaled image
      */
      virtual ImgBase *scaledCopy(const Size &newSize, scalemode eScaleMode=interpolateNN) const = 0;

      /// Create a scaled copy into a given destination image
      /** If the given destination pointer ppoDst is NULL, a deep copy of this image
          is returned. If ppoDst points to NULL, a new a deep copy of this image is
          created at *ppoDst. Otherwise, the destination image is only adapted in its
          depth to this image; its size is hold.
          @param ppoDst optionally given destination image pointer
          @param eScaleMode interpolation method to use when scaling the image
          @return scaled image
      */
      virtual ImgBase *scaledCopy(ImgBase **ppoDst=0, scalemode eScaleMode=interpolateNN) const = 0;

      /// Create a scaled copy with given size of an images ROI
      /** This function behaves identically to the scaledCopy function above, except it
          is applied on the source images ROI only.
          @param newSize size of the new image
          @param eScaleMode interpolation method to use when scaling the image
          @return image containing a scaled instance of the source images ROI
      */
      virtual ImgBase *scaledCopyROI(const Size &newSize, scalemode eScaleMode=interpolateNN) const = 0;

      /// Create a scaled copy of an images ROI with optionally given destination image
      /** This function behaves identically to the scaledCopy function above, except it
          is applied on the source images ROI only.
          @param ppoDst optionally given destination image pointer
          @param eScaleMode interpolation method to use when scaling the image
          @return image containing a scaled instance of the source images ROI
      */
      virtual ImgBase *scaledCopyROI(ImgBase **ppoDst=0, scalemode eScaleMode=interpolateNN) const = 0;
      
      /// @}
      /* }}} */
      
      /** @{ @name asImg<T> cast templates */
      /* {{{ open */ 

      /// dynamically casts this image to one of its Img<T> subclasses 
      /** This function performs an emulated dynamic_cast on this image and
          returns a Img<T>*. If this image can not be casted to the the template
          type that is specified by the template parameter T, a NULL-pointer is
          returned. To avoid an expensive RTTI-runtime check using dynamic_cast,
          this images depth is compared the the depth associated the the template
          parameter T.
          @return Img<T>* instance of this image
      **/
      template <class T>
      Img<T> *asImg() {
        ICLASSERT_RETURN_VAL( icl::getDepth<T>() == getDepth(), 0);
        return reinterpret_cast<Img<T>*>(this);
      }
      
      /// dynamically casts this image to one of its Img<T> subclasses (const version)
      /** const version of the above function
          @return const Img<T>* instance of this image
      **/
      template <class T>
      const Img<T> *asImg() const {
        ICLASSERT_RETURN_VAL( icl::getDepth<T>() == getDepth(), 0);
        return reinterpret_cast<const Img<T>*>(this);
      }

      /// @}
    
      /* }}} */

      /** @{ @name getter  (without ROI handling) */
      /* {{{ open */

      /// returns all params in terms of a const ImgParams reference
      /** This enables the programmer to write
          <pre>
          imageA.setParams(imageB.getParams());
          </pre>
      */
      const ImgParams &getParams() const{ return m_oParams; }
      /// returns the size of the images

      // returns the images size
      const Size& getSize() const { return m_oParams.getSize(); }

      /// returns the images width
      int getWidth() const { return m_oParams.getWidth(); }

      /// returns the images height
      int getHeight() const { return m_oParams.getHeight(); }      

      /// returns the pixel count of each channel
      int getDim() const { return m_oParams.getDim(); }

      /// returns the channel count of the image
      int getChannels() const { return m_oParams.getChannels(); }

      /// returns the depth (depth8u or depth32f)
      depth getDepth() const { return m_eDepth; }

      /// returns the current (color)-format of this image
      format getFormat() const { return m_oParams.getFormat(); }

      /// returns the timestamp of the image
      Time getTime() const { return m_timestamp; }

      /// returns the length of an image line in bytes (width*sizeof(Type))
      /** This information is compulsory for calling any IPP function.
          @return getWidth()*sizeof(Type) in the underlying Img template **/
      virtual int getLineStep() const = 0;

     
      
      /// @}

      /* }}} */
      
      /** @{ @name ROI handling */
      /* {{{ open */

      /// returns the images ROI rectangle
      const Rect &getROI() const{ return m_oParams.getROI(); }
      
      /// copies the current ROI into the given offset and size references
      void getROI(Point& offset, Size& size) const { m_oParams.getROI(offset,size); }

      /// returns the images ROI offset (upper left corner)
      Point getROIOffset() const{ return m_oParams.getROIOffset(); }

      /// returns the images ROI size
      Size getROISize() const{ return m_oParams.getROISize(); }

      /// returns the images ROI width
      int getROIWidth() const{ return m_oParams.getROIWidth(); }

      /// returns the images ROI height
      int getROIHeight() const{ return m_oParams.getROIHeight(); }
      
      /// returns the images ROI XOffset
      int getROIXOffset() const{ return m_oParams.getROIXOffset(); }

      /// returns the images ROI YOffset
      int getROIYOffset() const{ return m_oParams.getROIYOffset(); }
      
      /// returns the image rect (0,0,width, height)
      Rect getImageRect() const { return Rect(Point::null,getSize()); }
      
      /// sets the image ROI offset to the given value
      void setROIOffset(const Point &offset) { m_oParams.setROIOffset(offset); }
      
      /// sets the image ROI size to the given value
      void setROISize(const Size &size) { m_oParams.setROISize(size); }
      
      /// set both image ROI offset and size
      void setROI(const Point &offset, const Size &size){ m_oParams.setROI(offset,size); }
      
      /// sets the image ROI to the given rectangle
      void setROI(const Rect &roi) { m_oParams.setROI(roi); }

      /// checks, eventually adapts and finally sets the image ROI size
      /** @see ImgParams*/
      void setROIOffsetAdaptive(const Point &offset) { m_oParams.setROIOffsetAdaptive(offset); }
      
      /// checks, eventually adapts and finally sets the image ROI size
      /** @see ImgParams*/
      void setROISizeAdaptive(const Size &size){ m_oParams.setROISizeAdaptive(size); }

      /// checks, eventually adapts and finally sets the image ROI size
      /** @see ImgParams*/
      void setROIAdaptive(const Rect &roi) { m_oParams.setROIAdaptive(roi); }
     
      /// returns ROISize == ImageSize
      int hasFullROI() const { return m_oParams.hasFullROI(); }
      
      /// resets the image ROI to the whole image size with offset (0,0)
      void setFullROI() { m_oParams.setFullROI(); }
      
      /// @}
      /* }}} */
      
      /** @{ @name border functions */
      /* {{{ open */
      
      /// extrudes ROI borders through non-ROI borders
      /** This function can be used fill all image border pixles 
          (pixels outside the current ROI with the value of the closest
          ROI-pixel
      */
      virtual void fillBorder(bool setFullROI=true)=0;
      
      /// fills all non-ROI pixels with a given value
      virtual void fillBorder(icl64f val, bool setFullROI=true)=0;

      /// fills all non-ROI pixels with a given value
      /** here, for each channel a given value is used, so vals.size()
          must be at least this->getChannels()
      */
      virtual void fillBorder(const std::vector<icl64f> &vals, bool setFullROI=true)=0;


      /// copies images non-border pixels from source image.
      /** The source image must provided pixel values for each non border
          pixel of this image. So the source images size must be at least 
          (X+1)x(Y+1) where (X,Y) is the lower right non-border pixel
          of this image.*/
      virtual void fillBorder(const ImgBase *src, bool setFullROI=true)=0;

      /// @}
      /* }}} */


      /** @{ @name data access */
      /* {{{ open */
      
      /// returns a pointer to first data element of a given channel
      /** @see Img*/
      virtual const void* getDataPtr(int iChannel) const = 0;

      /// returns a pointer to first data element of a given channel
      /** @see Img*/
      virtual void* getDataPtr(int iChannel) = 0;
    
      /// @} 
      /* }}} */
      
      /** @{ @name channel management */
      /* {{{ open */

      /// Makes the image channels independent from other images.
      /** @param iIndex index of the channel, that should be detached.
                        (If iIndex is an legal channel index only the 
                        corresponding channel will be detached. If 
                        iIndex is -1 (default) all channels are detached
      **/
      virtual void detach(int iIndex = -1)=0;

      /// Removes a specified channel.
      /** If a non-matrix format image looses a channel,
          the new channel count will not match to the channel count,
          that is associated with the current format. In this case, a
          warning is written to std::out, and the format will be set to 
          formatMatrix implicitly. To avoid this warning the programmer 
          has to change the format explicitly before to formatMatrix.
          @param iChannel Index of channel to remove
      **/
      virtual void removeChannel(int iChannel)=0;
      
      /// Swap channel A and B
      /** The channel swap operation is shallow; only the channel
          pointers are swapped.
          @param iIndexA Index of channel A;
          @param iIndexB Index of channel B
      **/
      virtual void swapChannels(int iIndexA, int iIndexB)=0;

      
      /// sets all image parameters in order channels,size,format,roi
      void setParams(const ImgParams &params);

      /// sets the channel count to a new value
      /** This function works only on demand, that means, that
          channels will only be created/deleted, if the new 
          channel count differs from the current. If the current
          image has a non-matrix format, then the new channel count
          must match to the channel count associated with this format.
          If not, a warning is written to std::out, and the format is
          set to formatMatrix implicitly. To avoid this warning, the 
          image format must be set to formatMatrix explicitly before
          calling setChannels
          @param iNewNumChannels new channel count
      **/
      virtual void setChannels(int iNewNumChannels)=0;

      /// resizes the image to new size (image data is lost!) 
      /** operation is performed on demand - if the image
          has already the given size, then
          nothing is done at all. For resizing
          operation with scaling of the image data use scale.
          <b>Note:</b> The ROI of the image is set to the hole
          image using delROI(), notwithstanding if a resize
          operation was performed or not.
          @param s new image size  (if x or y is < 0, the 
                   original width/height is used)
          @see scale
      **/
      virtual void setSize(const Size &s)=0;

      /// sets the format associated with channels of the image
      /**
          The channel count of the image is set to the channel count
          associated with the set format, if they differ.
          E.g an image with one channel will have 3 channels after
          a setFormat(formatRGB) - call.
          @param fmt new format value
          @see getChannelsOfFormat
      **/
      void setFormat(format fmt);

      /// sets the timestamp of the image
      void setTime(const Time time) { m_timestamp = time; }
      /// sets timestamp of the image to the current time
      void setTime() { m_timestamp = Time::now(); }
      
      /// @}

      /* }}} */
                    
      /** @{ @name min and max element */
      /* {{{ open */

    /// Returns max pixel value of channel iChannel within ROI
    /** @param iChannel Index of channel
        @param coords (optinal) if not null, the pixel position of the max is 
               written into this argument
    **/
    icl64f getMax(int iChannel, Point *coords=0) const;
  
    /// Returns min pixel value of channel iChannel within ROI
    /** @param iChannel Index of channel
        @param coords (optinal) if not null, the pixel position of the min is 
               written into this argument
        **/
    icl64f getMin(int iChannel, Point *coords=0) const;
  

    /// return maximal pixel value over all channels (restricted to ROI)
    icl64f getMin() const;

    /// return minimal pixel value over all channels (restricted to ROI)
    icl64f getMax() const;

    /// Returns min and max pixel values of channel iChannel within ROI
    /** @param iChannel Index of channel
        @param minCoords (optinal) if not null, the pixel position of the min is 
               written into this argument
        @param maxCoords (optinal) if not null, the pixel position of the max is 
               written into this argument
        @return channel range in terms of a Range<icl64f> struct
    **/
    const Range<icl64f> getMinMax(int iChannel, Point *minCoords=0, Point *maxCoords=0) const;

    /// Returns min and max pixel values of all channels within ROI
    /** @return image range in terms of a Range<icl64f> struct
    **/
    const Range<icl64f> getMinMax() const;

                    
    /// @}
    /* }}} */
    
      /** @{ @name in-place image adaption */
      /* {{{ open */
      
       /// performs an in-place resize operation on the image (IPP-OPTIMIZED)
      /** The image size is adapted on demand to the given size, and the image
          data is scaled. This function is SLOW in comparison to the scaledCopy
          function that is also provided in this class, as an additional scaling
          buffer is allocated and released at runtime.
          @param s new size of this image
          @param eScaleMode interpolation method to use for the scaling operation 
      **/
      virtual void scale(const Size& s, scalemode eScaleMode=interpolateNN)=0;


      /// performs an in-place mirror operation
      /** This function is an in-place version of the flippedCopy function, that
          is also provided in this class. Its performance is comparable to the
          out-place function 
          @param eAxis axis for the mirror operations
          @param bOnlyROI if set, only the ROI of this image is mirrored, else
                          the whole image is mirrored. 
      **/
      virtual void mirror(axis eAxis, bool bOnlyROI=false)=0;
      
      /// Sets the ROI pixels of one or all channels to a specified value
      /** @param iChannel Channel to fill with zero (default: -1 = all channels)
          @param val destination value (default: 0)
          @param bROIOnly if set false, the whole image is set to val
      **/
      void clear(int iChannel = -1, icl64f val=0, bool bROIOnly=true);

      /// Normalize the channel min/ max range to the new min, max range.
      /** The min/ max range from the source channels are automatically detected,
          <b>separately</b> for each channel.
          @param dstRange range of all channels after the operation
      **/
      void normalizeAllChannels(const Range<icl64f> &dstRange);

      /// Normalize the channel from a given min/max range to the new range 
      /** @param iChannel channel index
          @param srcRange assumption of the current range of the channel
          @param dstRange range of the channel after the operation
       **/
      void normalizeChannel(int iChannel,const Range<icl64f> &srcRange, const Range<icl64f> &dstRange);


      /// Normalize the channel from a given min/max range to the new range 
      /** The min/ max range from the source channel is automatically detected,
          separately for this channel.  (Internally: this function calls normalizeChannel
          with srcRage = this->getMinMax(iChannel) )
          @param iChannel channel index
          @param dstRange range of the channel after the operation
      **/
      void normalizeChannel(int iChannel, const Range<icl64f> &dstRange);

      /// Normalize the image from a given min/max range to the new range 
      /** @param srcRange assumption of the current image range 
          @param dstRange range of the image after the operation
      **/
      void normalizeImg(const Range<icl64f> &srcRange,const Range<icl64f> &dstRange);
  
      /// Normalize the image from a min/max range to the new range 
      /** The min/ max range from the image is automatically detected, combined 
          over all image channels. (Internally: this function calls normalizeImg
          with srcRage = this->getMinMax() )
          @param dstRange range of the image after the operation
      **/
      void normalizeImg(const Range<icl64f> &dstRange);

      /// @}
      /* }}} */
      
      /** @{ @name utility functions */
      /* {{{ open */

      /// prints the image to std-out
      /** @param sTitle optional title, that can be printed before
          printing the image parameters to identify the message.
      **/
      void print(const std::string sTitle="image") const;

      /// validate the given channel index
      bool validChannel(const int iChannel) const {
         return iChannel >= 0 && iChannel < getChannels();
      }

      /// returns if two images have same size, and channel count
      /** @param s size to test
          @param nChannels channel count to test
      **/
      bool isEqual(const Size &s, int nChannels) const {
          FUNCTION_LOG("isEqual("<<s.width<<","<< s.height << ","<< nChannels << ")");
          return (getSize() == s) && (getChannels() == nChannels);
        }
      
      /// checks if the image has the given parameters
      bool isEqual(const ImgParams &params){
        FUNCTION_LOG("");
        return m_oParams == params;
      }

      /// checks if the image has given params and depth
      bool isEqual(const ImgParams &params, depth d){
        FUNCTION_LOG("");
        return m_oParams == params && getDepth() == d;
      } 
      
      /// checks if the image has given params and depth as another image
      bool isEqual(const ImgBase *otherImage){
        FUNCTION_LOG("");
        ICLASSERT_RETURN_VAL(otherImage,false);
        return getParams() == otherImage->getParams() && getDepth() == otherImage->getDepth();
      }

      /// returns whether image data is currently shared
      /** This function does only return true, if all channel pointers
          have reference count 1 -- i.e. all channels are currently
          not shared with another image */
      virtual bool isIndependent() const=0;
      /// @}
      /* }}} */

      protected:

      /* {{{ Constructor  */

      /// Creates an ImgBase object with specified image parameters 
      ImgBase(depth d, const ImgParams& params);

      /* }}} */

      /* {{{ data */

      /// all image params
      /** the params class consists of 
          - image size
          - number of image channels
          - image format
          - image ROI
      */
      ImgParams m_oParams;

      /// depth of the image (depth8 for icl8u/depth32 for icl32f)
      depth m_eDepth;

      /// timestamp of the image
      Time m_timestamp;

      /* }}} */
    };

  /// puts a string representation of the image into given steam
  std::ostream &operator<<(std::ostream &s, const ImgBase &image);

 
}

#endif 
