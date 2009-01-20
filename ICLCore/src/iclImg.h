#ifndef ICL_IMG_H
#define ICL_IMG_H

#include <iclImgBase.h>
#include <iclImgIterator.h>
#include <iclSmartPtr.h>
#include <iclException.h>
#include <cmath>
#include <algorithm>


namespace icl {
  /// The Img class implements the ImgBase Image interface with type specific functionalities \ingroup IMAGE \ingroup TYPES
  template<class Type>
  class Img : public ImgBase
  {
       
    /* this is declare as fried, because it accesses the private append function */
    template<class ImgType> friend 
    const ImgType* combineImages (const std::vector<const ImgType*>& vec, ImgBase** ppoDst);
   
    /// Private assign operator (internally used)
    /** This must be kept private! Because the assign operator could otherwise be exploited
        to violate the Img's const concept
    **/
    Img<Type>& shallowCopy(const Img<Type>& tSource);
    
    /// private append function for a specified image channel
    /** This must be kept private! Because it could otherwise be exploited
        to violate the Img's const concept
    **/
    void append(const Img<Type> *src, int iChannel=-1);

    /// private append function for a specified image channel
    /** This must be kept private! Because it could otherwise be exploited
        to violate the Img's const concept
    **/
    void append(const Img<Type> *src, const std::vector<int>& vChannels);

    protected:
	 
    /** @{ @name data **/ 
    /* {{{ open */

    /// internally used storage for the image channels
    std::vector<SmartPtr<Type> > m_vecChannels;
    /// @}

    /* }}} */

    /** @{ @name auxiliary functions **/
    /* {{{ open */

    /// Internally creates a new deep copy of a specified Type*
    /** if the give Type* ptDataToCopy is not NULL, the data addressed from it, 
        is copied deeply into the new created data pointer
        **/
    SmartPtr<Type> createChannel(Type *ptDataToCopy=0) const;

    /// returns the start index for a channel loop
    /** In some functions to cases must be regarded:
        - if given channel index is -1, then it has to be iterated over all
          image channels
        - else only the given image channel has to be touched
      
        To avoid code doublication, one can use the following for-loop
        <pre>
        void foo(int iChannel){
           for(int i = iIndex < 0 ? 0 : iIndex, iEnd = iIndex < 0 ? m_iChannels : iIndex+1; i < iEnd; i++)   { 
                 // do something
           }
        }
        </pre>
        When using the get<Start|End>Index functions the loop becomes much more
        readable:
        <pre>
        void foo(int iChannel){
           for(int i=getStartIndex(iIndex), iEnd=getEndIndex(iIndex); i<iEnd ;i++){
             // do something
           }
        }
        </pre>
        @param iIndex channel index
        @return start index for for-loops
        @see getEndIndex
        **/
    int getStartIndex(int iIndex) const { return iIndex < 0 ? 0 : iIndex; }

    /// returns the end index for a channel loop
    /** this function behaves essentially like the above function 
        @param iIndex channel index
        @return end index for for-loops
        @see getStartIndex
        */
    int getEndIndex(int iIndex) const { return iIndex < 0 ? getChannels() : iIndex+1; }

    /// @}

    /* }}} */
          
    /** @{ @name basic image manipulation */
    /* {{{ open */

    /// Scales pixel values from given min/max values to new min/max values (for internal use)
    /** Values exceeding the given range are set to the new min/max values.
        For an automatic scaling use the results of  min(),max() as as arguments.
        (Defining a range allows to compare different images.)
        @param iChannel channel index (if set to -1, then operation is 
        @param srcRange assumption of the images range
        @param dstRange image range after the operation

        performed on all channels)
        **/
    void normalize(int iChannel, const Range<Type> &srcRange, const Range<Type> &dstRange);

    /// in-place mirror operation on the given image rect (for internal use)
    /** @param eAxis axis for the mirror operation
        @param iChannel channel index to work on
        @param oOffset image rects offset to use
        @param oSize image rects size to use
    **/
    void mirror(axis eAxis, int iChannel,const Point& oOffset, const Size& oSize);
    /// @}
    /* }}} */

    public:

    /// null sized and null channel image
    static const Img<Type> null;
    
    /* {{{ open */
    /// creates a new image specified by the given param struct
    /** @param params initializing image parameters, if null, then a 
        null image is created  
        */
    Img(const ImgParams &params = ImgParams::null);
  
    /// Creates an image with specified number of channels and size.    
    /** the format of the image will be set to "formatMatrix"
        @param size image size
        @param channels Number of Channels 
        **/
    Img(const Size &size, int channels);
 
    /// Creates an image with specified size, number of channels and format
    /** @param s size of the new image
        @param fmt (color)-format of the image
        **/
    Img(const Size &s, format fmt);
 
    /// Crates an image with given size, channel count and format
    /** Note: channel count and format depend on each other, so if
        the given channel count and the given format are not compatible,
        an exception is thrown
        @param s size of the image
        @param channels channel count of the image (must be compatible to fmt)
        @param fmt format of the image (must be compatible to channels)
        */
    Img(const Size &s, int channels, format fmt);
  
    /// Creates an image with specified size and format, using shared data pointers as channel data
    /** The channel count is set to the channel count that is associated with given the format
        @param size new image size
        @param format (color)-format of the image
        @param vptData holds a pointer to channel data pointers. pptData must contain 
        enough Type-pointers for the given format. The data must not be 
        deleted during the "lifetime" of the Img. Call detach after the 
        constructor call, to induce the Img to allocate own memory for 
        the image data.
        **/
    Img(const Size &size, format format, const std::vector<Type*>& vptData);
  
    /// Creates an image with specified size and channel count, using shared data pointers as channel data
    /** the format is set to formatMatrix
        @param size new image size
        @param channels channel count of the image (format is set to "formatMatrix")
        @param vptData holds a pointer to channel data pointers. pptData must contain 
        enough Type-pointers for the given format. The data must not be 
        deleted during the "lifetime" of the Img. Call detach after the 
        constructor call, to induce the Img to allocate own memory for 
        the image data.
        **/
    Img(const Size &size, int channels, const std::vector<Type*>& vptData);

    /// Crates an image with given size, channel count and format
    /** Note: channel count and format depend on each other, so if
        the given channel count and the given format are not compatible,
        an exception is thrown
        @param size size of the image
        @param channels channel count of the image (must be compatible to fmt)
        @param fmt format of the image (must be compatible to channels)
        @param vptData array of data pointers, which are used as shared 
        pointers. Ensure, that these pointers are persistent
        during the lifetime of the image, or call detach, to
        make the image allocate it own memory for the data
        */
    Img(const Size &size, int channels, format fmt, const std::vector<Type*>& vptData);

    /// Copy constructor WARNING: Violates const concept
    /** Creates a flat copy of the source image. The new image will contain a
        flat copy of all channels of the source image. This constructor is only
        applicable to <b>non-const</b> Img<Type> references.
        <b>Note:</b> this implicit shallow copy can be exploited to
        violate ICL's const concept:
        \code
        void func(const Img8u &a){
          Img8u b = a;
          // b is now unconst and therewith the data of
          // a can b chaned
        }
        \endcode   
  
        @param tSrc non-const reference of source instance
        **/
    Img(const Img<Type>& tSrc);
    
    
    
    /// Destructor
    ~Img();

    /// null check : null images have 0-Channels and null-size
    bool isNull() const{
      return getSize()==Size::null && getChannels() == 0;
    }
    

    /* }}} */
  
    /** @{ @name operators */
    /* {{{ open */
    
    /// implicit cast to it's own reference (?)
    operator Img<Type>&(){
      return *this;
    }

    /// implicit cast to it's own reference (?) (const)
    operator const Img<Type>&() const {
      return *this;
    }

