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

using namespace std;

namespace ICL {
  

  /// forward declaration of the ICL-class
  template<class T> class ICL;

  /// typedef for 8bit integer images
  typedef ICL<iclbyte> ICL8u;

  /// typedef for 32bit float images
  typedef ICL<iclfloat> ICL32f;
  
  /// Base ICL-Class
  /**
  \section the class
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
      /* {{{ getter functions */

      ///@name getter functions
      //@{
      /// return the images width
      /**
      if the argument iChannel is defined, then the width of
      a specific channel is returned. This allows derived classes
      to contain channels with different resolutions
      @param iChannel determines the channel which's width should be
      returned.
      @return width of the image/selected channel
      **/
      int getWidth(int iChannel = 0)  const
        {
          return m_iWidth;
        }
  
      /// return the images height
      /**
      if the argument iChannel is defined, then the width of
      a specific channel is returned. This allows derived classes
      to contain channels with different resolutions
      @param iChannel determines the channel which's height should be
      returned.
      @return height of the image/selected channel
      **/

      int getHeight(int iChannel = 0) const
        {
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
  
      /// returns a pointer to first data element
      /** 
      This function is implemented by the derived template 
      class ICL<class T>
      @param iChannel determines the channel of which the
      data-pointer should be returned
      @return pointer to the fist data element**/
      virtual void* getDataPtr(int iChannel) const = 0;
      //@}

      /* }}} */

      /* {{{ accessing underlying r_e_a_l classes */

      //@{ @name accessing underlying r_e_a_l classes
      /// returns an ICL8u* intstance of this image
      /** 
      @return ICL8u* instance of this image
      **/
      ICL8u* asIcl8u()
        {
          return reinterpret_cast<ICL8u*>(this);
        }
  

      /// returns an ICL32f* intstance of this image
      /**
      @return ICL32f* instance of this image
      **/
      ICL32f* asIcl32f()
        {
          return reinterpret_cast<ICL32f*>(this);
        }
      //@}

      /* }}} */

      /* {{{ setter functions */

      ///@name setter functions
      //@{
      /// sets the format associated with channels of the image
      /**
      The new format value must be compatible to the channel count.
      For example formatRGB may is only compatible to 3-channel images.
      @param eFormat new format value
      @see getChannelsOfFormat
      **/
      void setFormat(iclformat eFormat);
      //@}

      /* }}} */

      protected:

      /* {{{ Constructors and Destructors */

      ///@name Constructors and Destructor
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
      
      /// Destructor
      virtual ~ICLBase();
      
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
