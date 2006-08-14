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
   - depth:  depth8 for iclbyte or depth32 for iclfloat-images
   - format: color-format associated with images channels 
             (see section "ICL Color Formats")
   - data:  getDataPtr(int) returns image data form nth channel 
            as void pointer. The function is implented in the 
            inherited classes ICL<iclbyte> and ICL<iclfloat>, 
            which also provide type-safe access functions, 
            e.g. getData (int).
   
  \section How to use the ICLBase class.
  Because ICLBase is an abstract class, there can be no objects
  instantiated from this class. It merely provides a common interface
  to methods provided by the inherited class ICL<iclbyte> and ICL<iclfloat>.

  The following example should explain how to work with ICLBase class.
  
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

  Many operations on the ICL image class are conceptually independent
  on the concrete pixel type, e.g. recombining channels or resizing. 
  For these operations the ICLBase class provides abstract or implemented
  methods ensuring a common and type-independent interface.

  For example, to resize an image, one can easily write:
  <pre>
  void any_function(ICLBase *poBase){
     poBase->resize(256,256);
     ...
  }
  </pre>
  instead of the more complicated and confusing code:
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
      /// Create a shallow copy of the image.
      /** It exploits the given destination image if possible,
          i.e. if the pixel depth matches. Else this image is released
          and a new one is created.
      **/
      virtual ICLBase* shallowCopy(ICLBase* poDst = NULL) const;

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
      virtual ICLBase *scaledCopyROI(ICLBase *poDst = NULL, iclscalemode eScaleMode=interpolateNN) const=0;
      //@}
      /* }}} */

      /* {{{ getter functions */

      ///@name getter functions
      //@{
      /// return size of the images
      const ICLsize& getSize() const { return m_oSize; }

      /// return width of the images
      int getWidth()  const
        {
          FUNCTION_LOG("");
          return m_oSize.width;
        }
  
      /// return height of the images
      int getHeight() const
        {
          FUNCTION_LOG("");
          return m_oSize.height;
        }

      /// returns the pixelcount of each channel
      /** @ return width * height
      
      */
      int getDim() const
        {
          FUNCTION_LOG("");
          return m_oSize.width * m_oSize.height;
        }


      /// returns the channel count of the image
      /**
          @return count of channels
      **/
      int getChannels() const
        {
          FUNCTION_LOG("");
          return m_iChannels;
        }


      /// returns the depth (depth8u or depth32f)
      /** 
          @return image depth ()
          @see icldepth
      **/
      icldepth getDepth() const
        {
          FUNCTION_LOG("");
          return m_eDepth;
        }

      /// returns the current (color)-format of this image
      /** 
          @return current (color)-format
          @see iclformat
      **/
      iclformat getFormat() const
        {
          FUNCTION_LOG("");
          return m_eFormat;
        }

      /// returns if two images have same size, and channel count
      /** @param iNewWidth image width to test
          @param iNewHeight image height to test
          @param iNewNumChannels image channel count to test
      **/
      int isEqual(int iNewWidth, int iNewHeight, int iNewNumChannels) const
        {
          FUNCTION_LOG("isEqual("<<iNewWidth<<","<< iNewHeight << ","<< iNewNumChannels<< ")");
          return (m_oSize.width == iNewWidth) && (m_oSize.height == iNewHeight) && (m_iChannels == iNewNumChannels);
          
        }
      //@}
      //@{ @name [getter functions for ROI handling]
      
      /// Gets the ROI of this image
      /** @see ICL*/
      void getROI(ICLpoint &offset, ICLsize &size) const;
      /// Gets the ROI of this image, componentwise
      void getROI(int &riX, int &riY, int &riWidth, int &riHeight) const;

      /// Gets the ROI offset of this image
      /** @see ICL*/
      void getROIOffset(ICLpoint &offset) const {
         offset = m_oROIoffset;
      }
      /// Gets the ROI offset of this image
      const ICLpoint& getROIOffset () const {return m_oROIoffset;}
      /// Gets the ROI offset of this image, componentwise
      void getROIOffset(int &riX, int &riY) const {
         riX = m_oROIoffset.x; riY = m_oROIoffset.y;
      }
      
      /// Gets the ROI size of this image
      /** @see ICL*/
      void getROISize(ICLsize &size) const {
         size = m_oROIsize;
      }
      /// Gets the ROI size of this image
      const ICLsize& getROISize () const {return m_oROIsize;}
      /// Gets the ROI size of this image, componentwise
      void getROISize(int &riWidth, int &riHeight) const {
         riWidth = m_oROIsize.width; riHeight = m_oROIsize.height;
      }

      /// returns if the image has a ROI that is smaller then the image
      int hasFullROI() const;
      
      /// resetur the image ROI to the whole image size with offset (0,0)
      void delROI();
      
      //@}

      /* }}} */

      /* {{{ abstract functions (implemented in the ICL class)*/

      //@{ @name [EIF for data access]
      
      /// returns a pointer to first data element
      /** @see ICL*/
      virtual void* getDataPtr(int iChannel) const = 0;
    
      /// returns a pointer to the channel data ROI, assuming given or own ROI
      /** @see ICL*/
      virtual iclbyte  *roiData8u(int iChannel, const ICLpoint* poROIoffset = 0) const=0;
 
      /// returns a pointer to the channel data ROI, assuming given or own ROI
      /** @see ICL*/
      virtual iclfloat *roiData32f(int iChannel, const ICLpoint* poROIoffset = 0) const=0;

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
      virtual ICL32f *convertTo32Bit(ICL32f* poDst) const=0;
  
      /// Return a copy of the object with depth 8 bit (IPP-OPTIMIZED)
      /** @see ICL*/
      virtual ICL8u *convertTo8Bit(ICL8u* poDst) const=0;
        
      //@}
    
      //@{ @name [EIF for low basic image processing routines]
      
      /// perform a scaling operation of the images (keeping the data) (IPP-OPTIMIZED)
      /** @see ICL*/
      virtual void scale(int iNewWidth, int iNewHeight, iclscalemode eScaleMode=interpolateNN)=0;
      
      /// Scale the channel min/ max range to the new range tMin, tMax.
      /** @see ICL*/
      virtual void scaleRange(float fMin=0.0, float fMax=255.0, int iChannel = -1)=0;

      /// Scales pixel values from given min/max values to new min/max values.
      /** @see ICL */
      virtual void scaleRange(float tNewMin, float tNewMax, float tMin, float tMax, int iChannel = -1)=0;

      //@}
      //@{ @name [EIF for IPP compability] (only if WITH_IPP_OPTIMIZATION is defined)
      