    /// Assign operator (flat copy of channels) WARNING: Violates const concept
    /** Both images will share their channel data. 
        Use deepCopy() to obtain a copy of an image which is not attached to the 
        source image.     
        <b>Note:</b> this implicit shallow copy can be exploited to
        violate ICL's const concept:
        \code
        void func(const Img8u &a){
          Img8u b = a;
          // b is now unconst and therewith the data of
          // a can b chaned
        }
        \endcode        
        @param tSource Reference to source object. 
        **/
    Img<Type>& operator=(const Img<Type>& tSource) {
      // call private const-version
      return this->shallowCopy (static_cast<const Img<Type>&>(tSource));
    }
    /*
        #ifdef WIN32  
        Img<Type>& operator=(const Img<Type>& tSource) {
        // call private const-version
        return this->shallowCopy (static_cast<const Img<Type>&>(tSource));
        }
        #endif
        
        #ifdef GCC_VER_423
        Img<Type>& operator=(const Img<Type>& tSource) {
        // call private const-version
        return this->shallowCopy (static_cast<const Img<Type>&>(tSource));
        }
        #endif
    */

    /// pixel access operator
    /** This operator may be used, to access the pixel data of the image
        e.g. copy of image data:
        <pre>
        Img8u oA(Size(320,240),1),oB(Size(320,240),1);
        for(int x=0;x<320;x++){
        for(int y=0;y<240;y++){
        oB(x,y,0)=oA(x,y,0);
        }
        }
        </pre>
        <h3>Efficiency</h3>
        Although the ()-operator is compiled inline, and optimized,
        it is very slow, as it has to select a channel internally 
        (array access) followed by the data access of the selected 
        channel (return array[x+w*y]). A measurement with a "-O3" 
        binary brought the result that pixel access is up to 10 times 
        faster when working directly with a channel data pointer. 
        Nevertheless, the ()-operator is provided in the Img-class, 
        as it offers a very intuitive access to the pixel data. 
        <b>Note:</b> The also provided ImgIterator provides an 
        additional ROI handling mechanism and is more than 5 times 
        faster. @see ImgIterator @see getIterator() @see beginROI()
  
        @param iX X-Position of the referenced pixel
        @param iY Y-Position of the referenced pixel
        @param iChannel channel index
        **/
    Type& operator()(int iX, int iY, int iChannel) {
      return const_cast<Type&>(static_cast<const Img<Type>*>(this)->operator()(iX,iY,iChannel)); 
    }
    const Type& operator()(int iX, int iY, int iChannel) const { 
      return getData(iChannel)[iX+getWidth()*iY]; 
    }

    /// sub-pixel access using nearest neighbor interpolation
    float subPixelNN(float fX, float fY, int iChannel) const {
      return (*this)((int)fX, (int)fY, iChannel);
    }
    /// sub-pixel access using linear interpolation
    float subPixelLIN(float fX, float fY, int iChannel) const;
  
    /// sub-pixel access using region average interpolation (not supported/possible)
    float subPixelRA(float fX, float fY, int iChannel) const;

    /// sub-pixel access operator, uses given interpolation method
    Type operator()(float fX, float fY, int iChannel, scalemode eMode) const;
  
    /// @}

    /* }}} */
  
    /** @{ @name shallow/deepCopy  functions */
    /* {{{ open  */
                  
    virtual Img<Type> *shallowCopy(const Rect &roi, 
                                   const std::vector<int> &channelIndices,
                                   format fmt,
                                   Time time =Time::null,
                                   ImgBase **ppoDst = NULL);

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
      const Img<Type> *shallowCopy(const Rect &roi, 
                                   const std::vector<int> &channelIndices,
                                   format fmt, 
                                   Time time=Time::null) const{
        // casting constness away is safe, because we effectively return a const Img<Type>*
        return const_cast<Img<Type>*>(this)->shallowCopy(roi,channelIndices,fmt,time,0);
      }
      

