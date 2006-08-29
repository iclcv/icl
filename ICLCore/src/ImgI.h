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
  
  /// ImgI is the Image-Interface class that provides save access to underlying Img-template
  /* {{{ ImgI class documentation */
  /**
  \section Class
  The ImgI class provides access to the following basic image features:
   - image size 
   - channel count
   - depth:  depth8 for icl8u or depth32 for icl32f-images
   - format: color-format associated with images channels 
             (see section "Img Color Formats")
   - raw data:  getDataPtr(int) returns image data form nth channel 
            as void pointer. The function is implented in the 
            inherited classes Img<icl8u> and Img<icl32f>, 
            which also provide type-safe access functions, 
            e.g. getData (int).
   
  \section How to use the ImgI class.
  As the ImgI is an abstract class, no ImgI objects can be instantiated
  It merely provides a common interface to methods provided by the 
  inherited class Img<icl8u> and Img<icl32f>.

  The following example should explain how to work with ImgI class.
  
  <pre>
  void special_function_8(Img32f* poImg32f){...}
  void special_function_32(Img8* poImg8f){...}
  
  void generic_function(ImgI *poImage){
     if(poImage->getDepth()==depth8u){
        special_function_8(poImage->asImg<icl8u>());
     }else{
        special_function_32(poImage->asImg<icl32f>());
     }
  }
  </pre>
  Template functions can be called in an analogous way:
  <pre>
  template<class T> void template_function(Img<T> *poImg){...}

  void generic_function(ImgI *poImage){
     if(poImage->getDepth()==depth8u){
        template_function<icl8u>(poImage->asImg<icl8u>);
     }else{
        template_function<icl32f>(poImage->asImg<icl32f>);
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
  /* }}} */
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

      /// creates a shallow copy of the image (shared channels).
      /** It exploits the given destination image if possible,
          i.e. if the pixel depth matches. Else this image is released
          and a new one is created.
          @param poDst destination image (if Null, a new one is created)
      **/
      ImgI* shallowCopy(ImgI* poDst = NULL) const;

      /// creates a shallow copy of selected channels of this image
      ImgI* shallowCopy(const int* const piStart, const int* const piEnd,
                        ImgI* poDst = NULL) const;

      /// copies the image data into the destination image
      /** this function is implemented in the Img-template class
          @see Img
      **/
      virtual ImgI* deepCopy(ImgI* poDst = NULL) const=0;
      
      /// copies (or scales if necessary) the image data into the destination image and performs a
      /** this function is implemented in the Img-template class
          @see Img
      **/
      virtual ImgI* scaledCopy(ImgI *poDst, scalemode eScaleMode=interpolateNN) const=0;
    

      /// copies the image data in the images ROI into the destination images ROI
      /** this function is implemented in the Img-template class
          @see Img
      **/
      virtual ImgI *deepCopyROI(ImgI *poDst = NULL) const=0;

      /// scales the image data in the image ROI into the destination images ROI
      /** this function is implemented in the Img-template class
          @see Img
      **/
      virtual ImgI *scaledCopyROI(ImgI *poDst = NULL, scalemode eScaleMode=interpolateNN) const=0;

      /// flips the image about the given axis into the destination image (IPP-OPTIMIZED)
       /** this function is implemented in the Img-template class
           @see Img
       **/ 
      virtual ImgI *flippedCopyROI(ImgI *poDst = NULL, axis eAxis = axisVert) const=0; 
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
      depth getDepth() const
        {
          FUNCTION_LOG("");
          return m_eDepth;
        }

      /// returns the current (color)-format of this image
      format getFormat() const
        {
          FUNCTION_LOG("");
          return m_eFormat;
        }

      /// returns the lenght of an image line in bytes (width*sizeof(Type))
      virtual int getLineStep() const = 0;

      /// returns if two images have same size, and channel count
      /** @param s size to test
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

      /// returns the images ROI offset (upper left corner)
      const Point& getROIOffset() const{
        FUNCTION_LOG("");
        return m_oROIOffset;
      }

      /// returns the images ROI size
      const Size& getROISize() const{
        FUNCTION_LOG("");
        return m_oROISize;
      }
     
      /// sets the image ROI to the given rectangle
      void setROI(const Rect &r){
        FUNCTION_LOG("");
        setROISize(Size(r.width,r.height));
        setROIOffset(Point(r.x,r.y));
      }
      
      /// sets the image ROI to the given offset and size
      void setROI(const Point &offset, const Size &size){
        FUNCTION_LOG("");
        setROISize(size);
        setROIOffset(offset);
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
      virtual void setChannels(int iNewNumChannels)=0;

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
      void setFormat(format eFormat);
      
      //@}
      /* }}} */

      //@{ @name image processing functions
      /* {{{ open */
      
      /// performs an inplace resize operation on the image (IPP-OPTIMIZED)
      /** @see Img*/
      virtual void scale(const Size& s, scalemode eScaleMode=interpolateNN)=0;
      /// performs an inplace mirror operation
      /** @see Img*/
      virtual void mirror(axis eAxis, bool bOnlyROI=false)=0;
      
      /// Scale the channel min/ max range to the new range tMin, tMax.
      /** @see Img*/
      virtual void scaleRange(float fNewMin=0.0, float fNewMax=255.0)=0;
      virtual void scaleRange(float fNewMin, float fNewMax, int iChannel)=0;

      /// Scales pixel values from given min/max values to new min/max values.
      /** @see Img */
      virtual void scaleRange(float tNewMin, float tNewMax, float tMin, float tMax)=0;
      virtual void scaleRange(float tNewMin, float tNewMax, float tMin, float tMax, int iChannel)=0;

      //@} 
      /* }}} */

      //@{ @name asImg<T> and convertTo<T>
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
          is given by the template parameter T.
          (For developers: The convertTo function builds the base function for
          other higher level funtions like deepCopy. Internally it calls the
          icl namespace function deepCopyChannel, which decides if data has to
          be copied or converted.)
          @param poDst destination image. If NULL, then a deep copy of the current
                       image is returned
          @see deepCopy
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
      @param eDepth depth of the image (depth8u or depth32f)
      @param iChannels channel count of the image (if -1, then the channel count is derived
                       from the set format e.g. formatRGB-> 3 channels )
      **/
      ImgI(const Size &s,
           format eFormat, 
           depth eDepth=depth8u,
           int iChannels=-1);
      
      /* }}} */

      /* {{{ data */

      /// channel count of the image
      int m_iChannels;

      /// size of image: width x height
      Size m_oSize;

      /// (color)-format associated with the images channels
      format m_eFormat;

      /// depth of the image (depth8 for icl8u/depth32 for icl32f)
      depth m_eDepth;

      // internal storage of the ROI parameters
      Point m_oROIOffset; //< ROI offset (upper left corner)
      Size  m_oROISize;   //< ROI size

      /* }}} */
    };
}

#endif 
