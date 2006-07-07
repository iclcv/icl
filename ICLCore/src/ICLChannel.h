/*
  ICLChannel.h

  Written by: Michael Götting (2004)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICLCHANNEL_H
#define ICLCHANNEL_H
    
//-------- includes --------
#include <iostream>
#include <vector>
#include <iterator>
#include <boost/shared_ptr.hpp>
#include "ICLCore.h"

//---- Use the following namespaces as default ----
using namespace std;

//---- ICL in its own namespace ----
namespace icl {

//---- Foreward deklaration ----
template <class Type> class ICLChannel;

/* {{{ class ICLChannelInfo */

class ICLChannelInfo
{ 
 private:  
  int m_iImageWidth, m_iImageHeight, m_iImageDim;
  int m_iImageRoiWidth, m_iImageRoiHeight;
  int m_iImageRoiXOffset, m_iImageRoiYOffset;
  
 public:
  //friend class ICLChannel<iclbyte>;
  //friend class ICLChannel<iclfloat>;
  
  ICLChannelInfo() {};
  ~ICLChannelInfo() {};
  
  //--------------------------------------------------------------------------
  /** Set the image size (The origin of the image is implied to be in the top 
      left corner, with x values increasing from left to right and y 
      values increasing downwards). Also set the ROI to the image width and 
      height and the ROI offset to zero. 
      @param iWidth The image width in pixel
      @param iHeight The image height in pixel
  **/
  void setSize(int iWidth, int iHeight) 
    {
      m_iImageWidth = iWidth;
      m_iImageHeight = iHeight;
      m_iImageDim = m_iImageWidth * m_iImageHeight;
      m_iImageRoiWidth = iWidth;
      m_iImageRoiHeight = iHeight;
      m_iImageRoiXOffset = 0;
      m_iImageRoiYOffset = 0;
    }

  //--------------------------------------------------------------------------
  /** Set the image ROI (ROI is the short form for Region Of Interest). 
      The origin of the ROI is implied to be in the top left corner, with x 
      values increasing from left to right and y values increasing downwards.
      @param iWidth The image roi width in pixel
      @param iHeight The image roi height in pixel
  **/
  void setRoi(int iWidth, int iHeight) 
    {
      m_iImageRoiWidth = iWidth;
      m_iImageRoiHeight = iHeight;
    }
  
  //--------------------------------------------------------------------------
  /** Set the image ROI offset. The origin of the ROI is implied to be in the 
      top left corner, with x values increasing from left to right and y 
      values increasing downwards.
      @param iXOffset The x - offset 
      @param iYOffset The y - offset
  **/
  void setRoiOffset(int iXOffset, int iYOffset) 
    {
      m_iImageRoiXOffset = iXOffset;
      m_iImageRoiYOffset = iYOffset;
    }

  //--------------------------------------------------------------------------
  /** Get the image roi width.
      @param -
      @return The image roi width in pixel
  **/
  int getRoiWidth() const
    {
      //---- Return the image roi width 
      return m_iImageRoiWidth;
    }
  
  //--------------------------------------------------------------------------
  /** Get the image roi height.
      @param -
      @return The image roi height in pixel
  **/
  int getRoiHeight() const
    {
      //---- Return the image height
      return m_iImageRoiHeight;
    }

  //--------------------------------------------------------------------------
  /** Get the image roi XOffset.
      @param -
      @return The image roi height in pixel
  **/
  int getRoiXOffset() const
    {
      //---- Return the image height
      return m_iImageRoiXOffset;
    }

  /// Get the image roi YOffset.
  /** @return The image roi height in pixel
  **/
  int getRoiYOffset() const
    {
      return m_iImageRoiYOffset;
    }

  /// Get the image width.
  /** @return The image width in pixel
  **/
  int getWidth() const
    {
      return m_iImageWidth;
    }
  
  /// Get the image height.
  /** @return The image height in pixel
  **/
  int getHeight() const
    {
      //---- Return the image height
      return m_iImageHeight;
    }
  
  /// Get the image dimension (width*height).
  /** @return The image dimension in pixel
  **/
  int getDim() const
    {
      //---- Return the image dim
      return m_iImageDim;
    }

  /// returns the data offset, for handling channels with ROI set.
  /** The returned value can be added to the images data ptr - 
      The result pointer points the bottom-left pixel of the ROI
      in the image.
      @return the ROI data offset
  **/
  int getRoiOffset() const
    {
      return m_iImageRoiXOffset + m_iImageWidth * m_iImageRoiYOffset;      
    }
  
};

/* }}} */

/* {{{ class ICLChannel */

/**
   @short ICLChannel manages data of a single image channel with origin 
   in the lower left corner. The datatype of the pixels is float (32bit) or
   unsigned char (8bit). This object supports data sharing. 
**/
template <class Type> 
class ICLChannel
{   
  protected:

  /// Array containing image data (row-by-row)
  Type *m_ptData;

  /// This flag indicates if data-buffer has to be deleted in the destructor
  bool m_bDeleteData;
  
  public:
  /// internally used for AutoPtrs
  typedef boost::shared_ptr<ICLChannel<Type> > AutoPtr;
  
  
  ///Info object
  ICLChannelInfo m_oInfo;
  