      /// Create a shallow copy of this image with a new format
      /** @param newFmt new format to choose. This must be compatible to the channel count 
                           of this image.
          @param poDst destination image (exploited as possible) 
          @return shallow copie with given format of NULL if an error occured 
      **/
      Img<Type> *reinterpretChannels(format newFmt, Img<Type> *poDst = NULL){
         ImgBase *poDstBase = poDst;
         return shallowCopy(getROI(),std::vector<int>(),newFmt,getTime(),&poDstBase);
      }
      
      
      /// Create a shallow copy of this image with a new format (const version)
      /** @param newFmt new format to choose. This must be compatible to the channel count 
                           of this image. 
          @return shallow copie with given format of NULL if an error occured 
      **/
      const Img<Type> *reinterpretChannels(format newFmt){
        return shallowCopy(getROI(),std::vector<int>(),newFmt,getTime());
      }
      /// Create a shallow copy of the image
      /** It exploits the given destination image if possible,
          i.e. if the pixel depth matches. Else this image is released
          and a new one is created. Optionally a second argument can be
          specified to get a new image with the given ROI.
          @param poDst pointer to the destination image pointer If ppoDst is NULL,
                        a new image is created, if ppoDst points to NULL, a new 
                        image is created at *ppoDst;
          @param roi new ROI of the new image. If Rect::null, the source images roi
                     is used.
          @return shallow copy of this image
      **/
      Img<Type>* shallowCopy(const Rect &roi,Img<Type>* poDst = NULL){
        ImgBase *poDstBase = poDst;
        return shallowCopy(roi,std::vector<int>(),getFormat(),getTime(),&poDstBase);
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
      const Img<Type>* shallowCopy(const Rect& roi) const {
         // casting constness away is safe, because we effectively return a const Img<Type>*
         return const_cast<Img<Type>*>(this)->shallowCopy(roi,0);
      }
     

      /// Create a shallow copy of selected channels of an image
      /** This function can be used if only one or some channels of a given const 
          image should be used in further processing steps. It helps to avoid the 
          necessity of "deepCopy" calls there.
          @param channelIndices vector containing channel indices to copy
          @param poDst destination image (if Null, a new one is created)
          @return image containing only the selected channels (as shallow copies)
                        format of that image becomes formatMatrix
          @see shallowCopy
      */
      Img<Type>* selectChannels (const std::vector<int>& channelIndices, Img<Type>* poDst=0){
        ImgBase *poDstBase = poDst;
        return shallowCopy(getROI(),channelIndices,formatMatrix,getTime(),&poDstBase);
      }

      /// Create a shallow copy of a single image channel of an image
      /** This function is a shortcut to use 
          icl::ImgBase::selectChannels(const std::vector<int>&,icl::ImgBase**) to 
          select a single channel from an image
          @param channelIndex index of the channel to select (if invalid, NULL is returned)
          @param poDst destination image 
          @return image containing only the selected channel
      **/
      Img<Type> *selectChannel(int channelIndex, Img<Type> *poDst=0){
        ICLASSERT_RETURN_VAL(validChannel(channelIndex), 0);
        std::vector<int> v(1); v[0]= channelIndex; 
        return selectChannels(v,poDst);
      }
      /// Create a shallow copy of selected channels of a const image.
      /** @param channelIndices vector containing channel indices to copy
          @return const image containing only the selected channels
      */
      const Img<Type>* selectChannels (const std::vector<int>& channelIndices) const {
         // casting constness away is safe, because we effectively return a const Img<Type>*
         return const_cast<Img<Type>*>(this)->selectChannels(channelIndices, 0);
      }
      
      /// Create a shallow copy of a single image channel of a const image
      /** This function is a shortcut to use 
          icl::ImgBase::selectChannels(const std::vector<int>&)const to 
          select a single channel from a const image image
          @param channelIndex index of the channel to select (if invalid, NULL is returned)
          @return const image containing only the selected channel
      **/
      const Img<Type> *selectChannel(int channelIndex) const{
        ICLASSERT_RETURN_VAL(validChannel(channelIndex), 0);
        std::vector<int> v(1); v[0]= channelIndex; return selectChannels(v);
      }

      //------------------------------------------------------------------------------   
      //------------------------------------------------------------------------------    
      //------------------------------------------------------------------------------   





    /// Perform a deep copy of an image
    /** \copydoc icl::ImgBase::deepCopy(icl::ImgBase**)const */
    virtual Img<Type>* deepCopy(ImgBase** ppoDst=0) const;

    /// Perform a deep copy of an image
    /** This is an overloaded version of the above function. It behaves
        essentially like the above function, except getting an Img<Type>* as
        destination image argument, what allows to apply the operation without
        a depth switch.
        @param poDst destination image, if NULL, a new Img<Type> is created
        @return deep copy of this image
    **/
    Img<Type> *deepCopy(Img<Type> *poDst) const;

    /// Perform a deep copy of an images ROI
    /** \copydoc icl::ImgBase::deepCopyROI(icl::ImgBase**)const */
    virtual Img<Type> *deepCopyROI(ImgBase **ppoDst=0) const;

    
    /// Perform a deep copy of an images ROI
    /** This is an overloaded version of the above function. It behaves
        essentially like the above function, except getting an Img<Type>* as
        destination image argument, what allows to apply the operation without
        a depth switch.
        @param poDst destination image, if NULL, a new Img<Type> is created. poDst
                     must have either ROI size identical to this images ROI size or
                     zero-dim size (in this case poDst's size is set to this
                     images ROI size)
        @return deep copy of this images ROI
    **/
    Img<Type> *deepCopyROI(Img<Type> *poDst) const;

    /// @}
    /* }}} */
  
    /** @{ scaled-, and flipped-Copy functions */
    /* {{{ open */
   
    /// create a scaled copy of this image
    /** \copydoc icl::ImgBase::scaledCopy(const icl::Size&,icl::scalemode)const */
    virtual Img<Type> *scaledCopy(const Size &newSize, scalemode eScaleMode=interpolateNN) const;
    
    /// create a scaled copy of this image
    /** \copydoc icl::ImgBase::scaledCopy(icl::ImgBase**,icl::scalemode)const */
    virtual Img<Type> *scaledCopy(ImgBase **ppoDst=0, scalemode eScaleMode=interpolateNN) const;

   
    /// create a scaled copy of this image
    /** Overloaded function to create a scaled copy of an image. This function gets
        an Img<Type>* as destination, what allows to apply the operation without any
        depth-switch.
        @param poDst destination image pointer, if NULL, a new Img<Type> is created
        @param eScaleMode interpolation method to use when scaling
    */
    Img<Type> *scaledCopy(Img<Type> *poDst, scalemode eScaleMode=interpolateNN) const;
    
    /// create a scaled copy of an images ROI with given size
    /** \copydoc icl::ImgBase::scaledCopyROI(const icl::Size&,icl::scalemode)const */
    virtual Img<Type> *scaledCopyROI(const Size &newSize, scalemode eScaleMode=interpolateNN) const;
    
    /// create a scaled copy of an images ROI with given destination image
    /** \copydoc icl::ImgBase::scaledCopyROI(icl::ImgBase**,icl::scalemode)const*/
    virtual Img<Type> *scaledCopyROI(ImgBase **ppoDst=0, scalemode eScaleMode=interpolateNN) const;
    
    /// create a scaled copy of this images ROI
    /** Overloaded function to create a scaled copy of an images ROI. This function gets
        an Img<Type>* as destination, what allows to apply the operation without any
        depth-switch.
        @param poDst destination image pointer, if NULL, a new Img<Type> is created
        @param eScaleMode interpolation method to use when scaling
    */
    Img<Type> *scaledCopyROI(Img<Type> *poDst, scalemode eScaleMode=interpolateNN) const;    

    /// @}
    /* }}} */

    /** @{ @name organization and channel management */
    /* {{{ open */

    /// Makes the image channels inside the Img independent from other Img.
    /** \copydoc icl::ImgBase::detach(int) */
    virtual void detach(int iIndex = -1);
  
    /// Removes a specified channel.
    /** \copydoc icl::ImgBase::removeChannel(int) */
    virtual void removeChannel(int iChannel);
  
    /// Append channels of external Img to the existing Img. 
    /** Both objects will share their data (cheap copy). 
        If a non-matrix format image gets new channels using it's 
        append method,
        the new channel count will not match to the channel count,
        that is associated with the current format. In this case, a
        waring is written to std::out, and the format will be set to 
        formatMatrix implicitly.
        @param src source image
        @param iChannel channel to append (or all, if < 0)
    **/
    void append(Img<Type> *src, int iChannel=-1) {
      // call private const-version
      this->append (static_cast<const Img<Type>*>(src), iChannel);
    }
  
    /// Append a set of selected channels from source image
    /** @param src source image
        @param vChannels vector of channels indices */
    void append(Img<Type> *src, const std::vector<int>& vChannels) {
      // call private const-version
      this->append (static_cast<const Img<Type>*>(src), vChannels);
    }  

    /// Returns a new image with a shallow copied single channel of this image
    /** param index channel index to extract (must be valid, else resulting image 
                    has no channels and error message) */
    Img<Type> extractChannel(int index);

    /// Returns a new image with a shallow copied single channel of this image
    /** param index channel index to extract (must be valid, else resulting image 
                    has no channels and error message) */
    const Img<Type> extractChannel(int index) const;

    /// Returns a new image with shallow copied single channels of this image
    /** param indices channel indices to extract (each must be valid, else error and
                      channel index that does not match is omitted) */
    Img<Type> extractChannels(const std::vector<int> &indices);

    /// Returns a new image with shallow copied single channels of this image
    /** param indices channel indices to extract (each must be valid, else error and
                      channel index that does not match is omitted) */
    const Img<Type> extractChannels(const std::vector<int> &indices) const;

    
    /// Swap channel A and B
    /** \copydoc icl::ImgBase::swapChannels(int,int) */
    virtual void swapChannels(int iIndexA, int iIndexB);
  
    /// Replace the channel A of this image with the channel B another image. 
    /** Both images must have the same width and height.
        @param iThisIndex channel to replace
        @param iOtherIndex channel to replace with
        @param poOtherImg Image that contains the new channel
        **/
    void replaceChannel(int iThisIndex, Img<Type> *poOtherImg, int iOtherIndex);

    /// sets the channel count to a new value
    /** \copydoc icl::ImgBase::setChannels(int) */
    virtual void setChannels(int iNewNumChannels);

    /// resizes the image to new values
    /** \copydoc icl::ImgBase::setSize(const icl::Size&) */
    virtual void setSize(const Size &s);
  
    /// @}

    /* }}} */
  
    /** @{ @name getter functions */
    /* {{{ open */

    /// Returns max pixel value of channel iChannel within ROI
    /** @param iChannel Index of channel
        @param coords (optinal) if not null, the pixel position of the max is 
               written into this argument
        **/
    Type getMax(int iChannel, Point *coords=0) const;
  
    /// Returns min pixel value of channel iChannel within ROI
    /** @param iChannel Index of channel
        @param coords (optinal) if not null, the pixel position of the min is 
               written into this argument
        **/
    Type getMin(int iChannel, Point *coords=0) const;
  

    /// return maximal pixel value over all channels (restricted to ROI)
    Type getMin() const;

    /// return minimal pixel value over all channels (restricted to ROI)
    Type getMax() const;

    /// Returns min and max pixel values of channel iChannel within ROI
    /** @param iChannel Index of channel
        @param minCoords (optinal) if not null, the pixel position of the min is 
               written into this argument
        @param maxCoords (optinal) if not null, the pixel position of the max is 
               written into this argument
    **/
    const Range<Type> getMinMax(int iChannel, Point *minCoords=0, Point *maxCoords=0) const;

    /// return minimal and maximal pixel values over all channels (restricted to ROI)
    const Range<Type> getMinMax() const;

    /// Returns the width of an image line in bytes
    /** \copydoc icl::ImgBase::getLineStep()const **/
    virtual int getLineStep() const{
      return getSize().width*sizeof(Type);
    }

    /// returns a Type save data data pointer to the channel data origin
    /** If the channel index is not valid (<0 or >= getChannels) NULL 
        is returned and an error is written to std::err
        @param iChannel specifies the channel 
        @return data origin pointer to the specified channel 
    */
    Type* getData(int iChannel) { 
      return const_cast<Type*>(static_cast<const Img<Type>*>(this)->getData(iChannel));
    }
    
    /// returns a Type save data data pointer to the channel data origin (const)
    /** \copydoc getData(int)*/
    const Type* getData(int iChannel) const {
      FUNCTION_LOG("");
      ICLASSERT_RETURN_VAL(validChannel(iChannel), 0);
      return m_vecChannels[iChannel].get();
    }
  
    /// returns a Type save data pointer to the first pixel within the images roi
    /** The following ASCII image shows an images ROI.
        <pre>
                       1st roi-pixel
                            |
                        ....|....................         ---
                        ....|..ooooooooo......... ---      |
                        ....|..ooooooooo.........  |       |
                        ....|..ooooooooo......... roi-h  image-h
        1st image pixel ....|..ooooooooo.........  |       |
             |          ....+->xoooooooo......... ---      |
             +--------->x........................         ---
                               |-roi-w-|
                        |---------image-w-------|
  
        </pre>
        <b>Note:</b> most ipp-function require the ROI-data pointer
        @param iChannel specifies the channel
        @return roi data pointer
    **/
    Type* getROIData(int iChannel) {
      return const_cast<Type*>(static_cast<const Img<Type>*>(this)->getROIData(iChannel));
    }

    /// returns a Type save data pointer to the first pixel within the images roi (const)
    /** \copydoc getROIData(int) */
    const Type* getROIData(int iChannel) const {
      FUNCTION_LOG("");
      ICLASSERT_RETURN_VAL(validChannel(iChannel),0);
      return getData(iChannel) + m_oParams.getPixelOffset();
    }

    /// returns the data pointer to a pixel with defined offset
    /** In some functions like filters, it might be necessary to change the images
        ROI parameters before applying the underlying image operation. 
        Temporarily changing the images ROI parameters causes problems in
        multi-threaded environments. To avoid this, this function provides
        access to a data pointer to an arbitrary notional ROI-offset
        @param iChannel selects the channel
        @param p notional ROI offset
        @return data pointer with notional ROI offset p
        **/
    Type* getROIData(int iChannel, const Point &p) {
      return const_cast<Type*>(static_cast<const Img<Type>*>(this)->getROIData(iChannel, p));
    } 
    /// returns the data pointer to a pixel with defined offset (const)
    /** \copydoc getROIData(int,const icl::Point&) */
    const Type* getROIData(int iChannel, const Point &p) const {
      FUNCTION_LOG("");
      ICLASSERT_RETURN_VAL(validChannel(iChannel),0);
      return getData(iChannel) + p.x + (p.y * getWidth());
    }

    /// returns the raw- data pointer of an image channel
    /** \copydoc icl::ImgBase::getDataPtr(int) **/
    virtual void* getDataPtr(int iChannel){
      return getData(iChannel);
    }
      
    /// returns the raw- data pointer of an image channel (const)
    /** \copydoc icl::ImgBase::getDataPtr(int)const **/
    virtual const void* getDataPtr(int iChannel) const{
      return getData(iChannel); 
    }
  
    /// @}

    /* }}} */
  
    /** @{ @name basic in-place image manipulations */
    /* {{{ open */

    
    /// STL based "for_each" implementations applying an Unary function on each ROI-pixel of given channel
    /** Internally this function uses std::for_each
        Example:\n
        \code
        #include <iclQuick.h>

        struct Thresh{
          inline void operator()(float &f){ f = f>128 ? 0 : 255; }
        };
        
        inline void ttt(float &f){
          f = f>128 ? 0 : 255;
        }
        
        int main(){
          ImgQ a = create("parrot");
        
          a.forEach_C(ttt,1);
          a.forEach_C(Thresh(),0);
        
          show(scale(a,0.2));
          return 0;
        }       
        \endcode
        @param f unary function or functor implementing "AnyType operator()(Type &val)" 
        @param channel valid channel index for this image
    */
    template<typename UnaryFunction>
    inline Img<Type> &forEach_C(UnaryFunction f, int channel){
      ICLASSERT_RETURN_VAL(validChannel(channel),*this);
      if(hasFullROI()){
        std::for_each<Type*,UnaryFunction>(getData(channel),getData(channel)+getDim(),f);
      }else{
        const_roi_iterator end = endROI(channel);
        for(ImgIterator<Type> it = beginROI(channel); it != end; it.incRow()){
          std::for_each<Type*,UnaryFunction>(&(*it),&(*it)+it.getROIWidth(),f);
        }        
      }
      return *this;
    }

    /// STL based "for_each" implementations applying an Unary function on each ROI-pixel
    /** Internally using std::for_each by calling Img<T>::forEach_C for all channels
        Example:\n
        \code
        #include <iclQuick.h>

        struct Thresh{
          inline void operator()(float &f){ f = f>128 ? 0 : 255; }
        };
        
        int main(){
          ImgQ a = scale(create("parrot"),0.2);
          a.forEach(Thresh());
          show(a);
          return 0;
        }

        \endcode
        @param f unary function or functor implementing "AnyType operator()(Type &val)" 
    */
    template<typename UnaryFunction>
    inline Img<Type> &forEach(UnaryFunction f){
      for(int c=0;c<getChannels();++c){
        forEach_C(f,c);
      }
      return *this;
    }
    
    /// STL based "transform" implementation applying an Unary function on ROI-pixles with given destination image
    /** Internally this function uses std::transform.         
        Example:\n
        \code
        #include <iclQuick.h>
        
        struct Thresh{
          inline float operator()(const float &f){ return f>128 ? 0 : 255; }
        };
        
        int main(){
          ImgQ a = scale(create("parrot"),0.2);
          ImgQ b(a.getParams());
        
          a.transform_C(Thresh(),1,2,b);
          show(b);
          return 0;
        }
        \endcode
        @param f unary function of functor implementing "dstType operator()(const Type &val)"
        @param srcChannel valid channel index for this image
        @param dstChannel valid channel index for dst image
        @param dst destination image with identical ROI-size to this images ROI-size
    */
    template<typename UnaryFunction, class dstType>
    inline Img<dstType> &transform_C(UnaryFunction f, int srcChannel, int dstChannel, Img<dstType> &dst) const{
      ICLASSERT_RETURN_VAL(getROISize() == dst.getROISize(),dst);
      ICLASSERT_RETURN_VAL(validChannel(srcChannel),dst);
      ICLASSERT_RETURN_VAL(dst.validChannel(dstChannel),dst);

      if(hasFullROI() && dst.hasFullROI()){
        std::transform(getData(srcChannel),getData(srcChannel)+getDim(),dst.getData(dstChannel),f);
      }else{
        ImgIterator<dstType> itDst = dst.beginROI(dstChannel);
        const_roi_iterator end = endROI(srcChannel);
        for(const_roi_iterator it = beginROI(srcChannel); it != end; it.incRow(), itDst.incRow()){
          std::transform(&(*it),&(*it)+it.getROIWidth(),&(*itDst),f);
        }        
      }
      return dst;
    }
  
    /// STL based "transform" implementation applying an Unary function on ROI-pixles with given destination image
    /** Internally this function uses std::transform.         
        Example:\n
        \code
        #include <iclQuick.h>
        
        inline icl8u t_func(const float &f){ 
          return f>128 ? 0 : 255; 
        }
        
        int main(){
          ImgQ a = scale(create("parrot"),0.2);
          Img8u b(a.getParams());
        
          a.transform(t_func,b);
          show(cvt(b));
          return 0;
        }

        \endcode
        @param f unary function of functor implementing "dstType operator()(const Type &val)"
        @param dst destination image with identical ROI-size and channel count (compared to this image)
    */
    template<typename UnaryFunction,class dstType>
    inline Img<dstType> &transform(UnaryFunction f, Img<dstType> &dst) const{
      ICLASSERT_RETURN_VAL(getChannels() == dst.getChannels(),dst);
      for(int c=0;c<getChannels();++c){
        transform_C(f,c,c,dst);
      }
      return dst;
    }


    /// STL-based "transform function combining two images pixel-wise into a given destination image (with ROI support)"
    /** Internally this function uses std::transform.         
        Example:
        \code
        #include <iclQuick.h>
        
        inline bool gt_func(const float &a,const float &b){
          return a > b;
        }
        struct EqFunctor{
          bool operator()(const float &a, const float &b){
            return a == b;
          }
        };
         
        int main(){
          ImgQ a = scale(create("parrot"),640,480);
          ImgQ b = scale(create("women"),640,480);
        
          Img8u dst(a.getParams());
        
          a.setROI(Rect(10,10,500,400)); 
          b.setROI(Rect(10,10,500,400));
          dst.setROI(Rect(10,10,500,400));
        
          a.combine_C(std::less<icl32f>(),0,0,0,b,dst);
          a.combine_C(gt_func,1,1,1,b,dst);  
          a.combine_C(EqFunctor(),2,2,2,b,dst);
        
          dst.setFullROI();
          show(norm(cvt(dst)));
          return 0;
        }
        \endcode
        @param f binary function of functor implementing "dstType operator() const(Type &a, otherSrcType &b) const"
        @param thisChannel valid channel of this image
        @param otherSrcChannel valid channel index for give 2nd source image
        @param dstChannel valid channel index of dst image
        @param otherSrc 2nd source image (ROI-size must be equal to this' ROI size)
        @param dst destination image (ROI-size must be equal to this' ROI size)
    */
    template<typename BinaryFunction, class dstType, class otherSrcType>
    inline Img<dstType> &combine_C(BinaryFunction f, 
                                   int thisChannel, 
                                   int otherSrcChannel, 
                                   int dstChannel, 
                                   const Img<otherSrcType> &otherSrc, 
                                   Img<dstType> &dst) const{
      ICLASSERT_RETURN_VAL(getROISize() == dst.getROISize()&& getROISize() == otherSrc.getROISize(),dst);
      ICLASSERT_RETURN_VAL(validChannel(thisChannel),dst);
      ICLASSERT_RETURN_VAL(otherSrc.validChannel(otherSrcChannel),dst);
      ICLASSERT_RETURN_VAL(dst.validChannel(dstChannel),dst);

      if(hasFullROI() && dst.hasFullROI() && otherSrc.hasFullROI()){
        std::transform(getData(thisChannel),getData(thisChannel)+getDim(),otherSrc.getData(otherSrcChannel),dst.getData(dstChannel),f);
      }else{
        ImgIterator<dstType> itDst = dst.beginROI(dstChannel);
        const ImgIterator<otherSrcType> itOtherSrc = otherSrc.beginROI(otherSrcChannel);
        const_roi_iterator end = endROI(thisChannel);
        for(const_roi_iterator it = beginROI(thisChannel); it!=end; it.incRow(), itDst.incRow(),itOtherSrc.incRow()){
          std::transform(&(*it),&(*it)+it.getROIWidth(),&(*itOtherSrc),&(*itDst),f);
        }        
      }
      return dst;
    }

    
    /// STL-based "transform function combining two images pixel-wise into a given destination image (with ROI support)"
    /** This function calls D = F(A,B), with D: destination ROI pixel, F: given binary functor/function,
        A: pixel of this image and B: pixel of other given src image. Internally this function uses std::transform
        Beispiel:
        \code
        #include <iclQuick.h>
        
        int main(){
          ImgQ a = scale(create("parrot"),640,480);
          ImgQ b = scale(create("women"),640,480);
        
          Img8u dst(a.getParams());
        
          a.setROI(Rect(10,10,300,100));
          b.setROI(Rect(10,10,300,100));
          dst.setROI(Rect(10,10,300,100));
        
          a.combine(std::less<icl32f>(),b,dst);
        
          dst.setFullROI();
          show(norm(cvt(dst)));
          return 0;
        }

        \endcode
        @param f binary function or functor implementing "dstType operator() const(Type &a, otherSrcType &b) const"
        @param otherSrc 2nd source image
        @param dst destination image
    */
    template<typename BinaryFunction, class dstType, class otherSrcType>
    inline Img<dstType> &combine(BinaryFunction f, const Img<otherSrcType> &otherSrc, Img<dstType> &dst) const{
      ICLASSERT_RETURN_VAL(getChannels() == otherSrc.getChannels(),dst);
      ICLASSERT_RETURN_VAL(getChannels() == dst.getChannels(),dst);
      for(int c=0;c<getChannels();++c){
        combine_C(f,c,c,c,otherSrc,dst);
      }
      return dst;
    }
    


    private:
    /// private helper function called from reduce_channels template
    template<typename Tsrc, typename Tdst, int Nsrc, int Ndst, typename ReduceFunc>
    static inline void reduce_arrays(const Tsrc *src[Nsrc], Tdst *dst[Ndst], unsigned int dim, ReduceFunc reduce){
      for(int i=dim-1;i>=0;--i){
        Tsrc tsrc[Nsrc];      
        Tdst tdst[Ndst];
        for(int j=0;j<Nsrc;tsrc[j]=src[j][i],++j) {}
        reduce(tsrc,tdst);
        for(int j=0;j<Ndst;dst[j][i]=tdst[j],++j) {}
        
      }
    }
    
    public:
    

    /// Utility function for combining image channels into another image
    /** In some application we want to combine an images channels pixel-by-pixel
        in any way and to store the result (or even results) in a 2nd image
        at the corresponding image location. This can easily be performed using
        the reduce channels template function. Look at the following example:
        <CODE>
        struct Thresh{
           Thresh(int t):t(t*3){}
           int t;    
           void operator()(const icl8u src[6], icl8u dst[1]) const{
             *dst = 255*( (abs(src[0]-src[3])+abs(src[1]-src[4])+abs(src[2]-src[5])) > t);
           }
        };

        Img8u bgMask(const Img8u &image,const Img8u &bgImage, int tollerance){
           Img8u imageAndBG;
           imageAndBG.append(&const_cast<Img8u&>(image));   // const_cast is ok here,
           imageAndBG.append(&const_cast<Img8u&>(bgImage)); // we will not change them!
           Img8u dst(image.getSize(),formatMatrix);
           imageAndBG.reduce_channels<icl8u,6,1,Thresh>(dst,Thresh(tollerance));
           return dst;
        }
        </CODE>

        \section PERF Performace Twearks
        Source and destination channel count is given as template parameter, to allow
        the compiler to leave out loops over single values or to unroll short ones. Besides,
        the compiler is able to use fixed sized arrays on the stack, instead of dynamically 
        allocated heap arrays. By this means performance of reduce_channels is accelerated
        by factor <b>12</b>.

        \section BENCHMARK Benchmarks
        Exemplarily, the example function above takes about 4-5ms on a 2GHz Core2Duo (using 
        -O4 and -funroll-loop optimization of gcc) on a VGA-sized image (640x480)
    */
    template<typename Tdst, int Nthis, int Ndst, typename ReduceFunc>
    void reduce_channels(Img<Tdst> &dst, ReduceFunc reduce) const {
      ICLASSERT_RETURN(this->getROISize() == dst.getROISize());
      ICLASSERT_RETURN((Nthis > 0) && (Ndst > 0));
      ICLASSERT_RETURN((this->getChannels()==Nthis) &&  (dst.getChannels()==Ndst));
      
      const Type *psrc[Nthis];
      Tdst *pdst[Ndst];  
      if(this->hasFullROI() && dst.hasFullROI()){
        for(int i=0;i<Nthis;psrc[i]=this->getData(i),++i) {}
        for(int i=0;i<Ndst;pdst[i]=dst.getData(i),++i) {}
        reduce_arrays<Type,Tdst,Nthis,Ndst,ReduceFunc>(psrc,pdst,this->getDim(),reduce);
      }else{
        const_roi_iterator itSrc[Nthis];
        ImgIterator<Tdst> itDst[Ndst];
        for(int i=0;i<Nthis;itSrc[i]=this->beginROI(i),++i) {}
        for(int i=0;i<Ndst;itDst[i]=dst.beginROI(i),++i) {}
        
        for(int l=this->getROI().height-1, w=this->getROI().width ;l>=0;--l){
          for(int i=0;i<Nthis;itSrc[i].incRow(),++i){
            psrc[i]=&(*(itSrc[i]));
          }
          for(int i=0;i<Ndst;itDst[i].incRow(),++i){
            pdst[i]=&(*(itDst[i]));
          }
          reduce_arrays<Type,Tdst,Nthis,Ndst,ReduceFunc>(psrc,pdst,w,reduce);
        }
      }
    }
    
    /// perform an in-place resize of the image (keeping the data) 
    /** \copydoc icl::ImgBase::scale(const icl::Size&,icl::scalemode)*/
    virtual void scale(const Size &s, scalemode eScaleMode=interpolateNN);

    /// perform an in-place mirror operation on the image
    /** \copydoc icl::ImgBase::mirror(icl::axis,bool) */
    virtual void mirror(axis eAxis, bool bOnlyROI=false);
  
    /// Sets the ROI pixels of one or all channels to a specified value
    /** @param iChannel Channel to fill with zero (default: -1 = all channels)
        @param tValue destination value (default: 0)
        @param bROIOnly if set false, the whole image is set to tValue
        **/
    void clear(int iChannel = -1, Type tValue = 0, bool bROIOnly=true);
  
    /// Normalize the channel min/ max range to the new min, max range.
    /** The min/ max range from the source channels are automatically detected,
        <b>separately</b> for each channel.
        @param dstRange new image range
    **/
    void normalizeAllChannels(const Range<Type> &dstRange);

    /// Normalize the channel from a given min/max range to the new range 
    /** @param iChannel channel index
        @param srcRange notional image range befor this function call
        @param dstRange image range after this function call
    **/
    void normalizeChannel(int iChannel, const Range<Type> &srcRange, const Range<Type> &dstRange);

    /// Normalize the channel from a given min/max range to the new range 
    /** The min/ max range from the source channel is automatically detected,
        separately for this channel
        @param iChannel channel index
        @param dstRange destination image range
    **/
    void normalizeChannel(int iChannel,const Range<Type> &dstRange);

    /// Normalize the image from a given min/max range to the new range 
    /** @param srcRange notional image range befor this function call
        @param dstRange image range after this function call
    **/
    void normalizeImg(const Range<Type> &srcRange, const Range<Type> &dstRange);
  
    /// Normalize the image from a min/max range to the new range 
    /** The min/ max range from the image is automatically detected, combined 
        over all image channels.
        @param dstRange destination image range
    **/
    void normalizeImg(const Range<Type> &dstRange);
  
    /// @}

    /* }}} */
  
    /** @{ @name pixel access using roi iterator */          
    /* {{{ open */
    
    /// iterator type (just a data pointer)
    typedef Type* iterator;

    /// const iterator type (just a const pointer)
    typedef const Type* const_iterator;

    /// type definition for ROI iterator
    typedef ImgIterator<Type> roi_iterator;

    /// type definition for a const ROI iterator
    typedef const ImgIterator<Type> const_roi_iterator;
    // old    typedef constConstImgIterator<Type> const_iterator;
    


    /// returns the image iterator (equal to getData(channel))
    iterator begin(int channel){
      return getData(channel);
    }
    
    /// returns the image iterator (equal to getData(channel)) (const)
    const_iterator begin(int channel) const{
      return const_cast<Img<Type>*>(this)->begin(channel);
    }

    /// returns the image end-iterator (equal to getData(channel)+getDim())
    iterator end(int channel){
      return getData(channel)+getDim();
    }

    /// returns the image end-iterator (const)
    const_iterator end(int channel) const{
      return getData(channel)+getDim();
    }

    /// returns the iterator for an images ROI
    inline roi_iterator beginROI(int channel){
      ICLASSERT_RETURN_VAL(validChannel(channel), roi_iterator());
      return roi_iterator(getData(channel),getWidth(),getROI());
    } 

    /// returns the iterator for an images ROI (const)
    inline const_roi_iterator beginROI(int channel) const{
      ICLASSERT_RETURN_VAL(validChannel(channel), roi_iterator());
      return const_cast<Img<Type>*>(this)->beginROI(channel);
    } 
    
    /// returns the end-iterator for an images ROI
    /** the returned iterator must not be incremented or decremented! */
    inline roi_iterator endROI(int channel) {
      ICLASSERT_RETURN_VAL(validChannel(channel), roi_iterator());
      return roi_iterator::create_end_roi_iterator(this,channel,getROI());
    }

    /// returns the end-iterator for an images ROI (const)
    inline const_roi_iterator endROI(int channel) const{
      ICLASSERT_RETURN_VAL(validChannel(channel), roi_iterator());
      return const_roi_iterator::create_end_roi_iterator(this,channel,getROI());
    }

    
    /// returns the x,y-coordinates of a pointer whithin a given channel
    /** E.g channel c's pointer of image I points to adress p=1005, image width is
        10 and data depth is 1 (icl-8u image). Then I.getLocation(1006,c) returns
        Point(1,0) and I.getLocation(1015,c) returns Point(0,1).\n
        The following rule is always true:
        <code>
        Point p = somewhat;
        image i = somewhat;
        p == i.getLocation(i.getData(0)+p.x+image.getWidth()*p.y,any-valid-channel);
        </code>
        Optionally, returned point can be calculated w.r.t. the images roi offset.
    **/
    Point getLocation(const Type *p, int channel, bool relToROI=false) const{
      ICLASSERT_RETURN_VAL(validChannel(channel), Point::null);
      ICLASSERT_RETURN_VAL(getDim(), Point::null);
      int offs = (int)(getData(channel)-p);
      int x = offs%getWidth();
      int y = offs/getWidth();
      if(relToROI){
        return Point(x-getROI().x,y-getROI().y);
      }else{
        return Point(x,y);
      }
    }
    
    /// shows the image value by value at std::out
    /** Warning: <b>SLOW</b>
        @param format this string is passed to printf internally
               uchars can be printed e.g. using format="3.0"
        @param visROI indicates ROI-pixels with a 'r'-postfix (only
               if image has no full-ROI)
    **/
    void printAsMatrix(const std::string &format="5.3", bool visROI=true) const;


#if 0
    REMOVED since ICL 4.1
    /// returns the iterator for the hole image 
    /** The following example taken from ImgIterator.h will show
        the iterator usage:
        \code
        void channel_convolution_3x3(Img32f &src, Img32f &dst,icl32f *pfMask, int iChannel){
           for(Img32f::iterator s=src.getIterator(iChannel) d=dst.getIterator(iChannel) ; s.inRegion() ; s++,d++){
              icl32f *m = pfMask;
              (*d) = 0;
              for(Img32f::iterator sR(s, 3, 3); sR.inRegion(); sR++,m++){
                 (*d) += (*sR) * (*m);
              }
           }  
        }
        \endcode
        <b>Note:</b> The performance hints illustrated in the
        ImgIterator documentation.
        @param iChannel selected channel index
        @return iterator
        @see ImgIterator
        @see end
    **/
    inline iterator getIterator(int iChannel)
    {
      FUNCTION_LOG("begin(" << iChannel << ")");
      ICLASSERT_RETURN_VAL(validChannel(iChannel), iterator());
      return iterator(getData(iChannel),getWidth(),Rect(Point::null,getSize()));
    }
    /// returns an iterator to an images ROI pixels
    /** this function behaves essentially like the above function 
        @param iChannel selects a channel
        @return roi-iterator
        @see getIterator
        */
    inline iterator beginROI(int iChannel)
    {
      FUNCTION_LOG("begin(" << iChannel << ")");
      ICLASSERT_RETURN_VAL(validChannel(iChannel), iterator());
      return iterator(getData(iChannel),getWidth(),getROI());
    } 

    /// const version of getIterator
    inline const_iterator getIterator(int iChannel) const
    {
      FUNCTION_LOG("begin(" << iChannel << ")");
      ICLASSERT_RETURN_VAL(validChannel(iChannel), const_iterator());
      return const_iterator(getData(iChannel),getWidth(),Rect(Point::null,getSize()));
    }
    /// const version of beginROI
    inline const_iterator beginROI(int iChannel) const
    {
      FUNCTION_LOG("begin(" << iChannel << ")");
      ICLASSERT_RETURN_VAL(validChannel(iChannel), const_iterator());
      return const_iterator(getData(iChannel),getWidth(),getROI());
    } 
 #endif
    /// @}

    /* }}} */
  
  };// class Img<Type>

  
  /* {{{ global functions */

