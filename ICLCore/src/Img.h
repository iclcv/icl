 /*
  Img.h

  Written by: Michael Götting and Christof Elbrechter (2006)
              University of Bielefeld
              AG Neuroinformatik
              {mgoettin,celbrech}@techfak.uni-bielefeld.de
*/

#ifndef Img_H
#define Img_H

#include "ImgI.h"
#include "ImgIterator.h"
#include "SmartPtr.h"
#include <cmath>
#include <algorithm>

using namespace std;

namespace icl {

  /// The Img class implements the ImgI Image interface with type specific functionalities
  /**
  @author Michael Goetting (mgoettin@TechFak.Uni-Bielefeld.de) 
  @author Christof Elbrechter (celbrech@TechFak.Uni-Bielefeld.de)
  **/
template <class Type>
class Img : public ImgI
{
 protected:
  
  // @{ @name data
  /* {{{ open */
  /// internally used storage for the image channels
  vector<SmartPtr<Type> > m_vecChannels;
  // @}
  /* }}} */

  // @{ @name auxillary functions
  /* {{{ open */

                  
                  
  /** Bilinear interpolation for 'subpixel' values.
      @param fX X coordinate of the pixel
      @param fY Y coordinate of the pixel
      @param iChannel Channel index (default 0)
      @return Value of the specified point
  **/
  Type interpolate(float fX,float fY,int iChannel=0) const;

  /// creates a new deep copy of a specified Type*
  /** if the give Type* ptDataToCopy is not NULL, the data addressed from it, 
      is copied deeply into the new created data pointer
  **/
  SmartPtr<Type> createChannel(Type *ptDataToCopy=0) const;

  /// returns the start index for a channel loop
  /** In many functions like deleteChannel, clear or scaleRange, 
      it has to be swiched over two cases:
      - if given channel index is -1, then it has to be iterated over all
        image channels
      - else only the given image channel has to be touched
      
      To avoid code doublication, one can use the following for-loop
      <pre>
      void foo(int iChannel){
         for(int i = iIndex < 0 ? 0 : iIndex, iEnd = iIndex < 0 ? m_iChannels : iIndex+1;
             i < iEnd; 
             i++) 
         {
             // do something
         }
      </pre>
      When using the get<Start|End>Index functions the loop becomes much more
      readable:
      <pre>
      void foo(int iChannel){
         for(int i=getStartIndex(iIndex), iEnd=getEndIndex(iIndex); i<iEnd ;i++){
         {
             // do something
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
  // @}
  /* }}} */
                                
 public:
  // @{ @name constructors / destructor
  /* {{{ open */

  /// Creates an image with specified number of channels and size.    
  /** the format of the image will be set to "iclMatrix"
      @param s size of the new image
      @param iChannels Number of Channels 
  **/
  Img(const Size &s, int iChannels=1);
 
  /// Creates an image with specified size, number of channels and format
  /** @param s size of the new image
      @param eFormat (color)-format of the image
      @param iChannels channel count of the image (if -1, then the channel
                       count is calculated from the given format (E.g. if
                       eFormat is formatRGB iChannels is set to 3)
  **/
  Img(const Size &s, format eFormat, int iChannels = -1);
 
  /// Creates an image with specified size, number of channels, format, using shared data pointers as channel data
  /** @param s size of the new image
      @param eFormat (color)-format of the image
      @param iChannels channel count of the image (if -1, then the channel
                       count is calculated from the given format (E.g. if
                       eFormat is formatRGB iChannels is set to 3)
      @param pptData holds a pointer to channel data pointers. pptData must
                     have size iChannels. The data must not be deleted during
                     the "lifetime" of the Img. Call detach after the 
                     constructor call, to induce the Img to allocate own memory
                     for the image data.
  **/
  Img(const Size &s, format eFormat, int iChannels, Type** pptData);

  /// Copy constructor
  /** creates a flat copy of the source image
      the new image will contain a flat copy of 
      all channels of the source image.
      @param tSrc Reference of instance to copy 
  **/
  Img(const Img<Type>& tSrc);
  
  
  /// Destructor
  ~Img();
  
  //@}
  /* }}} */
  
  //@{ @name operators
  /* {{{ open */

  /// Assign operator (flat copy of channels)
  /** Both images will share their channel data. 
      Use deepCopy() to obtain a copy of an image which is not attached to the 
      source image.      
      @param tSource Reference to source object. 
  **/
  Img<Type>& operator=(const Img<Type>& tSource);

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
      faster. @see ImgIterator @see getIterator() @see getROIIterator()
  
      @param iX X-Position of the referenced pixel
      @param iY Y-Position of the referenced pixel
      @param iChannel channel index
  **/
  Type& operator()(int iX, int iY, int iChannel) const
    {
      return getData(iChannel)[iX+m_oSize.width*iY];
    }