  /* {{{ Konstruktor/ Destruktor: */
  //@{ @name Constructors and Destructors
  /// Base constructor creates a channel with specified width,height and optional shared data
  /** This Baseconstructor works in two different ways:
      - if ptData is NULL (default), then the constructor allocates memory for 
        an image channel of the specified width and height. The memory will
        be freed in the ICLChannels constructor        
      - else ptData (not NULL) is used as foreign data, so ptData has to 
        be of size iWidth*iHeight, and it must not be deleted as long as
        the image stays valid. In this case, internally a flag is set,
        that indicates that the memory is not owned by this channel, so
        it has not to be freed in the destuctor.
      
      @param iWidth Image channel width
      @param iHeight Image channel height
      @param ptData pointer to data allocated elsewhere
  **/
  ICLChannel(int iWidth, int iHeight, Type *ptData=0);

  /// Copy constructor 
  /** Copy constructor - generates a copy of the source channel
      @param tSource Source image channel
  **/
  ICLChannel(const ICLChannel<Type>& tSource);
  
  
  /// Destructor
  ~ICLChannel();

  //@}
  /* }}} */
                                      
  /* {{{ class operator */
  //@{ @name class operators
  /// Get or Set the pixel at position (x,y)
  /** @param iX x-coordinate of the pixel
      @param iY y-coordinate of the pixel
      @return -
  **/
  Type& operator() (int iX, int iY) const
    {
      return m_ptData[iX+m_oInfo.getWidth()*iY];
    }
  
  //@}
  /* }}} */

  /* {{{ setter functions */

  /// Sets the channel roi to the new roi-width and roi-height.
  /** @param iNewRoiWidth New roi-width
      @param iNewRoiHeight New roi-height
  **/
  void setImageRoi(int iNewRoiWidth, int iNewRoiHeight)
    {
      m_oInfo.setRoi(iNewRoiWidth, iNewRoiHeight);
    }
  
  /// Sets the channel roi to the new roi-width and roi-height.
  /** 
      @param iXOffset The x-psoition 
      @param iYOffset The y-position
  **/
  void setImageRoiOffset(int iXOffset, int iYOffset)
    {
      m_oInfo.setRoiOffset(iXOffset, iYOffset);
    }

/* }}} */
  
  /* {{{ getter functions: */

  //@{ @name getter functions
  /// Returns minimal pixel value
  /** @return Minimal pixel value
  **/
  Type getMin() const;
  
  /// Returns maximal pixel value  
  /** @return Maximal pixel value
  **/
  Type getMax() const; 

  /// Return the pointer to the begin of the data. 
  /** @return The beginning of the data vector 
  **/
  Type* getDataBegin() { return m_ptData;}

  /// Return the pointer to the end of the data.
  /** @return The beginning of the data vector 
  **/
  Type* getDataEnd() { return m_ptData+m_oInfo.getDim();}

  /// Returns the width of the image channel
  /** @return Width of channel
  **/
  int getWidth() const { return m_oInfo.getWidth();}
  
  /// Returns the  height of the image channel
  /** @return height of image channel
  **/
  int getHeight() const { return m_oInfo.getHeight();}

  /// Returns the  dimension of the image channel (width*height)
  /** @return height of image channel
  **/
  int getDim() const { return m_oInfo.getDim();}
  
  /// Returns the width of the image channel roi
  /** @return Width of channel roi
  **/
  int getRoiWidth() const { return m_oInfo.getRoiWidth();}
  
  /// Returns the  height of the image channel roi
  /** @return height of image channel roi
  **/
  int getRoiHeight() const { return m_oInfo.getRoiHeight();}

  /// Returns the XOffset of the image channel roi
  /** @return Width of channel roi
  **/
  int getRoiXOffset() const { return m_oInfo.getRoiXOffset();}
  
  /// Returns the YOffset of the image channel roi
  /** @return height of image channel roi
  **/
  int getRoiYOffset() const { return m_oInfo.getRoiYOffset();}

  //@}
  /* }}} */ 
    
  /* {{{ basic channel functions: */

  //@{ @name manipulation functions

   /// Set each pixel to a specific value.
  /** @param tValue destination 
  **/
  void clear(Type tValue = 0)
    { 
      fill(m_ptData, m_ptData+m_oInfo.getDim(), tValue);
    };
  
  /// Resizes the channel to the new width and height.
  /** @param iNewWidth New width
      @param iNewHeight New height
  **/
  void resize(int iNewWidth, int iNewHeight);

  /// Scales each pixel value with scaleFactor.
  /** Scales each pixel value with scaleFactor.
      @param scaleFactor
  **/
  void scaleRange(Type scaleFactor);
  
  /// Scales pixel values from given min/max values to new min/max values.
  /** Values exceeding the given range are set to the new min/max values.
      For an automatic scaling use the results of  min(),max() as as arguments.
      (Defining a range allows to compare different images.)
      @param tNewMin destination minimum value
      @param tNewMax destination maximum value
      @param tMin current minimum value
      @param tMax current maximum value
  **/
  void scaleRange(float tNewMin, float tNewMax, float tMin, float tMax);

  //@}

/* }}} */
 
};

/* }}} */

} //namespace icl

#endif //ICLCHANNEL_H
