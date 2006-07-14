/*
ICLBase.h

Written by: Michael Götting and Christof Elbrechter(2006)
University of Bielefeld
AG Neuroinformatik
{mgoettin,celbrech}@techfak.uni-bielefeld.de
*/

#ifndef ICLBASE_H
#define ICLBASE_H
    
#include "ICLCore.h"
#include <vector>

using namespace std;

namespace icl {
  

  /// forward declaration of the ICL-class
  template<class T> class ICL;

  /// typedef for 8bit integer images
  typedef ICL<iclbyte> ICL8u;

  /// typedef for 32bit float images
  typedef ICL<iclfloat> ICL32f;
  
  /// Base ICL-Class
  /**
  \section Class
  The ICLBase class provides access to the following basic 
  image features:
   - width 
   - height
   - channel count
   - depth (currently depth8 for 
            iclbyte-images and 
            depth32 for iclfloat-images)
   - format (color)-format, that is associated with 
            images channels (see section "ICL Color Formats"
            on the mainpage)
   - data (virtually) getDataPtr(int) provides "virtual" 
            access for the data as void*. The functions are 
            implented in the inherited classes ICL<iclbyte>
            and ICL<iclfloat>
   
  \section EIF Expanded Interface Functions [EIF]
  A new feature of the ICLBase class are the so called 
  Expanded Interface Functions or short "EIF". 
  <b>**NEW**:</b> To achieve,
  that all functions of the ICL class, which 
  interfaces are independed from the depth that is used,
  are included as "abstract" functions into the ICLBase
  class interface.
  So all basic image operations like resizing
  or changing the channel count, can be performed 
  without any knowledge of the underlying depth.
  <h3>Example</h3>
  Take a look at the following code:
  <pre>
  void any_function(ICLBase *poBase){
     // what ever this function does, imagine
     // it has to enshure a special size of the image
     if(poBase->getDepth()==depth8()){
        poBase->asIcl8()->resize(256,256);
     }else{
        poBase->asIcl32()->resize(256,256);
     }
     ...
  }
  </pre>
  This will work fine, but the casting operations could be
  left out, if the "resize" function is a member of ICLBase.
  In this case, the code can be reduced to:
  <pre>
  void any_function(ICLBase *poBase){
     poBase->resize(256,256);
     ...
  }
  </pre>
  This will let you produce a better understandable code.


  \section Work
  The ICLBase class has no public constructor, as it has 
  virtual functions. 
  It may be used for generic image-
  processing functions. The following example should
  explain how to work with ICLBase class.
  
  <pre>
  void special_function_8(ICL32f* poICL32f){...}
  void special_function_32(ICL8* poICL8f){...}
  
  void generic_function(ICLBase *poImage){
     if(poImage->getDepth()==depth8u){
        special_function_8(poImage->asIcl8());
     }else{
        special_function_32(poImage->asIcl32());
     }
  }
  </pre>
  Template functions can be called in an analogous way:
  <pre>
  template<class T> void template_function(ICL<T> *poICL){...}

  void generic_function(ICLBase *poImage){
     if(poImage->getDepth()==depth8u){
        template_function<iclbyte>(poImage->asIcl8());
     }else{
        template_function<iclfloat>(poImage->asIcl32());
     }
  } 
  </pre>
  **/ 
  class ICLBase
    {   
      public:

      /* {{{ Destructor */
      //@{ name Destructor
      /// Destructor
      virtual ~ICLBase();
      //@}
      /* }}} */

      /* {{{ data exchange functions */

      //@{ @name data exchange functions
      /// copies the image data into the destination image
      /** this function is implemented in the ICL-template class
          @see ICL
      **/
      virtual ICLBase* deepCopy(ICLBase* poDst = NULL) const=0;
      
      /// copies (or scales if necessary) the image data into the destination image and performs a
      /** this function is implemented in the ICL-template class
      @see ICL
      **/
      virtual ICLBase* scaledCopy(ICLBase *poDst, iclscalemode eScaleMode=interpolateNN) const=0;
    

      /// copies the image data in the images ROI into the destination images ROI
      /** this function is implemented in the ICL-template class
      @see ICL
      **/
      virtual ICLBase *deepCopyROI(ICLBase *poDst = NULL) const=0;

      /// scales the image data in the image ROI into the destination images ROI
      /** this function is implemented in the ICL-template class
      @see ICL
      **/
      virtual ICLBase *scaledCopyROI(ICLBase *poDst = NULL) const=0;
      //@}
      /* }}} */

      /* {{{ getter functions */