  /// utility class that helps for an implicit conversion between Img<T>* to ImgBase**  \ingroup IMAGE
  /** @see bpp(Img<T> &)
      @see bpp(Img<T> *)
  */
  template<class T> struct ImgBasePtrPtr {
    /// 1st private constructor
    ImgBasePtrPtr(Img<T> &i);
    
    /// 2nd private constructor
    ImgBasePtrPtr(Img<T> *i);

    /// Destructor 
    /** In this class, the destrutctor does all of the work:
        if the pointer returned by the implicit cast to 
        ImgBase** operator was chnaged (reallocated) it
        is detected here, and a warnig is writte to std::out.
        To ensure compability for later computation, the
        reallocated images data is converted into the given
        source image. By this means, the result image gets all
        parameters of the source image except its depth. Images,
        that are not further needed, are deleted automatically.
    */
    ~ImgBasePtrPtr();
    
    /// Implicit cast operator (allows to place an ImgBasePtrPtr where an ImgBase** is expected) 
    operator ImgBase** (){ return &r; }

    private:
    /// Internal image pointers
    ImgBase *o,*r,*rbef;
  };

  /// utility function to cast an Img<T> implicitly into an ImgBase **  \ingroup IMAGE
  /** This function may be useful, whenever you are working with
      Img<T> objects are Pointers. As some functions expect ImgBase**
      arguments to ensure, that the destination image can be adapted
      even in its depth, this function can be used to get an ImgBase**
      that points to a pointer of the given image. This is a necessary
      concept, but mutch additional work, if you are just want to try 
      out some algorithm on your Img<T>. If you are shure, that the
      result image will not be adapted by the function you can easily
      use the bpp-function, which will additionally warn you if this 
      assumption was wrong.
      The following code
      example demonstrates this:
      \code
      void apply_func(ImageBase **dst){
         ...
         // this function may realloc dst
      }
      Img8u src;
      Img8u dst;
      
      // without bpp function
      ImgBase *pDst= &Dst;
      apply_func(&src,&pDst);  // this will cause an error if pDst is reallocated
                               // because stack objects may not be released
      
      // with bpp function
      apply_func(&src,bpp(dst)); // this will cause a warning at maxinum
      
      \endcode
  **/
  template<class T>
  ImgBasePtrPtr<T> bpp(Img<T> *image) { return ImgBasePtrPtr<T>(image); }

