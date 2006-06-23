 /*
  ICL.h

  Written by: Michael Götting (2004)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
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
   @short ICL implements an arry of ICLChannel images with an arbitrary number 
          of image channels.
   @author Michael Goetting (mgoettin@TechFak.Uni-Bielefeld.de)
**/
template <class Type>
class ICL : public ICLBase
{
 protected:
  typedef typename ICLChannel<Type>::AutoPtr ICLChannelPtr;
  
 private:
  vector<ICLChannelPtr> m_ppChannels;
  string m_sVarType;
  int m_iChannels;
  
  /* {{{ Auxillary function */
  //--------------------------------------------------------------------------
  /** Delete channels **/
  void deleteChannels()
    {
      //---- Delete channel due to reference counting ----
      m_ppChannels.clear();
    }
  
  //--------------------------------------------------------------------------
  /** Set var type **/
  void setVarType();
  
  //--------------------------------------------------------------------------
  /** Bilinear interpolation for 'subpixel' values.
      @param fX X coordinate of the pixel
      @param fY Y coordinate of the pixel
      @param iChannel Channel index (default 0)
      @return Value of the specified point
  **/
  Type interpolate(float fX, 
                   float fY,
                   int iChannel=0) const;
  
  //--------------------------------------------------------------------------
  /** Check if the width and height of a given ICL is equal
      @param -
      @return Return ICL_TRUE if the 2 ICL objects are equal otherwise
              ICL_FALSE
  **/
  int isEqual(int iNewWidth, int iNewHeight, int iNumNewChannels);
  
/* }}} */

 public:
  /* {{{ Konstruktor/ Destruktor: */

  //@{
  //--------------------------------------------------------------------------
  /** Constructs an image with specified number of channels and size.    
      @param iWidth Width of image
      @param iHeight Height of image
      @param iChannels Number of Channels 
  **/
  ICL(int iWidth=1, int iHeight=1, int iChannels=1);
  
  //--------------------------------------------------------------------------
  /** Constructs an image with specified number of channels. With an individual
      size for each channel.
      @param vecWidth Width of image[i], with i = position of channel
      @param vecHeight Height of image[i], with i = position of channel
  **/
  ICL(vector<int> vecWidth, vector<int> vecHeight);
  
  //--------------------------------------------------------------------------
  /** Copy constructor 
      @param tSrc Reference of instance to copy 
  **/
  ICL(const ICL<Type>& tSrc);
  
  //--------------------------------------------------------------------------
  /** Destructor 
   **/
  ~ICL();
  
  //@}

/* }}} */
                                      
  /* {{{ class operator */

  //@{
  //--------------------------------------------------------------------------
  /** Assign operator. Both images will share their channel data. 
      Use deepCopy() to obtain a copy of an image which is not attached to the 
      source image.      
      @param tSource Reference to source object. 
  **/
  ICL& operator=(const ICL<Type>& tSource);

  //--------------------------------------------------------------------------
  /** operator()
  **/
  inline
  Type& operator()(int iX, int iY, int iChannel);

  //@}

/* }}} */
  
  /* {{{ class organisation : */

  //@{
  //--------------------------------------------------------------------------
  /** Returns an independent exact copy of the object. 
      @param pToICL Copy the current ICL to this object
      @return Pointer to to new independent ICL object
  **/
  ICL<Type>* deepCopy(ICL<Type>* pToICL = NULL);
  
  //--------------------------------------------------------------------------
  /** Makes the image channels inside the ICL independent from other ICL.
      If iIndex is an legal channel index only the corresponding channel will 
      be detached. If the legal index channel is set to -1 the whole ICL 
      becomes independent.
  **/
  void detach(int iIndex = -1);
  
  //--------------------------------------------------------------------------
  /** Removes a specified channel.
      @param iChannel Index of channel to remove
  **/
  void removeChannel(int iChannel);
  
  //--------------------------------------------------------------------------
  /** Append channels of external ICL to the existing ICL. 
      Both objects will share their data (cheap copy). 
      @param pExternal Reference to instance of ICL<Type>
  **/
  void appendICL(const ICL<Type>& pExternal);
  