  //@}
  /* }}} */
  
  //@{ @name moving / scaling image data
  /* {{{ open  */

  /// perform a deep copy (given destination image is resized on demand) (IPP-OPTIMIZED)
  /** Returns an independent exact copy of the object (except for the depth, which
      is fixed if poDst is not NULL). If the given destination
      image is not NULL, then deepCopy ensures, that is has a compatible
      size to this, by resizing destination image on demand. 
      <b>WARNING:</b> If the destination image has another depth, then this image,
      the deepCopy will internally call <b>convertTo8<dest-depth>(poDst)</b>.
     <b>The images ROI will not be regarded</b>, to copy just the ROI 
      into another image use deepCopyROI(ImgI *poDst.) or 
      scaledCopyROI(ImgI *poDst).
      @param poDst Destination image for the copied data 
                   if NULL, then a new image is created and returned
      @return Pointer to new independent Img object
  **/
  virtual ImgI* deepCopy(ImgI* poDst = NULL) const;

  /// returns a scaled copy of the image data (scaling on demand) (IPP-OPTIMIZED)
  /** the function performs a deep copy of the image data
      into another image. If the given image is not NULL, its size
      is taken to calculate a scaling factor to scale the image into
      the destination. 
      If the count of channels of this image and the destination image
      do not match, then count of destination image channels will be
      adapted to this' count of image channels.
      <b>WARNING:</b> If the destination image has another depth than 
      the image, then internally a temporary buffer is created, to scale 
      and convert the image in two steps. This will hardly <b>slow down 
      performace</b>.
      <b>The images ROI will not be regarded</b>, to copy just the ROI 
      into another image use deepCopyROI(ImgI *poDst.) or 
      scaledCopyROI(ImgI *poDst).
      @param poDst destination image (if NULL) than it is created new with
                   identical size of this image.
      @param eScaleMode defines the interpolation mode, that is used for the scaling
                        operation.
                        possible modes are:
                           - interpolateNN  --> nearest neighbor interpolation (fastest)
                           - interpolateLIN  --> bilinear interpolation
                           - interpolateRA  --> region average 
      @see scalemode
      @see resize
      @see deepCopy
  **/
  virtual ImgI* scaledCopy(ImgI *poDst, scalemode eScaleMode=interpolateNN) const;
  
  /// copies the image data in the images ROI into the destination images ROI (IPP-OPTIMIZED)
  /** This function will copy the content of the images ROI into the
      destination images ROI. The ROIs must have equal dimensions - if not, then then poDst 
      is returned imediately without performing any operation, and a warning is written to 
      std::out. If the given destination image is NULL (by default),
      deepCopyROI will create a new image, with identical channel count, depth and
      format. This image has the size of the source images ROI-size, and will contain
      the source images' ROI data.

      <h3>Efficiency</h3>
      The copy calls of the ROIs are performed line by line, using <b>memcpy</b>.
      This will speedup performance for huge ROIs (width>100) by up to 50%. 

      <h3>Depth conversion</h3>
      The deepCopyROI function supports IPP-OPTIMIZED depth conversion.
      
  **/
  virtual ImgI *deepCopyROI(ImgI *poDst = NULL) const;
  /// deep copy a single channel from source ROI to destination ROI
  virtual void  deepCopyROI(ImgI *poDst, int iSrcChannel, int iDstChannel) const;
  
  /// scales the image data in the image ROI into the destination images ROI (IPP-OPTIMIZED)
  /** This function copies ROI data from one image into the ROI of another one. If the source
      and destination ROIs have different sizes the moved data is scaled.
      If the destination image is NULL or the ROIs have identical sized, deepCopyROI is called.
  
      <h3>Efficiency</h3>
      The IPP-OPTIMIZED implementation is <b>very fast</b> - as the used ippResize-function
      uses as well source ROI as destination ROI by default. <b>Note</b> that the non
      OPTIMIZED function is just a fallback implementation to provided identical functionality
      to the optimized implementation which uses a temporary image buffer, to perform the operation.
      @param poDst destination image (if NULL) than it is created new with
                   identical size of this images ROI size.
      @param eScaleMode defines the interpolation mode, that is used for the scaling
                        operation.
                        possible modes are:
                           - interpolateNN  --> nearest neighbor interpolation (fastest)
                           - interpolateLIN  --> bilinear interpolation
                           - interpolateRA  --> region average 
  **/
  virtual ImgI *scaledCopyROI(ImgI *poDst = NULL, scalemode eScaleMode=interpolateNN) const;
                  
  /* }}} */
  
  //@{ @name organization and channel management                                       
  /* {{{ open */
  