  /// utility function to cast an Img<T>* implicitly into an ImgBase **  \ingroup IMAGE
  /** see the above function for more details */
  template<class T>
  ImgBasePtrPtr<T> bpp(Img<T> &image) { return ImgBasePtrPtr<T>(image); }
  

 /// Conversion function to transform a pointer into an object
  /** This function is very common: in many cases, Img<T> member functions
      return pointer instead of objects. This might lead to some extra code
      lines, if these pointer are used locally:
      \code
      void foo(const Img8u *image){
        const Img8u *channel0 = image->selectChannel(0);
        const Img8u *imageWithFullROI = image->shallowCopy(image->getImageRect());
      
        // do something with the shallow copies
        // e.g. call some other functions
        show_one_channel_image(*channel0);
        process_image_with_full_roi(*imageWithFullROI);

        delete channel0;
        delete imageWithFullROI;
      }
      \endcode
      
      This code can be written much shorter using the *NEW* p2o function:
      \code
      void foo(const Img8u *image){
        show_one_channel_image(p2o(image->selectChannel(0)));
        process_image_with_full_roi(p2o(image->shallowCopy(image->getImageRect())));
      }
      \endcode
      
      internally: p2o uses a smart pointer to ensure <b>given pointer is released
      properly before function returns</b>

      \section IMPL implementation
      \code
      template<class T>
      inline T p2o(T *ptr){
        return T(*SmartPtr<T>(const_cast<T*>(ptr)));
      }
      \endcode
      
      \section CMPLX Complexity
      the function internally creates a smart Pointer object and calls the copy constructor
      of given T once. Hence the function complexity scales with the implementation of the
      standard copy constructor of T. Particularly for ICL's Img<T> classes, the default 
      copy constructor performs a shallow copy internally which induces negligible 
      computational costs.
   */
  template<class T>
  static inline T p2o(T *ptr){
    return *SmartPtr<T,PointerDelOp>(ptr);
  }