  //--------------------------------------------------------------------------
  /** Appends the channel iChannel of an external image to the current image
      (cheap copy).
      @param iChannel Channel index to append
      @param srcICL The source channel 
  **/
  void appendChannel(int iChannel, 
                     const ICL<Type>& srcICL);

  //--------------------------------------------------------------------------
  /** Swap channel A and B
      @param iIndexA Index of channel A;
      @param iIndexB Index of channel B
  **/
  void swapChannels(int iIndexA, int iIndexB);

  //--------------------------------------------------------------------------
  /** Change the size of the referenced ICL channel. All data within this 
      channel will be destroyed. Shared channels will be detached 
      automatically before resizing.
      @param iNewWidth New image width
      @param iNewHeight New image height
      @param iChannel Channel index 
  **/
  void resizeChannel(int iNewWidth, int iNewHeight,int iChannel);
  
  //--------------------------------------------------------------------------
  /** Change the number of ICL channels. New channels have all the same width 
      and height. If only the number of channels is incremented, the data of 
      the already existing channels will be remain unchanged and the new 
      channels will be initialized with zeros. If the number of channels is 
      reduced, channels will be deleted (start with the last one) while the 
      data of remaining channels will be unchanged. 
      @param iNewWidth New image width
      @param iNewHeight New image height
      @param iNewNumChannel New channel number 
  **/
  void resizeICL(int iNewWidth, int iNewHeight,int iNumNewChannel);

  //--------------------------------------------------------------------------
  /** Change the number of ICL channels and the size. The channels have all the
      same width and height. All the data within the ICL will be lost. Already
      shared data will be detached before renew the ICL structure.
      @param iNewWidth New image width
      @param iNewHeight New image height
      @param iNewNumChannel New channel number 
  **/
  void renewICL(int iNewWidth, int iNewHeight,int iNumNewChannel);

  //--------------------------------------------------------------------------
  /** Change the number of ICL channels and the size. 
      All the data within the ICL will be lost. Already shared data will be
      detached before renew the ICL structure.
      @param vecNewWidth New image width[i], i = position of channel
      @param vecNewHeight New image height[i], i = position of channel
  **/
  void renewICL(vector<int> vecNewWidth, vector<int> vecNewHeight);
  
  //--------------------------------------------------------------------------
  /** Replace the channel A with the channel B of ICL srcICL. 
      Both images must have the same width and height.
      @param iIndexA Channel to replace
      @param iIndexB Channel to replace with
      @param srcICL  Image object containing rtThatImage
  **/
  void replaceChannel(int iIndexA, 
                      int iIndexB, 
                      const ICL<Type>& srcICL);
  
  //@}

/* }}} */

  /* {{{ Type converter */
  //@{
  //--------------------------------------------------------------------------
  /** Return a copy of the object with depth 32 bit.      
      @return Copy of the object with depth 32 bit 
  **/
  ICL<iclfloat> convertTo32Bit() const ;
  
  //--------------------------------------------------------------------------
  /** Place the iclfloat copy of the image into an existing image.      
      @param tImg The result after the convertion process
      @return -
  **/
  void convertTo32Bit(ICL<iclfloat> &tImg) const ;
  
  //--------------------------------------------------------------------------
  /** Return a copy of the object with depth 8 bit. Information may be lost!
      @return Copy of the object with depth 8 bit 
  **/
  ICL<iclbyte> convertTo8Bit() const;

  //--------------------------------------------------------------------------
  /** Place the iclbyte copy of the image into an existing image.      
      @param tImg The result after the convertion process
      @return -
  **/
  void convertTo8Bit(ICL<iclbyte> &tImg) const;
  
  //@}
/* }}} */

  /* {{{ Set functions: */

  //@{
  //--------------------------------------------------------------------------
  /** Sets the pixel at the specified position in the specified channel
      to a new value.
      @param iX X coordinate of the pixel
      @param iY Y coordinate of the pixel
      @param iChannel Channel index of the point
      @param value New value of the specified point
  **/
  void setPixel(int iX, int iY, int iChannel,Type value);
  
