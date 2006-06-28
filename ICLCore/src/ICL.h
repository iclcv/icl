 /*
  ICL.h

  Written by: Michael Götting and Christof Elbrechter (2006)
              University of Bielefeld
              AG Neuroinformatik
              {mgoettin,celbrech}@techfak.uni-bielefeld.de
*/

#ifndef ICL_H
#define ICL_H

//-------- includes --------
#include "ICLBase.h"
#include "ICLChannel.h"
#include <cmath>
#include <typeinfo>

//-------- namespace --------
using namespace std;

//---- ICL in its own namespace ----
namespace ICL {

//-------- ICL class definition --------
/**
   @short ICL implements an array of ICLChannel images with an arbitrary number 
          of image channels.
   @author Michael Goetting (mgoettin@TechFak.Uni-Bielefeld.de) 
   @author Christof Elbrechter (celbrech@TechFak.Uni-Bielefeld.de)
**/
template <class Type>
class ICL : public ICLBase
{
 protected:
  /// internally used type for the channel vector
  typedef typename ICLChannel<Type>::AutoPtr ICLChannelPtr;
  
  /// internal used storage for the image channels
  vector<ICLChannelPtr> m_ppChannels;
  
  /* {{{ Auxillary function */

  /// delete channles internally (for reference counting)
  void deleteChannels()
    {
      m_ppChannels.clear();
    }
  
  /** Bilinear interpolation for 'subpixel' values.
      @param fX X coordinate of the pixel
      @param fY Y coordinate of the pixel
      @param iChannel Channel index (default 0)
      @return Value of the specified point
  **/
  Type interpolate(float fX,float fY,int iChannel=0) const;
  
  //
  /** Check if the width and height and channel of a given ICL is equal
      @param iNewWidth compare value for the image width
      @param iNewHeight compare value for the image height
      @param iNewNumChannels compare value for the channel count
      @return Return 1 if the 2 ICL objects are equal otherwise 0
              
  **/
  int isEqual(int iNewWidth, int iNewHeight, int iNewNumChannels) const;
  
/* }}} */

 public:
  /* {{{ Konstructors / Destructors: */

  //@{
  /// Creates an image with specified number of channels and size.    
  /** the format of the image will be set to "iclMatrix"
      @param iWidth Width of image
      @param iHeight Height of image
      @param iChannels Number of Channels 
  **/
  ICL(int iWidth=1, int iHeight=1, int iChannels=1);
 
  /// Creates an image with specified number of channels and size.    
  /** the format of the image will be set to "iclMatrix"
      @param iWidth Width of image
      @param iHeight Height of image
      @param iChannels Number of Channels 
  **/
  
  /// Creates an image with specified size, number of channels and format
  /** @param iWidth width of the image
      @param iHeight height of the image
      @param eFormat (color)-format of the image
      @param iChannels channel count of the image (if -1, then the channel
                       count is calculated from the given format (E.g. if
                       eFormat is formatRGB iChannels is set to 3)
  **/
  ICL(int iWidth, int iHeight, iclformat eFormat, int iChannels = -1);
 
  /// Creates an image with specified size, number of channels, format, using shared data pointers as channel data
  /** @param iWidth width of the image
      @param iHeight height of the image
      @param eFormat (color)-format of the image
      @param iChannels channel count of the image (if -1, then the channel
                       count is calculated from the given format (E.g. if
                       eFormat is formatRGB iChannels is set to 3)
      @param pptData holds a pointer to channel data pointers. pptData must
                     have size iChannels. The data must not be deleted during
                     the "lifetime" of the ICL.
  **/
  ICL(int iWidth, int iHeight, iclformat eFormat, int iChannels, Type** pptData);

  /// Copy constructor
  /** creates a flat copy of the source images
      the new image will contain a flat copy of 
      all channels of the source image.
      @param tSrc Reference of instance to copy 
  **/
  ICL(const ICL<Type>& tSrc);
  
  
  /// Destructor
  ~ICL();
  
  //@}

  /* }}} */
                                      
  /* {{{ class operator */

  //@{ @name class operators

  /// Assign operator (flat copy of channels)
  /** Both images will share their channel data. 
      Use deepCopy() to obtain a copy of an image which is not attached to the 
      source image.      
      @param tSource Reference to source object. 
  **/
  ICL<Type>& operator=(const ICL<Type>& tSource);

  /// function-operator (provides access to the pixel data)
  /** This operator has to be used, to access the pixeldata of the image
      e.g. copy of image data:
      <pre>
      ICL8u oA(320,240,1),oB(320,240,1);
      for(int x=0;x<320;x++){
         for(int y=0;y<240;y++){
            oB(x,y,0)=oA(x,y,0);
         }
      }
      </pre>
      @param iX X-Position of the referenced pixel
      @param iY Y-Posttion of the referenced pixel
      @param iChannel channel index
  **/
  inline
  Type& operator()(int iX, int iY, int iChannel) const;