  /// Combine several images using shallow copy. \ingroup IMAGE 
  template<class ImgType>
  const ImgType* combineImages (const std::vector<const ImgType*>& vec);

  /// Combine several images using shallow copy. Non-const version \ingroup IMAGE
  template<class ImgType>
  ImgType* combineImages (const std::vector<ImgType*>& vec) {
    return const_cast<ImgType*>(combineImages(reinterpret_cast<const std::vector<const ImgType*>&>(vec)));
  }

  /* {{{   deepCopyChannel */
  /// Copies the channel from one image to another \ingroup IMAGE
  template<class T>
  inline void deepCopyChannel( const Img<T> *src, int srcC, Img<T> *dst, int dstC){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getSize() == dst->getSize() );
    ICLASSERT_RETURN( src->validChannel(srcC) );
    ICLASSERT_RETURN( dst->validChannel(dstC) );
    icl::copy<T>(src->getData(srcC),src->getData(srcC)+src->getDim(),dst->getData(dstC));
  }

  /* }}} */
 
  /* {{{   convertChannel */

  /// copies/converts the data from one image to another image (IPP-OPTIMIZED) \ingroup IMAGE
  /** The deepCopyChannel function is a higher lever wrapper for the 
      icl::copy(..) function. It extracts the data pointers and data dimension
      from the source- and destination image to call icl::copy(..)
      @param src source image
      @param srcC source images channel
      @param dst destination image
      @param dstC destination image channel
   **/ 
  template<class S,class D> 
  inline void convertChannel(const Img<S> *src, int srcC, Img<D> *dst, int dstC){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( src && dst );
    ICLASSERT_RETURN( src->getSize() == dst->getSize() );
    ICLASSERT_RETURN( src->validChannel(srcC) );
    ICLASSERT_RETURN( dst->validChannel(dstC) );
    icl::convert<S,D>(src->getData(srcC),src->getData(srcC)+src->getDim(),dst->getData(dstC));
  }