#ifdef WITH_IPP_OPTIMIZATION
      /// returns the ROI rect in IPP-compatible format
      /** @see ICL*/
      virtual IppiRect getROI() const {
         IppiRect oRoi = {m_oROIoffset.x,m_oROIoffset.y,
                          m_oROIsize.width,m_oROIsize.height};
         return oRoi;
      }

      /// returns the line width in bytes 
      /** @see ICL*/
      virtual int ippStep() const
        {
          return (m_eDepth == depth8u ? sizeof(iclbyte) : sizeof(iclfloat)) * getWidth();
        }

      /// alias for getROISize
      virtual IppiSize ippSize() const { return getROISize(); }

#endif //WITH_IPP_OPTIMIZATION

      //@}

      /* }}} */

      /* {{{ accessing underlying r_e_a_l classes */

      //@{ @name accessing underlying r_e_a_l classes
      /// returns an ICL8u* intstance of this image
      /** 
      @return ICL8u* instance of this image
      **/
      ICL8u* asIcl8u() const
        {
          FUNCTION_LOG("");
          return reinterpret_cast<ICL8u*>((void*)this);
        }
  

      /// returns an ICL32f* intstance of this image
      /**
      @return ICL32f* instance of this image
      **/
      ICL32f* asIcl32f() const
        {
          FUNCTION_LOG("");
          return reinterpret_cast<ICL32f*>((void*)this);
        }
      //@}

      /* }}} */

      /* {{{ setter functions */

      ///@name setter functions (including ROI handling)
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

      /// sets the ROI to a specified rect
      /** negative values are allowed and are interpreted relative
          to image size or upper left corner appropriately. */
      void setROI(int iX, int iY, int iWidth, int iHeight);
      /// sets the ROI to a specified rect <b>without any check</b>
      void setROI(const ICLpoint &oOffset, const ICLsize &oSize);
      
      /// set the ROI offset to a specified point
      /** @see ICL*/
      void setROIOffset(int iX, int iY) {
         setROI(iX,iY,m_oROIsize.width,m_oROIsize.height);
      }
      /// set the ROI offset to a specified point
      void setROIOffset(const ICLpoint &oOffset) {
         setROI(oOffset.x, oOffset.y, m_oROIsize.width, m_oROIsize.height);
      }
      
      /// set the ROI size to a specified dimension
      /** @see ICL*/
      void setROISize(int iWidth, int iHeight) {
         setROI(m_oROIoffset.x, m_oROIoffset.y, iWidth, iHeight);
      }
      /// set the ROI size to a specified dimension
      void setROISize(const ICLsize &oSize) {
         setROI(m_oROIoffset.x, m_oROIoffset.y, oSize.width, oSize.height);
      }

      //@}

      /* }}} */

      /* {{{ utility functions */
      //@{ @name utility functions
      /// prints the image to std-out
      /** @param sTitle optional title, that can be printed before
                        printing the image parameters
      **/
      void print(string sTitle="image") const;
      //@}
      /* }}} */

      protected:

      /* {{{ Constructor  */

      /// Creates an ICLBase object with specified width, height, format, depth and channel count
      /** 
      If channel count is <= 0, the number of channels is computed 
      automatically from format.
      **/
      ICLBase(int iWidth, 
              int iHeight, 
              iclformat eFormat, 
              icldepth eDepth=depth8u,
              int iChannels=-1);
      
      /* }}} */

      /* {{{ data */

      /// channel count of the image
      int m_iChannels;

      /// size of image: width x height
      ICLsize m_oSize;

      /// (color)-format associated with the images channels
      iclformat m_eFormat;

      /// depth of the image (depth8 for iclbyte/depth32 for iclfloat)
      icldepth m_eDepth;

      // internal storage of the ROI parameters
      ICLpoint m_oROIoffset; //< ROI offset
      ICLsize  m_oROIsize;   //< ROI size

      /* }}} */
    };
}

#endif 