  /// Makes the image channels inside the Img independent from other Img.
  /** @param iIndex index of the channel, that should be detached.
      (If iIndex is an legal channel index only the corresponding channel will 
      be detached. If the legal index channel is set to -1 the whole Img 
      becomes independent.)
  **/
  virtual void detach(int iIndex = -1);
  
  /// Removes a specified channel.
  /** @param iChannel Index of channel to remove
  **/
  virtual void removeChannel(int iChannel);
  
  /// Append channels of external Img to the existing Img. 
  /** Both objects will share their data (cheap copy). 
      @param poSrc source image
      @param iChannel channel to append (or all, if < 0)
  **/
  void append(Img<Type> *poSrc, int iChannel=-1);
  
  /// Swap channel A and B
  /** @param iIndexA Index of channel A;
      @param iIndexB Index of channel B
  **/
  virtual void swapChannels(int iIndexA, int iIndexB);
  
  /// Replace the channel A of this image with the channel B another image. 
  /** Both images must have the same width and height.
      @param iThisIndex channel to replace
      @param iOtherIndex channel to replace with
      @param poOtherImg Image that contains the new channel
  **/
  void replaceChannel(int iThisIndex, Img<Type> *poOtherImg, int iOtherIndex);

  /// sets the channel count to a new value
  /** This function works only on demand, that means, that
      channels will only be created/deleted, if the new 
      channel count differs from the current.
      @param iNewNumChannels new channel count
      @see resize
  **/
  virtual void setNumChannels(int iNewNumChannels);

  /// creates a hole new Img internally
  /** Change the number of Img channels and the size. The function works
      on demand: If the image has already the correct size and number of
      channels, nothing is done at all. This allows you to call renew ()
      always when you want to ensure a specific image size.
      If the size must be adapted: 
      <b> All the data within the Img will be lost. </b> 
      @param s new image size  (if x or y is < 0, the orignal width/height is used)
      @param iNewNumChannel New channel number (if < 0, the orignal 
             channel count is used)
  **/
  virtual void renew(const Size &s, int iNewNumChannel);

  /// resizes the image to new values
  /** operation is performed on demand - if image
      has already size iNewWidth,iNewHeight, then
      nothing is done at all. For resizing
      operation with scaling of the image data use scale.
      <b>Note:</b> The ROI of the image is set to the hole
      image using delROI(), notwithstanding if a resize
      operation was performed or not.
      @param s new image size  (if x or y is < 0, the orignal width/height is used)
      @see scale
  **/
  virtual void resize(const Size &s);
  
  
  //@}

  /* }}} */
  
  //@{ @name getter functions                                                   
  /* {{{ open */

  /// Returns max pixel value of channel iChannel (IPP-OPTIMIZED)
  /** @param iChannel Index of channel (if -1 then the maximum 
                      of all channels is calculated)
  **/
  Type getMax(int iChannel=-1) const;
  
  /// Returns min pixel value of channel iChannel (IPP-OPTIMIZED)
  /** @param iChannel Index of channel (if -1 then the minimum 
                      of all channels is calculated)
  **/
  Type getMin(int iChannel=-1) const;
  
  /// Returns min and max pixel value of channel iChannel (IPP-OPTIMIZED)
  /** @param rtMin reference to store the min value 
      @param rtMax reference to store the max value
      @param iChannel Index of channel (if -1 then the maximum and 
                      then minium of all channels is calculated)
     
  **/
  void getMinMax(Type &rtMin, Type &rtMax, int iChannel=-1) const;

  /// Returns the width of an image line in bytes
  virtual int getLineStep() const{
    return getSize().width*sizeof(Type);
  }