  //@}

  /* }}} */
  
  /* {{{ moving / scaling image data */
  //@{ //@name moving / scaling image data

  /// perform a deep copy into an optional destination image)
  /** Returns an independent exact copy of the object. 
      @param poDst Destination image for the copied data 
                   if NULL, then a new image is created and retuned
      @return Pointer to new independent ICL object
  **/
  ICL<Type>* deepCopy(ICL<Type>* poDst = NULL) const;

  /// _s_m_a_r_t_ copy of the image data (scaling on demand)
  /** the smart copy function performes a deep copy of the image data
      into another image. If the given image is not NULL, it's size
      is taken to calculate a scaling factor to scale the image into
      the destination. 
      If the count of channels of this image and the destination image
      do not match, then the scaling operation is only performed
      on the minimum of both images channel counts.
      @param poDst destination image (if NULL) than it is created new with
                   with identical size of this image.
      @param eScaleMode defines the interpolation mode, that is used for the scaling
                        operation. (if interpolateNon, the image becomes black)
                        possible modes are:
                           - interpolateNon --> no interpolation (image will become black)
                           - interpolateNN  --> nearest neightbour interpolation (fastest)
                           - interpolateBL  --> bilinear interpolation
                           - interpolateAV  --> region avarage 
      @see iclscalemode
      @see resize
      @see deepCopy
  **/
  ICL<Type>* smartCopy(ICL<Type> *poDst, iclscalemode eScaleMode=interpolateNN) const;
  
                  
  /* }}} */

  /* {{{ class organisation / channel management */

  //@{ //@name organisation and channel management

  
  
  /// Makes the image channels inside the ICL independent from other ICL.
  /** @param iIndex index of the channel, that should be detached.
      (If iIndex is an legal channel index only the corresponding channel will 
      be detached. If the legal index channel is set to -1 the whole ICL 
      becomes independent.)
  **/
  void detach(int iIndex = -1);
  
  /// Removes a specified channel.
  /** @param iChannel Index of channel to remove
  **/
  void removeChannel(int iChannel);
  
  /// Append channels of external ICL to the existing ICL. 
  /** Both objects will share their data (cheap copy). 
      @param poSrc source ICL<Type>
  **/
  void appendICL(ICL<Type>* poSrc);
  
  /// Appends the channel iChannel of an external image to the current image
  /** Both objects will share their data (cheap copy). 
      @param iChannel Channel index to append
      @param poSrc ICL<Type> that contains the source channel 
  **/
  void appendChannel(int iChannel, 
                     ICL<Type> *poSrc);

  /// Swap channel A and B
  /** @param iIndexA Index of channel A;
      @param iIndexB Index of channel B
  **/
  void swapChannels(int iIndexA, int iIndexB);
  
  /// Replace the channel A of this image with the channel B another image. 
  /** Both images must have the same width and height.
      @param iThisIndexA Channel to replace
      @param iOtherIndexB Channel to replace with
      @param poOtherICL Image pointer that contains the new channel
  **/
  void replaceChannel(int iThisIndexA, 
                      int iOtherIndexB, 
                      ICL<Type>* poOtherICL);

  /// sets the channel count to a new value
  /** This function works only on demand, that means, that
      channels will only be created/deleted, if the new 
      channel count differs from the current.
      @param iNewNumChannels new channel count
      @see resize
  **/
  void setNumChannels(int iNewNumChannels);

 
 
  /// creates a hole new ICL internally
  /** Change the number of ICL channels and the size. The channels have all the
      same width and height. All the data within the ICL will be lost. Already
      shared data will be detached before renew the ICL structure.
      @param iNewWidth New image width
      @param iNewHeight New image height
      @param iNewNumChannel New channel number 
  **/
  void renewICL(int iNewWidth, int iNewHeight,int iNewNumChannel);

  /// resizes the image to new values
  /** operation is performed on demand - if image
      has already size iNewWidth,iNewHeight, then
      the image values are set to 0 only. For resizing
      operation with scaling of the image data use scale
      @param iNewWidth new image width
      @param iNewHeight new image height
      @see scale
  **/
  void resize(int iNewWidth, int iNewHeight);
  
  
  //@}

  /* }}} */

  /* {{{ Type conversion iclbyte/iclfloat */
  //@{ @name type conversion
  
  /// Return a copy of the object with depth 32 bit.  
  /** @param poDst destination image (if NULL, then a new image is created) 
      @return Copy of the object with depth 32 bit 
  **/
  ICL32f *convertTo32Bit(ICL32f* poDst = NULL) const ;
  
