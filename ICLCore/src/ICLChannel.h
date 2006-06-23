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
#include "ICLMacros.h"

//---- Use the following namespaces as default ----
using namespace std;

//---- ICL in its own namespace ----
namespace ICL {

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

  //--------------------------------------------------------------------------
  /** Get the image roi YOffset.
      @param -
      @return The image roi height in pixel
  **/
  int getRoiYOffset() const
    {
      //---- Return the image height
      return m_iImageRoiYOffset;
    }

  //--------------------------------------------------------------------------
  /** Get the image width.
      @param -
      @return The image width in pixel
  **/
  int getWidth() const
    {
      //---- Return the image width 
      return m_iImageWidth;
    }
  
  //--------------------------------------------------------------------------
  /** Get the image height.
      @param -
      @return The image height in pixel
  **/
  int getHeight() const
    {
      //---- Return the image height
      return m_iImageHeight;
    }
  
  //--------------------------------------------------------------------------
  /** Get the image dimension (width*height).
      @param -
      @return The image dimension in pixel
  **/
  int getDim() const
    {
      //---- Return the image dim
      return m_iImageDim;
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
  //------------------------------------------------------------------------
 private:
  
 protected:
  //------------------------------------------------------------------------
  /** Array containing image data (row-by-row).**/
  Type *m_ptData;
  
  /** Array of pointers to the beginning of each row. **/
  Type **m_pptRow;
  
 public:
  typedef boost::shared_ptr<ICLChannel<Type> > AutoPtr;
  
  //--------------------------------------------------------------------------
  //Info object
  ICLChannelInfo m_oInfo;
  
  /* {{{ Konstruktor/ Destruktor: */
  //@{
  //-------------------------------------------------------------------------- 
  /** Allocates memory for an image channel of the specified width and height.
      @param iWidth Image channel width
      @param iHeight Image channel height
  **/
  ICLChannel(int iWidth, int iHeight);

  //-------------------------------------------------------------------------- 
  /** Copy constructor - generates a copy of the source channel
      @param tSource Source image channel
  **/
  ICLChannel(const ICLChannel<Type>& tSource);
  
  //-------------------------------------------------------------------------- 
  /** Destructor **/
  ~ICLChannel();

  //@}
  /* }}} */
                                      
  /* {{{ class operator */
  //@{
  //--------------------------------------------------------------------------
  /** Get or Set the pixel at position (x,y)
      @param iX x-coordinate of the pixel
      @param iY y-coordinate of the pixel
      @return -
  **/
  Type& operator() (int iX, int iY) const
    {
      return (m_pptRow[iY])[iX];
    }
  
  //@}
  /* }}} */

  /* {{{ Set functions: */
  //@{
  //--------------------------------------------------------------------------
  /** Assigns a new value to the pixel at the specified position.
      @param iX x-coordinate of the pixel
      @param iY y-coordinate of the pixel
      @param tValue New pixel value at position
  **/
  void setPixel(int iX, int iY, Type tValue)
    {
      //---- Set pixel to value ----
      (m_pptRow[iY])[iX] = tValue;
    }
  
  //--------------------------------------------------------------------------
  /** Copies data from pSource to row iRow.
      @param pSrc An STL vector with the src data
      @param iRow Index of row
  **/
  void setRowData(vector<Type> pSrc,int iRow)
    {
      //---- Copy ----
      std::copy(pSrc.begin(), pSrc.end(), getRowPtr(iRow));
    } 
  
  //@}
/* }}} */

  /* {{{ Get functions: */

  //@{
  //--------------------------------------------------------------------------
  /** Return the value of the pixel at the position (x,y).
      @param iX x-coordinate of the pixel
      @param iY y-coordinate of the pixel
      @return Value at pixel position
  **/
  Type getPixel(int iX, int iY) const
    {
      //---- Return pixel value at position ----
      return (m_pptRow[iY])[iX];
    }
  
  //--------------------------------------------------------------------------
  /** Returns minimal pixel value
      @return Minimal pixel value
  **/
  Type getMin() const;
  
  //--------------------------------------------------------------------------
  /** Returns maximal pixel value  
      @return Maximal pixel value
  **/
  Type getMax() const; 

  //-------------------------------------------------------------------------- 
  /** Return pointer of the selected row.
      @param iRow Arg index of row
  **/
  Type* getRowPtr(int iRow) const {return m_pptRow[iRow];}
  
  //--------------------------------------------------------------------------
  /** Return the pointer to the begin of the data. 
      @return The beginning of the data vector **/
  Type* getDataBegin() { return m_ptData;}

  //--------------------------------------------------------------------------
  /** Return the pointer to the end of the data. 
      @return The beginning of the data vector **/
  Type* 
  getDataEnd() { return m_ptData+m_oInfo.getDim();}

  //--------------------------------------------------------------------------
  /** Returns the width of the image channel
      @return Width of channel
  **/
  int getWidth() const { return m_oInfo.getWidth();}
  
  //--------------------------------------------------------------------------
  /** Returns the  height of the image channel
      @return height of image channel
  **/
  int getHeight() const { return m_oInfo.getHeight();}

  //--------------------------------------------------------------------------
  /** Returns the  dimension of the image channel (width*height)
      @return height of image channel
  **/
  int getDim() const { return m_oInfo.getDim();}
  
  //--------------------------------------------------------------------------
  /** Returns the width of the image channel roi
      @return Width of channel roi
  **/
  int getRoiWidth() const { return m_oInfo.getRoiWidth();}
  
  //--------------------------------------------------------------------------
  /** Returns the  height of the image channel roi
      @return height of image channel roi
  **/
  int getRoiHeight() const { return m_oInfo.getRoiHeight();}

  //--------------------------------------------------------------------------
  /** Returns the XOffset of the image channel roi
      @return Width of channel roi
  **/
  int getRoiXOffset() const { return m_oInfo.getRoiXOffset();}
  
  //--------------------------------------------------------------------------
  /** Returns the YOffset of the image channel roi
      @return height of image channel roi
  **/
  int getRoiYOffset() const { return m_oInfo.getRoiYOffset();}

  //--------------------------------------------------------------------------
  /** Returns the depth in bit of the image channel
      @return Depth of image channel in bit
  **/
  //int getDepth() const { return m_iDepth;}
  
  //@}

/* }}} */
  
  /* {{{ basic channel functions: */

  //@{

  //--------------------------------------------------------------------------
  /** Set each pixel to zero. **/
  void clear(Type tValue = 0)
    { 
      fill(m_ptData, m_ptData+m_oInfo.getDim(), tValue);
    };
  
  //--------------------------------------------------------------------------
  /** Resizes the channel to the new width and height.
      @param iNewWidth New width
      @param iNewHeight New height
  **/
  void resize(int iNewWidth, int iNewHeight);

  //--------------------------------------------------------------------------
  /** Sets the channel roi to the new roi-width and roi-height.
      @param iNewRoiWidth New roi-width
      @param iNewRoiHeight New roi-height
  **/
  void setImageRoi(int iNewRoiWidth, int iNewRoiHeight)
    {
      m_oInfo.setRoi(iNewRoiWidth, iNewRoiHeight);
    }
  
  //--------------------------------------------------------------------------
  /** Sets the channel roi to the new roi-width and roi-height.
      @param iXOffset The x-psoition 
      @param iYOffset The y-position
  **/
  void setImageRoiOffset(int iXOffset, int iYOffset)
    {
      m_oInfo.setRoiOffset(iXOffset, iYOffset);
    }
  
  //--------------------------------------------------------------------------
  /** Scales each pixel value with scaleFactor.
      @param scaleFactor
  **/
  void scaleRange(Type scaleFactor);
  
  //--------------------------------------------------------------------------
  /** Scales pixel values from given min/max values to new min/max values.
      Values exceeding the given range are set to the new min/max values.
      For an automatic scaling use the results of  min(),max() as as arguments.
      (Defining a range allows to compare different images.)
      @param tNewMin, tNewMax, New min/max values
      @param tMin,    tMax,    Assumed min/max values
  **/
  void scaleRange(float tNewMin, float tNewMax, float tMin, float tMax);

  //@}

/* }}} */
 
};

/* }}} */

} //namespace ICL

#endif //ICLCHANNEL_H