  //--------------------------------------------------------------------------
  /** Sets the pixel at the specified position to a specified value. 
      The value is an array of dimension equal to the number of channels.
      @param iX X coordinate of the pixel
      @param iY Y coordinate of the pixel
      @param pValue Pointer to memory that contains the data to set
  **/
  void setPixel(int iX, int iY, Type* pValue);
  
  //--------------------------------------------------------------------------
  /** Sets the Roi of the specified channel
      @param iChannelRoiWidth Image Roi Width
      @param iChannelRoiHeight Image Roi Height
      @param iChannel Number of Channel to get the Roi from
  **/
  void setChannelRoi(int iChannelRoiWidth, 
                     int iChannelRoiHeight, 
                     int iChannel)
    {
      m_ppChannels[iChannel]->setImageRoi(iChannelRoiWidth, 
                                          iChannelRoiHeight);
    }
  
  //--------------------------------------------------------------------------
  void setChannelRoiOffset(int iXOffset, int iYOffset, int iChannel)
    {
      m_ppChannels[iChannel]->setImageRoiOffset(iXOffset, iYOffset);
    }
  
  //@}

/* }}} */

  /* {{{ Get Functions */

  //@{
  //--------------------------------------------------------------------------
  /** Copy the data from the specified channel to the target stl vector
      @param ptTarget The stl vector
      @param iChannel Copy the data from the channel iChannel
  **/
  void getDataVec(vector<Type> &ptTarget, int iChannel) const;

  //--------------------------------------------------------------------------
  /** Gets the value of one pixel. The value of the pixel is an
      array of dimension equal to the number of channels of the
      image.
      @param iX X coordinate of the pixel
      @param iY Y coordinate of the pixel
      @param ptTarget Pointer to memory that will be filled with the pixel 
      data
  **/   
  void getPixel(int iX, int iY, Type* ptTarget) const;

  //--------------------------------------------------------------------------
  /** Return the pixel value at the specified position in the 
      specified channel.
      @param iX X coordinate of the pixel
      @param iY Y coordinate of the pixel
      @param iChannel Channel index
      @return Value of specified point
  **/
  Type getPixel(int iX, int iY, int iChannel) const
    {
      //---- Get pixel value ----
      return m_ppChannels[iChannel]->getPixel(iX,iY);
    }
  
  //--------------------------------------------------------------------------
  /** Gets a pixel pillar of the channels specified by the mask pbMask. 
      @param iX X coordinate of the pixel
      @param iY Y coordinate of the pixel
      @param pbMask Array (dim = \#channels) of bool values that 
                    select/deselect channels (true = selected, 
                    false = deselected). 
      @param ptTarget Pointer to memory that will be filled with the pixel
                      data
      @return Number of elements filled in ptTarget
  **/   
  int getFilteredPixel(int iX, int iY, bool* pbMask,Type* ptTarget) const;
  
  //--------------------------------------------------------------------------
  /** Returns max pixel value of channel iChannel
      @param iChannel Index of channel
  **/
  Type getMax(int iChannel) const
    {
      //---- Get the maximum pixel value ----
      return m_ppChannels[iChannel]->getMax();
    }
  
  //--------------------------------------------------------------------------
  /** Returns min pixel value of channel iChannel
      @param iChannel Index of channel
  **/
  Type getMin(int iChannel) const
    {
      //---- Get the minimum pixel value ----
      return m_ppChannels[iChannel]->getMin();
    }

  //--------------------------------------------------------------------------
  /** Returns number of channels
      @return Number of image channels
  **/
  int getChannels() const 
    {
      return m_iChannels;
    }

  //--------------------------------------------------------------------------
  /** Returns the height (in pixel) of channel iChannel
      @return The height of channel iChannel
  **/
  int getHeight(int iChannel) const 
    {
      return m_ppChannels[iChannel]->getHeight();
    }

  //--------------------------------------------------------------------------
  /** Returns the height of the ICL stack
      @return The height of all ICL channels
  **/
  vector<int> getHeight() const; 