  /// Return a copy of the object with depth 8 bit
  /** Waring: Information may be lost!
      @param poDst destination image (if NULL, then a new image is created) 
      @return Copy of the object with depth 8 bit 
  **/
  ICL8u *convertTo8Bit(ICL8u* poDst = NULL) const;

   //@}
  /* }}} */

  /* {{{ Setter functions: */
  //@{ @name setter functions
  
  /// sets the ROI (region of interests) to a specified rect
  /** @param iX X-Offset of the ROI
      @param iY Y-Offset of the ROI
      @param iWidth width of the ROI
      @param iHeight height of the ROI
  **/
  void setROI(int iX, int iY,int iWidth,int iHeight);
  
  /// set the ROI- (region of interests) offset to a specified point
  /** @param iX X-Offset of the ROI
      @param iY Y-Offset of the ROI
  **/
  void setROIOffset(int iX, int iY);
  
  /// set the ROI- (region of interests) size to a specified dimension
  /** @param iWidth width of the ROI
      @param iHeight height of the ROI      
  **/
  void setROISize(int iWidth, int iHeight);
  
  //@}

/* }}} */

  /* {{{ Getter Functions */

  //@{ @name getter functions
  
  /// Returns max pixel value of channel iChannel
  /** @param iChannel Index of channel
  **/
  Type getMax(int iChannel) const
    {
      return m_ppChannels[iChannel]->getMax();
    }
  
  /// Returns min pixel value of channel iChannel
  /** @param iChannel Index of channel 
  **/
  Type getMin(int iChannel) const
    {
      return m_ppChannels[iChannel]->getMin();
    }
  
  /// Returns pointer to the specified channel data
  /** This method provides
      direct access to the channel data memory.
      @param iChannel Channel to get data from
  **/
  Type* getData(int iChannel) const
    { 
      return (m_ppChannels[iChannel]->getDataBegin());
    }

  /// return the raw- data pointer of an image channel
  /** This function is inherited from the base class ICLBase
      @param iChannel determines the channel which's dataptr should
                      be returned
  **/
  virtual void* getDataPtr(int iChannel) const
    {
      return (m_ppChannels[iChannel]->getDataBegin());
    }
  
  /// Gets the ROI (region of interests) of this image
  /** @param riX reference to store the x value of the roi
      @param riY reference to store the y value of the roi
      @param riWidth reference to store the width value of the roi
      @param riHeight reference to store the height value of the roi
  **/
  void getROI(int &riX, int &riY, int &riWidth, int &riHeight) const;

  /// Gets the ROI- (region of interests) offset of this image
  /** @param riX reference to store the x value of the roi
      @param riY reference to store the y value of the roi
  **/
  void getROIOffset(int &riX, int &riY) const;
  
  /// Gets the ROI- (region of interests) size of this image
  /** @param riWidth reference to store the width value of the roi
      @param riHeight reference to store the height value of the roi
  **/
  void getROISize(int &riWidth, int &riHeight) const;

  //@}

  /* }}} */
  
  /* {{{ basic image manipulation: */

  //@{ @name basic image manipulations

  /// perform a smart resize operation of the images 
  /** Optional it allows to keep the image
      data during this information, which means, that the channels
      are scaled internally.
      Scaling the channels is only perfomed on demand.
      @param iNewWidth destination width for the scaling operation 
                       (if set to -1 then the original width is used)
      @param iNewHeight destination height for the scaling operation
                       (if set to -1 then the original height is used)
      @param eScaleMode defines the interpolation mode, that is used for the scaling
                          operation. (if interpolateNon, the image becomes black)
                          possible modes are:
                           - interpolateNon --> no interpolation (image will become black)
                           - interpolateNN  --> nearest neightbour interpolation (fastest)
                           - interpolateBL  --> bilinear interpolation
                           - interpolateAV  --> region avarage 
      @see iclscalemode
 
  **/
  void scale(int iNewWidth, int iNewHeight, iclscalemode eScaleMode=interpolateNN);
  
  /// Sets the pixels of one or all channels to a specified value
  /** @param tValue destination value
      @param iChannel Channel to fill with zero (default: -1 = all)
   **/
  void clear(int iChannel = -1, Type tValue = 0);
  
  /// Scale the channel min/ max range to the new range tMin, tMax. 
  /** @param fMin new mininum value for the channel
      @param fMax new maximum value for the channel
      @param iChannel channel index (if set to -1, then operation is 
                      performed on all channels)
  **/
  void scaleRange(float fMin=0.0, float fMax=255.0, int iChannel = -1);

  //@}

/* }}} */

}; // class

} //namespace ICL

#endif //ICL_H