  /* }}} */

  /* {{{   clearChannelROI */

  /// sets an arbitrary image ROI to a given value \ingroup IMAGE
  /** This function is used as basic operation for higher level image operation like
      Img<T>::clear(T value).
      @param im image
      @param c channel
      @param clearVal value for the cleared pixels
      @param offs lower left point for the to-be-cleared region
      @param size size of the to-be-cleared region
      */ 
  template<class T>
  inline void clearChannelROI(Img<T> *im, int c, T clearVal, const Point &offs, const Size &size) {
    FUNCTION_LOG("");
    ICLASSERT_RETURN( im );

    ImgIterator<T> it(im->getData(c),im->getSize().width,Rect(offs,size));
    const ImgIterator<T> itEnd = ImgIterator<T>::create_end_roi_iterator(im,c,Rect(offs,size));
    std::fill(it,itEnd,clearVal);
  }

  /** \cond */
#ifdef HAVE_IPP
  /// IPP-OPTIMIZED specialization for icl8u clearing (using ippiSet)
  template <>
  inline void clearChannelROI(Img<icl8u> *im, int c, icl8u clearVal, const Point &offs, const Size &size){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( im );
    ippiSet_8u_C1R(clearVal,im->getROIData(c,offs),im->getLineStep(),size);
  }
  /// IPP-OPTIMIZED specialization for icl16s clearing (using ippiSet)
  template <>
  inline void clearChannelROI(Img<icl16s> *im, int c, icl16s clearVal, const Point &offs, const Size &size){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( im );
    ippiSet_16s_C1R(clearVal,im->getROIData(c,offs),im->getLineStep(),size);
  }
  /// IPP-OPTIMIZED specialization for icl32s clearing (using ippiSet)
  template <>
  inline void clearChannelROI(Img<icl32s> *im, int c, icl32s clearVal, const Point &offs, const Size &size){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( im );
    ippiSet_32s_C1R(clearVal,im->getROIData(c,offs),im->getLineStep(),size);
  }
  /// IPP-OPTIMIZED specialization for icl32f clearing (using ippiSet)
  template <>
  inline void clearChannelROI(Img<icl32f> *im, int c, icl32f clearVal, const Point &offs, const Size &size){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( im );
    ippiSet_32f_C1R(clearVal,im->getROIData(c,offs),im->getLineStep(),size);
  }
#endif
  /** \endcond */

  /* }}} */