  //--------------------------------------------------------------------------
  /** Returns the width (in Pixel) of channel iChannel
      @return Number of image channels
  **/
  int getWidth(int iChannel) const 
    {
      return m_ppChannels[iChannel]->getWidth();
    }

  //--------------------------------------------------------------------------
  /** Returns the width of the ICL stack
      @return The width of all ICL channels
  **/
  vector<int> getWidth() const; 
  
  //--------------------------------------------------------------------------
  /** Returns pointer to the specified channel data. This method allows 
      direct access to the channel data memory.
      @param iChannel Channel to get data from
  **/
  //typename vector<Type>::const_iterator getChannelDataPtr(int iChannel) const
  //  { 
  //    return (m_ppChannels[iChannel]->getDataPtrBegin());
  //  }

  //--------------------------------------------------------------------------
  /** Returns pointer to the specified channel data. This method allows 
      direct access to the channel data memory.
      @param iChannel Channel to get data from
  **/
  Type* getData(int iChannel) 
    { 
      return (m_ppChannels[iChannel]->getDataBegin());
    }
  
  //--------------------------------------------------------------------------
  /** Gets the Roi of the specified channel 
      @param piChannelRoiWidth Pointer to Image Roi Width
      @param piChannelRoiHeight Pointer to Image Roi Height
      @param iChannel Number of Channel to get the Roi from
  **/
  void getChannelRoi(int* piChannelRoiWidth, 
                     int* piChannelRoiHeight, 
                     int iChannel);

  //--------------------------------------------------------------------------
  /** Gets the Roi offset of the specified channel 
      @param piXOffset Pointer to XOffset for Roi
      @param piYOffset Pointer to YOffset for Roi
      @param iChannel Number of Channel to get the Roi from
  **/
  void getChannelRoiOffset(int* piXOffset, 
                           int* piYOffset, 
                           int iChannel);
  
  //--------------------------------------------------------------------------
  /** Instead of the  previous version of getRegion(...) this version returns
      the result in the given ICL object. 
      The region is specified by the (x,y)-coordinate in dependency of the 
      center mode and the region's width and height. 
      Note that the region must be completely inside the image.
      @param iCenterMode \li 0: lower left Corner
                         \li 1: center mode
      @param iX x-coordinate
      @param iY y-coordinate
      @param iWidth width of selected region
      @param iHeight height of selected region
      @param poTarget Image of selected region
      @return -
  **/
  void getRegion(int iCenterMode,
                 int iX, 
                 int iY, 
                 int iRegionWidth, 
                 int iRegionHeight,
                 ICL<Type> &poTarget);
  
  //--------------------------------------------------------------------------
  /** Return the variable type of the current ICL object
      @return iType Return a value for the depth 
                    (8 = unsigend char, 32 = float)
  **/
  int getVarType() const;

    //@}

/* }}} */
  
  /* {{{ basic image manipulation: */

  //@{
  //--------------------------------------------------------------------------
  /** Sets the pixels of one or all channels to zero
      @param iChannel Channel to fill with zero (default: -1 = all)
  **/
  void clear(int iChannel = -1, Type tValue = 0);

  //--------------------------------------------------------------------------
  /** Scale image by factor fFactor. Using different methods 
      (Simple, BiLinInter, Average) According to the selected method the 
      pixels in the returned image are build of one pixel (simple), bilinear 
      interpolation of four pixels (BiLinInter) or the average  of a region 
      (average) from the source image.
      @param fFactor Scaling factor
      @param method Scaling method 
  **/
  //void scale(float fFactor, int method, ICL<Type> *img) const;

  //--------------------------------------------------------------------------
  /** Scale the channel min/ max range to the new range tMin, tMax. 
      @param tMin -
      @param tMax -
      @param channel Scale channel CHANNEL
             \li channel = -1 scale all channels [default]
  **/
  void scaleRange(float tMin, float tMax, int channel = -1);

  //@}

/* }}} */

}; // class

} //namespace ICL

#endif //ICL_H

