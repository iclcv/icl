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
  
  /// ImgI is the Image-Interface class that provides save access to underlying Img-template
  /**
  \section Class
  The ImgI class provides access to the following basic image features:
   - image size 
   - channel count
   - depth:  depth8 for iclbyte or depth32 for iclfloat-images
   - format: color-format associated with images channels 
             (see section "Img Color Formats")
   - raw data:  getDataPtr(int) returns image data form nth channel 
            as void pointer. The function is implented in the 
            inherited classes Img<iclbyte> and Img<iclfloat>, 
            which also provide type-safe access functions, 
            e.g. getData (int).
   
  \section How to use the ImgI class.
  As the ImgI is an abstract class, no ImgI objects can be instantiated
  It merely provides a common interface to methods provided by the 
  inherited class Img<iclbyte> and Img<iclfloat>.

  The following example should explain how to work with ImgI class.
  
  <pre>
  void special_function_8(Img32f* poImg32f){...}
  void special_function_32(Img8* poImg8f){...}
  
  void generic_function(ImgI *poImage){
     if(poImage->getDepth()==depth8u){
        special_function_8(poImage->asImg<iclbyte>());
     }else{
        special_function_32(poImage->asImg<iclfloat>());
     }
  }
  </pre>
  Template functions can be called in an analogous way:
  <pre>
  template<class T> void template_function(Img<T> *poImg){...}

  void generic_function(ImgI *poImage){
     if(poImage->getDepth()==depth8u){
        template_function<iclbyte>(poImage->asImg<iclbyte>);
     }else{
        template_function<iclfloat>(poImage->asImg<iclfloat>);
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
     poBase->resize(Size(256,256));
     ...
  }
  </pre>

  **/ 
  class ImgI
    {   
      public:
      
      //@{ @name Destructor
      /* {{{ open */
    
      /// Destructor
      virtual ~ImgI();
      
      //@}
      /* }}} */ 
      
      //@{ @name functions for data exchange
      /* {{{ open */

      /// creates a shallow copy of the image.
      /** It exploits the given destination image if possible,
          i.e. if the pixel depth matches. Else this image is released
          and a new one is created.
          @param ppoDst destination image (if Null, a new one is created)
      **/
      virtual void shallowCopy(ImgI** ppoDst = NULL) const;

      /// copies the image data into the destination image
      /** this function is implemented in the Img-template class
          @see Img
      **/
      virtual ImgI* deepCopy(ImgI* poDst = NULL) const=0;
      
      /// copies (or scales if necessary) the image data into the destination image and performs a
      /** this function is implemented in the Img-template class
          @see Img
      **/
      virtual ImgI* scaledCopy(ImgI *poDst, ScaleMode eScaleMode=interpolateNN) const=0;
    

      /// copies the image data in the images ROI into the destination images ROI
      /** this function is implemented in the Img-template class
      @see Img
      **/
      virtual ImgI *deepCopyROI(ImgI *poDst = NULL) const=0;

      /// scales the image data in the image ROI into the destination images ROI
      /** this function is implemented in the Img-template class
      @see Img
      **/
      virtual ImgI *scaledCopyROI(ImgI *poDst = NULL, ScaleMode eScaleMode=interpolateNN) const=0;
      //@}

      /* }}} */

      //@{ @name getter functions
      /* {{{ open */

      /// returns the size of the images
      const Size& getSize() const {
        FUNCTION_LOG(""); 
        return m_oSize; 
      }
      
      /// returns the pixelcount of each channel
      int getDim() const
        {
          FUNCTION_LOG("");
          return m_oSize.width * m_oSize.height;
        }


      /// returns the channel count of the image
      int getChannels() const
        {
          FUNCTION_LOG("");
          return m_iChannels;
        }


      /// returns the depth (depth8u or depth32f)
      Depth getDepth() const
        {
          FUNCTION_LOG("");
          return m_eDepth;
        }

      /// returns the current (color)-format of this image
      Format getFormat() const
        {
          FUNCTION_LOG("");
          return m_eFormat;
        }

      /// returns the lenght of an image line in bytes (width*sizeof(Type))
      virtual int getLineStep() const = 0;

      /// returns if two images have same size, and channel count
      /** @param iNewWidth image width to test
          @param iNewHeight image height to test
          @param nChannels channel count to test
      **/
      int isEqual(const Size &s,int nChannels) const
        {
          FUNCTION_LOG("isEqual("<<s.width<<","<< s.height << ","<< nChannels << ")");
          return (m_oSize == s) && (m_iChannels == nChannels);
          
        }
      //@}
      /* }}} */
      
      //@{ @name ROI handling functions
      /* {{{ open */
      
      /// returns the images ROI rectangle
      Rect getROI() const{
        FUNCTION_LOG("");
        return Rect(m_oROIOffset,m_oROISize);
      }
      void getROI(Point& offset, Size& size) const {
        FUNCTION_LOG("");
        offset = m_oROIOffset;
        size   = m_oROISize;
      }

      /// returns the images ROI offset
      const Point& getROIOffset() const{
        FUNCTION_LOG("");
        return m_oROIOffset;
      }

      /// returns the images ROI size
      const Size& getROISize() const{
        FUNCTION_LOG("");
        return m_oROISize;
      }
     
      /// fast and direct setting of ROI to a given rectangle
      /** setting of ROI rectangle without any checks. Compare
          setROIOffset and setROISize below. */
      void setROI(const Rect &r){
        FUNCTION_LOG("");
        m_oROIOffset = Point(r.x,r.y);
        m_oROISize   = Size(r.width,r.height);
      }
      /// fast and direct setting of ROI without any checks
      /** sets ROI offset and size to specified arguments without any checks. 
          Compare setROIOffset and setROISize below. */
      void setROI(const Point &offset, const Size &size){
        FUNCTION_LOG("");
        m_oROIOffset = offset;
        m_oROISize   = size;
      }

      /// sets the image ROI offset to a given point
      /** While the methods setROI(rect) and setROI(offset,size) directly
          set the images ROI from the given arguments, the following separate
          methods provide a check for validity of the given arguments, which
          are adapted to meaningful values otherwise.
          Additionally, negative values are interpreted relative to the
          whole image size resp. the upper right corner of the image.

          E.g. an offset (5,5) with size (-10,-10) sets the ROI to the
          inner sub image with a 5-pixel margin. offset(-5,-5) and size (5,5) 
          sets the ROI to the upper right 5x5 corner. 
          Attention: If both size and offset are changed in series,
          the calling order should be: setROISize before setROIOffset,
          because the latter refers to the existing ROI size.
      **/
      /** for more deails look at the above function setROI */
      void setROIOffset(const Point &offset);
      
      /// sets the image ROI size to a given Size
      /** for more deails look at the above function setROIOffset */
      void setROISize(const Size &size);
     
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

      //@{ @name data access
      /* {{{ open */
      
      /// returns a pointer to first data element
      /** @see Img*/
      virtual void* getDataPtr(int iChannel) const = 0;
    
      //@} 
      /* }}} */
      
      //@{ @name class organisation
      /* {{{ open */

      /// Makes the image channels independent from other images.
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

      /// creates a hole new Img internally (image data will be lost)
      /** @see Img*/
      virtual void renew(const Size &s, int iNewNumChannel)=0;

      /// resizes the image to new values (image data is scaled)
      /** @see Img*/
      virtual void resize(const Size &s)=0;

      /// sets the format associated with channels of the image
      /**
      The channel count of the image is set to the channel count
      asociated with the set format, if they differ.
      E.g an image with one channel will have 3 channels after
      a setFormat(formatRGB) - call.
      @param eFormat new format value
      @see getChannelsOfFormat
      **/
      void setFormat(Format eFormat);
      
      //@}
      /* }}} */

      //@{ @name Type conversion functions
      /* {{{ */

      
      // @}
      /* }}} */

      //@{ @name image processing functions
      /* {{{ open */
      
      /// performs an inplace scaling operation of each pixel value (IPP-OPTIMIZED)
      /** @see Img*/
      virtual void scale(const Size& s, ScaleMode eScaleMode=interpolateNN)=0;
      
      /// Scale the channel min/ max range to the new range tMin, tMax.
      /** @see Img*/
      virtual void scaleRange(float fMin=0.0, float fMax=255.0, int iChannel = -1)=0;

      /// Scales pixel values from given min/max values to new min/max values.
      /** @see Img */
      virtual void scaleRange(float tNewMin, float tNewMax, float tMin, float tMax, int iChannel = -1)=0;

      //@} 
      /* }}} */

      //@{ @name accessing underlying Img classes 
      /* {{{ open */

      /// returns an Img<T>* intstance of this image (internal: reinterpret_cast)
      /** 
      @return Img<T>* instance of this image
      **/
      template <class T>
      Img<T> *asImg() const{
        return reinterpret_cast<Img<T>*>((void*)this);
      }

      /// returns an Img<T> instance of this image (type-conversion or deep copy)
      /** If the requested type differs from the actual image type, then a type
          conversion is performed to transfer the image data to poDst. Else
          deepCopy is called, to transfer the image data. If poDst is NULL, it
          is created with identical parameters, except for the images depth, which
          is given by the template parameter T
      */
      template <class T>
      Img<T> *convertTo( Img<T>* poDst=NULL ) const;

      //@}
      /* }}} */

      //@{ @name utility functions
      /* {{{ open */
      /// prints the image to std-out
      /** @param sTitle optional title, that can be printed before
          printing the image parameters to identify the message.
      **/
      void print(string sTitle="image") const;
      //@}
      /* }}} */

      protected:

      /* {{{ Constructor  */

      /// Creates an ImgI object with specified width, height, format, depth and channel count
      /** 
      If channel count is <= 0 (default), the number of channels is computed 
      automatically from format using the getChannelsOfFormat function from the icl namespace.
      @param s size of the ImgI
      @param eFormat (color)-format of the image
      **/
      ImgI(const Size &s,
           Format eFormat, 
           Depth eDepth=depth8u,
           int iChannels=-1);
      
      /* }}} */

      /* {{{ data */

      /// channel count of the image
      int m_iChannels;

      /// size of image: width x height
      Size m_oSize;

      /// (color)-format associated with the images channels
      Format m_eFormat;

      /// depth of the image (depth8 for iclbyte/depth32 for iclfloat)
      Depth m_eDepth;

      // internal storage of the ROI parameters
      Point m_oROIOffset; //< ROI offset
      Size  m_oROISize;   //< ROI size

      /* }}} */
    };
}

#endif 