      ///@name getter functions
      //@{
      /// return the images width
      /**
      if the argument iChannel is defined, then the width of
      a specific channel is returned. This allows derived classes
      to contain channels with different resolutions
      @param iChannel determines the channel which width should be
      returned.
      @return width of the image/selected channel
      **/
      int getWidth(int iChannel = 0)  const
        {
          (void)iChannel;
          return m_iWidth;
        }
  
      /// return the images height
      /** if the argument iChannel is defined, then the width of
          a specific channel is returned. This allows derived classes
          to contain channels with different resolutions
          @param iChannel determines the channel which height should be
                          returned.
          @return height of the image/selected channel
      **/

      int getHeight(int iChannel = 0) const
        {
          (void)iChannel;
          return m_iHeight;
        }


      /// returns the channel count of the image
      /**
          @return count of channels
      **/
      int getChannels() const
        {
          return m_iChannels;
        }


      /// retruns the depth (depth8u or depth32f)
      /** 
          @return image depth ()
          @see icldepth
      **/
      icldepth getDepth() const
        {
          return m_eDepth;
        }

      /// returns the current (color)-format of this image
      /** 
          @return current (color)-format
          @see iclformat
      **/
      iclformat getFormat() const
        {
          return m_eFormat;
        }

      /// returns if two images have same size, and channel count
      /** @param iNewWidth image width to test
          @param iNewHeight image height to test
          @param iNewNumChannels image channel count to test
      **/
      int isEqual(int iNewWidth, int iNewHeight, int iNewNumChannels) const
        {
          return (m_iWidth == iNewWidth) && (m_iHeight == iNewHeight) && (m_iChannels == iNewNumChannels);
          
        }
      
      //@}

      /* }}} */

      /* {{{ abstract functions (implemented in the ICL class)*/

      //@{ @name [EIF for data access]
      
      /// returns a pointer to first data element
      /** @see ICL*/
      virtual void* getDataPtr(int iChannel) const = 0;
    
      //@}
      //@{ @name [EIF for channel management] (now default)

      /// Makes the image channels inside the ICL independent from other ICL.
      /** @see ICL*/
      virtual void detach(int iIndex = -1)=0;
      /// Removes a specified channel.
       /** @see ICL*/
      virtual void removeChannel(int iChannel)=0;
      
      /// Swap channel A and B
      /** @see ICL*/
      virtual void swapChannels(int iIndexA, int iIndexB)=0;

      /// sets the channel count to a new value
      /** @see ICL*/
      virtual void setNumChannels(int iNewNumChannels)=0;

      /// creates a hole new ICL internally
      /** @see ICL*/
      virtual void renew(int iNewWidth, int iNewHeight,int iNewNumChannel)=0;

      /// resizes the image to new values
      /** @see ICL*/
      virtual void resize(int iNewWidth, int iNewHeight)=0;
      
      //@}
      //@{ @name [EIF for type conversion]

      /// Return a copy of the object with depth 32 bit. (IPP-OPTIMIZED)
      /** @see ICL*/
      virtual ICL32f *convertTo32Bit(ICL32f* poDst = NULL) const=0;
  
      /// Return a copy of the object with depth 8 bit (IPP-OPTIMIZED)
      /** @see ICL*/
      virtual ICL8u *convertTo8Bit(ICL8u* poDst = NULL) const=0;
        
      //@}
      //@{ @name [EIF for Region of Interest handling]
      
      /// sets the ROI (region of interests) to a specified rect
      /** @see ICL*/
      virtual void setROI(int iX, int iY,int iWidth,int iHeight)=0;
      
      /// set the ROI- (region of interests) offset to a specified point
      /** @see ICL*/
      virtual void setROIOffset(int iX, int iY)=0;
      
      /// set the ROI- (region of interests) size to a specified dimension
      /** @see ICL*/
      virtual void setROISize(int iWidth, int iHeight)=0;

      /// set the ROI rectangle geometry to [x,y,w,h]
      /**  @see ICL*/
      virtual void setROIRect(std::vector<int> oRect)=0;

      /// Gets the ROI (region of interests) of this image
      /** @see ICL*/
      virtual void getROI(int &riX, int &riY, int &riWidth, int &riHeight) const=0;
      
      /// Gets the ROI- (region of interests) offset of this image
      /** @see ICL*/
      virtual void getROIOffset(int &riX, int &riY) const=0;
      
      /// Gets the ROI- (region of interests) size of this image
      /** @see ICL*/
      virtual void getROISize(int &riWidth, int &riHeight) const=0;

       /// Gets the ROI (region of interests) of this image
      /** @see ICL*/
      virtual std::vector<int> getROIRect() const=0;

