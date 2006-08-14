/*
ImgI.h

Written by: Michael Götting and Christof Elbrechter(2006)
University of Bielefeld
AG Neuroinformatik
{mgoettin,celbrech}@techfak.uni-bielefeld.de
*/

#ifndef ICLBASE_H
#define ICLBASE_H
    
#include "ICLCore.h"

using namespace std;

namespace icl {
  

  /// forward declaration of the Img-class
  template<class T> class Img;

  /// typedef for 8bit integer images
  typedef Img<iclbyte> Img8u;

  /// typedef for 32bit float images
  typedef Img<iclfloat> Img32f;
  
  /// Base Img-Class
  /**
  \section Class
  The ImgI class provides access to the following basic 
  image features:
   - width 
   - height
   - channel count
   - depth:  depth8 for iclbyte or depth32 for iclfloat-images
   - format: color-format associated with images channels 
             (see section "Img Color Formats")
   - data:  getDataPtr(int) returns image data form nth channel 
            as void pointer. The function is implented in the 
            inherited classes Img<iclbyte> and Img<iclfloat>, 
            which also provide type-safe access functions, 
            e.g. getData (int).
   
  \section How to use the ImgI class.
  Because ImgI is an abstract class, there can be no objects
  instantiated from this class. It merely provides a common interface
  to methods provided by the inherited class Img<iclbyte> and Img<iclfloat>.

  The following example should explain how to work with ImgI class.
  
  <pre>
  void special_function_8(Img32f* poImg32f){...}
  void special_function_32(Img8* poImg8f){...}
  
  void generic_function(ImgI *poImage){
     if(poImage->getDepth()==depth8u){
        special_function_8(poImage->asIcl8());
     }else{
        special_function_32(poImage->asIcl32());
     }
  }
  </pre>
  Template functions can be called in an analogous way:
  <pre>
  template<class T> void template_function(Img<T> *poImg){...}

  void generic_function(ImgI *poImage){
     if(poImage->getDepth()==depth8u){
        template_function<iclbyte>(poImage->asIcl8());
     }else{
        template_function<iclfloat>(poImage->asIcl32());
     }
  } 
  </pre>

  Many operations on the Img image class are conceptually independent
  on the concrete pixel type, e.g. recombining channels or resizing. 
  For these operations the ImgI class provides abstract or implemented
  methods ensuring a common and type-independent interface.

  For example, to resize an image, one can easily write:
  <pre>
  void any_function(ImgI *poBase){
     poBase->resize(256,256);
     ...
  }
  </pre>
  instead of the more complicated and confusing code:
  <pre>
  void any_function(ImgI *poBase){
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

  class ImgI
    {   
      public:

      /* {{{ Destructor */
      //@{ name Destructor
      /// Destructor
      virtual ~ImgI();
      //@}
      /* }}} */

      /* {{{ data exchange functions */

      //@{ @name data exchange functions
      /// Create a shallow copy of the image.
      /** It exploits the given destination image if possible,
          i.e. if the pixel depth matches. Else this image is released
          and a new one is created.
      **/
      virtual void shallowCopy(ImgI** ppoDst = NULL) const;

      /// copies the image data into the destination image
      /** this function is implemented in the Img-template class
          @see Img
      **/
      virtual ImgI* deepCopy(ImgI** ppoDst = NULL) const=0;
      
      /// copies (or scales if necessary) the image data into the destination image and performs a
      /** this function is implemented in the Img-template class
          @see Img
      **/
      virtual ImgI* scaledCopy(ImgI **ppoDst, iclscalemode eScaleMode=interpolateNN) const=0;
    

      /// copies the image data in the images ROI into the destination images ROI
      /** this function is implemented in the Img-template class
      @see Img
      **/
      virtual ImgI *deepCopyROI(ImgI **ppoDst = NULL) const=0;

      /// scales the image data in the image ROI into the destination images ROI
      /** this function is implemented in the Img-template class
      @see Img
      **/
      virtual ImgI *scaledCopyROI(ImgI **ppoDst = NULL, iclscalemode eScaleMode=interpolateNN) const=0;
      //@}
      /* }}} */

      /* {{{ getter functions */

      ///@{ @name getter functions