  /** {{{  check function */
#define CHECK_VALUES(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize)                                             \
  FUNCTION_LOG("");                                                                                                 \
  ICLASSERT_RETURN( src && dst );                                                                                   \
  ICLASSERT_RETURN( srcSize == dstSize );                                                                           \
  ICLASSERT_RETURN( src->validChannel(srcC) );                                                                      \
  ICLASSERT_RETURN( dst->validChannel(dstC) );                                                                      \
  ICLASSERT_RETURN( srcOffs.x >= 0 && srcOffs.y >= 0 && dstOffs.x >= 0 && dstOffs.y >= 0);                          \
  ICLASSERT_RETURN( srcOffs.x+srcSize.width <= src->getWidth() && srcOffs.y+srcSize.height <= src->getHeight() );   \
  ICLASSERT_RETURN( dstOffs.x+dstSize.width <= dst->getWidth() && dstOffs.y+dstSize.height <= dst->getHeight() );   

  /** }}} */
  

  
  /* {{{   deepCopyChannelROI */

  /// copies the channel roi from one image to another \ingroup IMAGE
  /** Essential deep copy function. 
      @param src source image
      @param srcC source channel
      @param srcOffs source images ROI offset
      @param srcSize source images ROI size
      @param dst destination image 
      @param dstC destination channel
      @param dstOffs destination images ROI offset
      @param dstSize destination images ROI size (must be equal to srcSize) 
  **/
  template <class T>
  inline void deepCopyChannelROI(const Img<T> *src,int srcC, const Point &srcOffs, const Size &srcSize,
                                 Img<T> *dst,int dstC, const Point &dstOffs, const Size &dstSize) {
    CHECK_VALUES(src,srcC,srcOffs,srcSize,dst,dstC,dstOffs,dstSize);
  
    const ImgIterator<T> itSrc(const_cast<T*>(src->getData(srcC)),src->getSize().width,Rect(srcOffs,srcSize));
    ImgIterator<T> itDst(dst->getData(dstC),dst->getSize().width,Rect(dstOffs,dstSize));
    const ImgIterator<T> itSrcEnd = ImgIterator<T>::create_end_roi_iterator(src,srcC,Rect(srcOffs,srcSize));

    for(;itSrc != itSrcEnd;itSrc.incRow(),itDst.incRow()){
      icl::copy<T>(&*itSrc,&*itSrc+srcSize.width,&*itDst);
    }
  }

  /* }}} */

  /* {{{   convertChannelROI */

  /// @{ @name type conversion of channel ROIs 
  /// copies/converts the ROI data from one image to the ROI of another image (IPP-OPTIMIZED) \ingroup IMAGE
  /** This function is used by all other deepCopyROI functions internally. 
      It copies / converts ROI image data to another images ROI using the
      icl::copy(..) function line by line or, in case of IPP-optimization enabled,
      corresponding ippCopy/ippConvert Calls (see the following specialized template 
      functions also).
      @param src source image
      @param srcC source image channel
      @param srcOffs source images ROI-offset (src->getROIOffset() is <b>not</b> regarded)
      @param srcSize source images ROI-size (src->getROISize() is <b>not</b> regarded)
      @param dst destination image      
      @param dstC destination image channel
      @param dstOffs destination images ROI-offset (dst->getROIOffset() is <b>not</b> regarded)
      @param dstSize destination images ROI-size (dst->getROISize() is <b>not</b> regarded)
      @param dstSize destination images ROI-size (dst->getROISize() is <b>not</b> regarded)
   **/
  template <class S,class D>
  inline void convertChannelROI(const Img<S> *src,int srcC, const Point &srcOffs, const Size &srcROISize,
                                Img<D> *dst,int dstC, const Point &dstOffs, const Size &dstROISize)
  {
    FUNCTION_LOG("");
    CHECK_VALUES(src,srcC,srcOffs,srcROISize,dst,dstC,dstOffs,dstROISize);
    
    const ImgIterator<S> itSrc(const_cast<S*>(src->getData(srcC)),src->getSize().width,Rect(srcOffs,srcROISize));
    ImgIterator<D> itDst(dst->getData(dstC),dst->getSize().width,Rect(dstOffs,dstROISize));
    const ImgIterator<S> itSrcEnd = ImgIterator<S>::create_end_roi_iterator(src,srcC,Rect(srcOffs,srcROISize));
    for(;itSrc != itSrcEnd ;itSrc.incRow(),itDst.incRow()){
      icl::convert<S,D>(&*itSrc,&*itSrc+srcROISize.width,&*itDst);
    }
  }
 
  /// @}

  /* }}} */

  /* {{{   scaledCopyChannelROI */

  /// @{ @name scaling of channel ROIs 
  /// scales an image channels ROI into another images ROI (with implicit type conversion) (IPP-OPTIMIZED) \ingroup IMAGE
  /** This function provides all necessary functionalities for scaling images. Please regard, that the fallback-
      implementation is very slow. Only scaling operations with identical source and destination type 
      is optimized by corresponding ippResize calls (see also the specialized template functions).
      @param src source image
      @param srcC source image channel
      @param srcOffs source images ROI-offset (src->getROIOffset() is <b>not</b> regarded)
      @param srcSize source images ROI-size (src->getROISize() is <b>not</b> regarded)
      @param dst destination image      
      @param dstC destination image channel
      @param dstOffs destination images ROI-offset (dst->getROIOffset() is <b>not</b> regarded)
      @param dstSize destination images ROI-size (dst->getROISize() is <b>not</b> regarded)
      @param eScaleMode scaling mode to use (nearest neighbor, linear, or region-average)
   **/
  template<class T> 
  void scaledCopyChannelROI(const Img<T> *src,int srcC, const Point &srcOffs, const Size &srcSize,
                            Img<T> *dst,int dstC, const Point &dstOffs, const Size &dstSize,
                            scalemode eScaleMode);





  /* }}} */

  /* {{{   flippedCopyChannelROI */

  /// mirror copy ROI data from one image to the ROI of another image (IPP-OPTIMIZED) \ingroup IMAGE
  /** This function is used by flippedCopyROI and Mirror operator.
      @param eAxis mirror axis (axisHorz, axisVert or axisBoth)
      @param src source image
      @param srcC source image channel
      @param srcOffs source images ROI-offset (src->getROIOffset() is <b>not</b> regarded)
      @param srcSize source images ROI-size (src->getROISize() is <b>not</b> regarded)
      @param dst destination image      
      @param dstC destination image channel
      @param dstOffs destination images ROI-offset (dst->getROIOffset() is <b>not</b> regarded)
      @param dstSize destination images ROI-size (dst->getROISize() is <b>not</b> regarded)
      @param dstSize destination images ROI-size (dst->getROISize() is <b>not</b> regarded)
   **/
  template <class T>
  void flippedCopyChannelROI(axis eAxis,
                             const Img<T> *src,int srcC, const Point &srcOffs, const Size &srcSize,
                             Img<T> *dst,int dstC, const Point &dstOffs, const Size &dstSize);
  
  
  /// mirror copy of an image from source to destination image (1:1 copy) \ingroup IMAGE
  /** This function creates a flipped instance of this image. Even the ROI is flipped internally.
      Example:
      <pre>
        ......                    ......     r = roi
        ..rrrr  -> flipped x ->   rrrr..
        ..rrrr                    rrrr..
      </pre>
      
      @param eAxis axis to flip
      @param poSrc source image
      @param ppoDst image. This image is exploited if possible. It is adjusted to 
                        the source image in depth, size,channels,format,and time
      
   **/
  void flippedCopy(axis eAxis, const ImgBase *poSrc, ImgBase **ppoDst=0);

  /// mirror copy of an images ROI into a destination images ROI \ingroup IMAGE
  /** Example:
      <pre>
        ......                    ......    R,r = roi
        ..RRrr  -> flipped x ->   rrRR..
        ..RRrr                    rrRR..
      </pre>
      
      @param eAxis axis to flip
      @param poSrc source image
      @param ppoDst destination image (expoited if possible). This images
                    ROI size must be equal to ppoDst's ROI size otherwise 
                    an errormessage is shown, and the function returns 0.
                    If ppoDst is null, a new image is created with size of 
                    this images ROI size. If ppoDst points to NULL, the new
                    image is created at *ppoDst.
      @return flippedCopy      
  **/
  void flippedCopyROI(axis eAxis, const ImgBase *poSrc, ImgBase **ppoDst=0);
  /* }}} */

  /* }}} */ 

} //namespace icl

#undef CHECK_VALUES

#endif //Img_H