      /// returns if the image has a ROI that is smaller then the image
      inline int hasROI() const
        {
          int iX,iY,iW,iH;
          getROI(iX,iY,iW,iH);
          return (iX==0 && iY==0 && iW==m_iWidth && iH == m_iHeight);
        }
      
      /// resetur the image ROI to the hole image size with offset (0,0)
      inline void delROI() 
        {
          setROI(0,0,m_iWidth,m_iHeight);
        }
      
      //@}
      //@{ @name [EIF for low basic image processing routines]
      
      /// perform a scaling operation of the images (keeping the data) (IPP-OPTIMIZED)
      /** @see ICL*/
      virtual void scale(int iNewWidth, int iNewHeight, iclscalemode eScaleMode=interpolateNN)=0;
      
      /// Scale the channel min/ max range to the new range tMin, tMax.
      /** @see ICL*/
      virtual void scaleRange(float fMin=0.0, float fMax=255.0, int iChannel = -1)=0;

      //@}
      //@{ @name [EIF for IPP compability] (only if WITH_IPP_OPTIMIZATION is defined)
      
#ifdef WITH_IPP_OPTIMIZATION
      /// returns a pointer to the channel data as Ipp8u*
      /** @see ICL*/
      virtual Ipp8u *ippData8u(int iChannel) const=0;
 
      /// returns a pointer to the channel data as Ipp32f*
      /** @see ICL*/
      virtual Ipp32f *ippData32f(int iChannel) const=0;

      /// returns the ROI size in IPP-compatible format
      /** @see ICL*/
      virtual IppiSize ippROISize() const=0;

      /// returns the ROI offset in IPP-compatible format
      /** @see ICL*/
      virtual IppiPoint ippROIOffset() const=0;

      /// returns the ROI rect in IPP-compatible format
      /** @see ICL*/
      virtual IppiRect ippROI() const=0;

      /// returns the line width in bytes 
      /** @see ICL*/
      virtual int ippStep() const=0;

      /// returns the image size in IPP-compatible format
      /** @see ICL*/
      virtual IppiSize ippSize() const=0;

      //@}

#endif //WITH_IPP_OPTIMIZATION

      /* }}} */

      /* {{{ accessing underlying r_e_a_l classes */

      //@{ @name accessing underlying r_e_a_l classes
      /// returns an ICL8u* intstance of this image
      /** 
      @return ICL8u* instance of this image
      **/
      ICL8u* asIcl8u() const
        {
          return reinterpret_cast<ICL8u*>((void*)this);
        }
  

      /// returns an ICL32f* intstance of this image
      /**
      @return ICL32f* instance of this image
      **/
      ICL32f* asIcl32f() const
        {
          return reinterpret_cast<ICL32f*>((void*)this);
        }
      //@}

      /* }}} */

      /* {{{ setter functions */

      ///@name setter functions
      //@{
      /// sets the format associated with channels of the image
      /**
      The channel count of the image is set to the channel count
      asociated with the set format, if they differ.
      E.g an image with one channel will have 3 channels after
      a setFormat(formatRGB) - call.
      @param eFormat new format value
      @see getChannelsOfFormat
      **/
      void setFormat(iclformat eFormat);
      //@}

      /* }}} */

      /* {{{ utility functions */
      //@{ @name utility functions
      /// prints the image to std-out
      /** @param sTitle optional title, that can be printed before
                        printing the image parameters
      **/
      void print(string sTitle="image");
      //@}
      /* }}} */

      protected:

      /* {{{ Constructors  */

      ///@name Constructors 
      //@{
      /// Creates an ICLBase Object with specified width, height, channel count and depth 
      /** 
      TODO: put some information here!
      **/
      ICLBase(int iWidth=1, 
              int iHeight=1, 
              int iChannels=1, 
              icldepth eDepth=depth8u);
      
      /// Creates an ICLBase Object with specified width, height, format, depth and channel count
      /** 
      TODO: put some information here!
      **/
      ICLBase(int iWidth, 
              int iHeight, 
              iclformat eFormat, 
              icldepth eDepth=depth8u,
              int iChannels=-1);
      
     
      
      //@}

      /* }}} */

      /* {{{ data */

      /// width of the image                                    
      int m_iWidth;

      /// height of the image
      int m_iHeight;

      /// channel count of the image
      int m_iChannels;

      /// (color)-format associated with the images channels
      iclformat m_eFormat;

      /// depth of the image (depth8 for iclbyte/depth32 for iclfloat)
      icldepth m_eDepth;

      /* }}} */
    };
}

#endif 