  /// returns a Type save data data pointer to the channel data origin
  /** If the channel index is not valid (<0 or >= getChannels) NULL 
      is returned and an error is written to std::err
      @param iChannel specifies the channel 
      @return data origin pointer to the specified channel 
  */
  Type* getData(int iChannel) const
    { 
      FUNCTION_LOG("");
      ICLASSERT_RETURN_VAL( iChannel >= 0 , 0);
      ICLASSERT_RETURN_VAL( iChannel < getChannels() , 0);
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
  */
  Type *getROIData(int iChannel) const{
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL( iChannel >= 0 ,0);
    ICLASSERT_RETURN_VAL( iChannel < getChannels() ,0);
    return getData(iChannel) + m_oROIOffset.x + (m_oROIOffset.y * m_oSize.width);
  }

  /// returns the data pointer to a pixel with defined offset
  /** In some functions like filters, it might be necessary to change the images
      ROI parameters before applying the underlying image operation. 
      Temporarily changing the images ROI parameters causes problems in
      multi-threaded environments. To avoid this, this function provides
      access to a data pointer to an abitrary notional ROI-offset
      @param iChannel selects the channel
      @param p notional ROI offset
      @return data pointer with notional ROI offset p
  **/
  Type *getROIData(int iChannel, const Point &p) const{
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL( iChannel >= 0 ,0);
    ICLASSERT_RETURN_VAL( iChannel < getChannels() ,0);
    return getData(iChannel) + p.x + (p.y * m_oSize.width);
  }


  /// return the raw- data pointer of an image channel
  /** This function is inherited from the base class ImgI
      @param iChannel determines the channel which's dataptr should
                      be returned
      @return raw data pointer
  **/
  virtual void* getDataPtr(int iChannel) const
    {
      FUNCTION_LOG("");
      ICLASSERT_RETURN_VAL( iChannel >= 0 ,0);
      ICLASSERT_RETURN_VAL( iChannel < getChannels() ,0);
      return getData(iChannel);
    }
  
  //@}
  /* }}} */
  
  //@{ @name basic image manipulations
  /* {{{ open */

  

  /// perform a scaling operation of the images (keeping the data) (IPP-OPTIMIZED)
  /** Scaling the channels is only performed on demand.
      @param s size of the destination image, if s.width is -1, then only the height
               of the image is adapted. The same is valid for s.height.
      @param eScaleMode defines the interpolation mode, that is used for the scaling
                          operation. (if interpolateNon, the image becomes black)
                          possible modes are:
                           - interpolateNN   --> nearest neighbor interpolation (fastest)
                           - interpolateLIN  --> bilinear interpolation
                           - interpolateRA   --> region average 
      @see scalemode
      @see resize
  **/
  virtual void scale(const Size &s, scalemode eScaleMode=interpolateNN);
 
 
  
  /// Sets the pixels of one or all channels to a specified value
  /** @param iChannel Channel to fill with zero (default: -1 = all channels)
      @param tValue destination value (default: 0)
   **/
  void clear(int iChannel = -1, Type tValue = 0);
  
  /// Scale the channel min/ max range to the new range tMin, tMax.
  /** @param fMin new mininum value for the channel
      @param fMax new maximum value for the channel
      @param iChannel channel index (if set to -1, then operation is 
                      performed on all channels)
  **/
  virtual void scaleRange(float fMin=0.0, float fMax=255.0, int iChannel = -1); 

  /// Scales pixel values from given min/max values to new min/max values.
  /** Values exceeding the given range are set to the new min/max values.
      For an automatic scaling use the results of  min(),max() as as arguments.
      (Defining a range allows to compare different images.)
      @param tNewMin destination minimum value
      @param tNewMax destination maximum value
      @param tMin current minimum value
      @param tMax current maximum value
      @param iChannel channel index (if set to -1, then operation is 
                      performed on all channels)
  **/
  virtual void scaleRange(float tNewMin, float tNewMax, float tMin, float tMax, int iChannel=-1);


  //@}

/* }}} */
  
  //@{ @name pixel access using roi iterator                                                                     
  /* {{{ open */

  /// type definition for roi iterator
  typedef ImgIterator<Type> iterator;

  /// returns the iterator for the hole image 
  /** The following example taken from ImgIterator.h will show
      the iterator usage:
      <pre>
      void channel_convolution_3x3(Img32f &src, Img32f &dst,icl32f *pfMask, int iChannel)
      { 
         for(Img32f::iterator s=src.begin(iChannel) d=dst.begin() ; s.inRegion() ; s++,d++)
         {
            icl32f *m = pfMask;
            (*d) = 0;
            for(Img32f::iterator sR(s, 3, 3); sR.inRegion(); sR++,m++)
            {
               (*d) += (*sR) * (*m);
            }
         }  
      }
      </pre>
      <b>Note:</b> The performance hints illustrated in the
      ImgIterator documentation.
      @param iChannel selected channel index
      @return iterator
      @see ImgIterator
      @see end
  */
  inline iterator getIterator(int iChannel)
    {
      FUNCTION_LOG("begin(" << iChannel << ")");
      ICLASSERT_RETURN_VAL(iChannel >=0 , iterator());
      ICLASSERT_RETURN_VAL(iChannel < getChannels() ,iterator());
      return iterator(getData(iChannel),m_oSize.width,Rect(Point(0,0),m_oSize));
    }
  /// returns an iterator to an images ROI pixles
  /** this function behaves essentially like the above function 
      @param iChannel selects a channel
      @return roi-iterator
      @see getIterator
  */
  inline iterator getROIIterator(int iChannel)
    {
      FUNCTION_LOG("begin(" << iChannel << ")");
      ICLASSERT_RETURN_VAL(iChannel >=0 , iterator());
      ICLASSERT_RETURN_VAL(iChannel < getChannels() ,iterator());
      return iterator(getData(iChannel),m_oSize.width,getROI());
    } 
 
  //@}
  /* }}} */
                                       
};// class
  
  
} //namespace icl

#endif //Img_H

