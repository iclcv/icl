/*
ImgI.h

Written by: Michael Götting and Christof Elbrechter(2006)
University of Bielefeld
AG Neuroinformatik
{mgoettin,celbrech}@techfak.uni-bielefeld.de
*/

#ifndef ICLBASE_H
#define ICLBASE_H
    
#include <vector>
#include "ICLCore.h"
#include "ImgParams.h"

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
      ImgI* shallowCopy(ImgI** ppoDst = NULL) const;

      /// creates a shallow copy of selected channels of this image
      /** @param channelIndices vector containing channel indices to copy
          @param poDst destination image (if Null, a new one is created)*/
      ImgI* shallowCopy(const std::vector<int>& channelIndices, ImgI** ppoDst = NULL) const;

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

      /// returns all params in terms of a const ImgParams reference
      /** This enables the programmer to write
          <pre>
          imageA.setParams(imageB.getParams());
          </pre>
      */
      const ImgParams &getParams() const{ return m_oParams; }
      /// returns the size of the images

      // returns the images size
      const Size& getSize() const { return m_oParams.getSize(); }
      
      /// returns the pixelcount of each channel
      int getDim() const { return m_oParams.getDim(); }

      /// returns the channel count of the image
      int getChannels() const { return m_oParams.getChannels(); }

      /// returns the depth (depth8u or depth32f)
      depth getDepth() const { return m_eDepth; }

      /// returns the current (color)-format of this image
      format getFormat() const { return m_oParams.getFormat(); }

      /// returns the lenght of an image line in bytes (width*sizeof(Type))
      virtual int getLineStep() const = 0;

      /// returns if two images have same size, and channel count
      /** @param s size to test
          @param nChannels channel count to test
      **/
      bool isEqual(const Size &s, int nChannels) const
        {
          FUNCTION_LOG("isEqual("<<s.width<<","<< s.height << ","<< nChannels << ")");
          return (getSize() == s) && (getChannels() == nChannels);
        }
      
      /// checks if the image has the given parameters
      bool isEqual(const ImgParams &params){
        FUNCTION_LOG("");
        return m_oParams == params;
      }
      
      //@}

      /* }}} */
      
      //@{ @name ROI handling functions
      /* {{{ open */

      /// returns the images ROI rectangle
      const Rect &getROI() const{ return m_oParams.getROI(); }
      
      /// copies the current ROI into the given offset and size refereces
      void getROI(Point& offset, Size& size) const { m_oParams.getROI(offset,size); }

      /// returns the images ROI offset (upper left corner)
      Point getROIOffset() const{ return m_oParams.getROIOffset(); }

      /// returns the images ROI size
      Size getROISize() const{ return m_oParams.getROISize(); }

      /// returns the images ROI width
      int getROIWidth() const{ return m_oParams.getROIWidth(); }

      /// returns the images ROI height
      int getROIHeight() const{ return m_oParams.getROIHeight(); }
      
      /// returns the images ROI XOffset
      int getROIXOffset() const{ return m_oParams.getROIXOffset(); }

      /// returns the images ROI YOffset
      int getROIYOffset() const{ return m_oParams.getROIYOffset(); }

      /// returns the images width
      int getWidth() const { return m_oParams.getWidth(); }

      /// returns the images height
      int getHeight() const { return m_oParams.getHeight(); }
     
      /// sets all image parameters in order channels,size,format,roi
      void setParams(const ImgParams &params);

      /// sets the image ROI offset to the given value
      void setROIOffset(const Point &offset) { m_oParams.setROIOffset(offset); }
      
      /// sets the image ROI size to the given value
      void setROISize(const Size &size) { m_oParams.setROISize(size); }
      
      /// set both image ROI offset and size
      void setROI(const Point &offset, const Size &size){ m_oParams.setROI(offset,size); }
      
      /// sets the image ROI to the given rectangle
      void setROI(const Rect &roi) { m_oParams.setROI(roi); }

      /// checks, eventually adapts and finally sets the image ROI size
      /** @see ImgParams*/
      void setROIOffsetAdaptive(const Point &offset) { m_oParams.setROIOffsetAdaptive(offset); }
      
      /// checks, eventually adapts and finally sets the image ROI size
      /** @see ImgParams*/
      void setROISizeAdaptive(const Size &size){ m_oParams.setROISizeAdaptive(size); }

      /// checks, eventually adapts and finally sets the image ROI size
      /** @see ImgParams*/
      void setROIAdaptive(const Rect &roi) { m_oParams.setROIAdaptive(roi); }
     
      /// returns ROISize == ImageSize
      int hasFullROI() const { return m_oParams.hasFullROI(); }
      
      /// resets the image ROI to the whole image size with offset (0,0)
      void setFullROI() { m_oParams.setFullROI(); }
      
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
      virtual void setSize(const Size &s)=0;

      /// sets the format associated with channels of the image
      /**
      The channel count of the image is set to the channel count
      asociated with the set format, if they differ.
      E.g an image with one channel will have 3 channels after
      a setFormat(formatRGB) - call.
      @param eFormat new format value
      @see getChannelsOfFormat
      **/
      void setFormat(format fmt);
      
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
      void print(const std::string sTitle="image") const;
      //@}
      /* }}} */

      protected:

      /* {{{ Constructor  */

      /// Creates an ImgI object with specified image parameters 
      ImgI(depth d, const ImgParams& params);

      /* }}} */

      /* {{{ data */

      /// all image params
      /** the params class consists of 
          - image size
          - number of image channels
          - image format
          - image ROI      
      */
      ImgParams m_oParams;

      /// depth of the image (depth8 for icl8u/depth32 for icl32f)
      depth m_eDepth;


      /* }}} */
    };
}

#endif 