      /// return size of the images
      const Size& getSize() const {
        FUNCTION_LOG(""); 
        return m_oSize; 
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

      /// TODO!!! for ipp- function calls
      virtual int getLineStep() const = 0;

      /// returns if two images have same size, and channel count
      /** @param iNewWidth image width to test
          @param iNewHeight image height to test
          @param iNewNumChannels image channel count to test
      **/
      int isEqual(const Size &s,int nChannels) const
        {
          FUNCTION_LOG("isEqual("<<s.width<<","<< s.height << ","<< iNewNumChannels<< ")");
          return (m_oSize == s) && (m_iChannels == nChannels);
          
        }
      //@}
      //@{ @name [getter functions for ROI handling]
      
      /// Gets the ROI of this image
      /** @see Img*/
      
      /// TODO comment
      Rect getROI() const{
        FUNCTION_LOG("");
        return Rect(m_oROIOffset,m_oROISize);
      }

      /// TODO comment
      const Point& getROIOffset() const{
        FUNCTION_LOG("");
        return m_oROIOffset;
      }

      /// TODO comment
      const Size& getROISize() const{
        FUNCTION_LOG("");
        return m_oROISize;
      }
     
      /// TODO comment
      void setROI(const Rect &r){
        FUNCTION_LOG("");
        setROIOffset(Point(r.x,r.y));
        setROISize(Size(r.width,r.height));
      }

      /// TODO comment
      void setROIOffset(const Point &p);
      
      /// TODO comment
      void setROISize(const Size &s);
     
      /// returns ROISize == ImageSize
      int hasFullROI() const {
        FUNCTION_LOG("");
        return m_oROISize == m_oSize;
      };
      
      /// resets the image ROI to the whole image size with offset (0,0)
      void setFullROI() {
        FUNCTION_LOG("");
        m_oROIOffset = Point();
        m_oROISize = m_oSize;
      }
      
      //@}

      /* }}} */

      /* {{{ abstract functions (implemented in the Img class)*/

      //@{ @name Raw-data access
      
      /// returns a pointer to first data element
      /** @see Img*/
      virtual void* getDataPtr(int iChannel) const = 0;
    
     
      //@} @{ @name Channel Management

      /// Makes the image channels inside the Img independent from other Img.
      /** @see Img*/
      virtual void detach(int iIndex = -1)=0;
      /// Removes a specified channel.
      /** @see Img*/
      virtual void removeChannel(int iChannel)=0;
      
      /// Swap channel A and B
      /** @see Img*/
      virtual void swapChannels(int iIndexA, int iIndexB)=0;

      /// sets the channel count to a new value
      /** @see Img*/
      virtual void setNumChannels(int iNewNumChannels)=0;

      /// creates a hole new Img internally
      /** @see Img*/
      virtual void renew(const Size &s, int iNewNumChannel)=0;

      /// resizes the image to new values
      /** @see Img*/
      virtual void resize(const Size &s)=0;
      
      //@}
      //@{ @name Type conversion functions

      /// Return a copy of the object with depth 32 bit. (IPP-OPTIMIZED)
      /** @see Img*/
      //template<class T>
      //virtual Img<T> *convertTo<T>(Img<T>* poDst) const=0;
      //@}
    
      //@{ @name Basic image processing functions
      
      /// perform a scaling operation of the images (keeping the data) (IPP-OPTIMIZED)
      /** @see Img*/
      virtual void scale(const Size& s, iclscalemode eScaleMode=interpolateNN)=0;
      
      /// Scale the channel min/ max range to the new range tMin, tMax.
      /** @see Img*/
      virtual void scaleRange(float fMin=0.0, float fMax=255.0, int iChannel = -1)=0;

      /// Scales pixel values from given min/max values to new min/max values.
      /** @see Img */
      virtual void scaleRange(float tNewMin, float tNewMax, float tMin, float tMax, int iChannel = -1)=0;

      //@}
 
      /* }}} */

      /* {{{ accessing underlying r_e_a_l classes */

      //@{ @name accessing underlying r_e_a_l classes
      /// returns an Img8u* intstance of this image
      /** 
      @return Img8u* instance of this image
      **/
      // template <class T>
      //Img<T> *asImg<T>()
      //  {
      //    FUNCTION_LOG("");
      //    return reinterpret_cast<Img<T>*>((void*)this);
      //  }
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

      /// Creates an ImgI object with specified width, height, format, depth and channel count
      /** 
      If channel count is <= 0, the number of channels is computed 
      automatically from format.
      **/
      ImgI(const Size &s,
           iclformat eFormat, 
           icldepth eDepth=depth8u,
           int iChannels=-1);
      
      /* }}} */

      /* {{{ data */

      /// channel count of the image
      int m_iChannels;

      /// size of image: width x height
      Size m_oSize;

      /// (color)-format associated with the images channels
      iclformat m_eFormat;

      /// depth of the image (depth8 for iclbyte/depth32 for iclfloat)
      icldepth m_eDepth;

      // internal storage of the ROI parameters
      Point m_oROIOffset; //< ROI offset
      Size  m_oROISize;   //< ROI size

      /* }}} */
    };
}

#endif 
