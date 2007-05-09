#ifndef ICL_IMG_CHANNEL_H
#define ICL_IMG_CHANNEL_H

#include "iclImg.h"

namespace icl{
  /// Utility helper class for faster and more convenient access to single channel image data
  /** Yet, the (x,x,c)-operator of the Img<T> class is slow, because of its high channel lookup
      overhead. The (x,y,c)-operator needs first to lookup for the channel c in the channel 
      array of that object, before it can address a pixels value by dereferencing the shared 
      pointers wrapped data pointer. In many cases, The programmer works on fixed channel images, and
      needs a simple (x,y)-data access operator, which translates an (x,y) pixel address into
      the raw data pointer address dataPointer[x+width*y].\n
      The ImgChannel class was designed to facilitate this procedure. To avoid the arise of an
      exploit for accessing const Img data as un-const, ImgChannels can only be created by the
      the template function pickChannel available (as const and unconst version) in the icl 
      namespace, if the header "iclImgChannel.h" was included.
      The following code example shows how to replace the Imgs (x,y,c)-operator by using the 
      ImgChannel template class:
      \code
      inline void copy_image(Img32f &A,Img32f &B){
        for(int x=0;x<1000;x++){                     // default version for an image copy
          for(int y=0;y<1000;y++){                   // each pixel is addressed by the
            A(x,y,0)=B(x,y,0);                       // (x,y,c)-operator (image size is fixed)
          }
        }
      }
      inline void copy_image(Img32f &A,Img32f &B){
        ImgChannel32f a = pickChannel(&A,0);        // new faster version (about 6.5-times
        ImgChannel32f b = pickChannel(&B,0);        // faster, than the above function
        for(int x=0;x<1000;x++){                    // pixels are addressed by the 
          for(int y=0;y<1000;y++){                  // ImgChannel objects
            a(x,y)=b(x,y);
          }
        }
      }
      \endcode
      The lower function is about 6.5 times faster than the upper one!
      
      When working on multi channel images with a fixed channel count, the
      advantage of using the ImgChannel class becomes even greater:
      \code
      inline void copy_3_channels(ImgQ &A,ImgQ &B){
        for(int x=0;x<1000;x++){
          for(int y=0;y<1000;y++){
            A(x,y,0)=B(x,y,0);
            A(x,y,1)=B(x,y,1);
            A(x,y,2)=B(x,y,2);
          }
        }
      }

      inline void copy_3_channels(ImgQ &A,ImgQ &B){
        ImgChannel32f a[] = { pickChannel(&A,0),pickChannel(&A,1),pickChannel(&A,2) };
        ImgChannel32f b[] = { pickChannel(&B,0),pickChannel(&B,1),pickChannel(&B,2) };
          for(int x=0;x<1000;x++){
            for(int y=0;y<1000;y++){            // more than one channel can be
              a[0](x,y)=b[0](x,y);              // stored in a fixed size array of
              a[1](x,y)=b[1](x,y);              // ImgChannel32f structs; this allows
              a[2](x,y)=b[2](x,y);              // direct access to the channel data pointer
            }
          }
        }
      }
      \endcode
      In The above example, the lower version of the copy function is even 7 times as fast
      as the upper version. At last, we want to examine the speed advantage for dynamic
      channel count and image size. The corresponding code is illustrated in the following 
      code example:
      \code
      
      inline void dynamic_image_copy(ImgQ &A,ImgQ &B){
        for(int x=0;x<A.getWidth();x++){
          for(int y=0;y<A.getHeight();y++){
            for(int c=0;c<A.getChannels();c++){         // here, a dynamic channel loop
              A(x,y,c)=B(x,y,c);                        // is implemented
            }
          }
        }
      }
      
      inline void dynamic_image_copy_2(ImgQ &A,ImgQ &B){
        ImgChannel32f *a = new ImgChannel32f[A.getChannels()];     // pre allocate buffer for
        ImgChannel32f *b = new ImgChannel32f[A.getChannels()];     // the channel Objects 
        for(int c=0;c<A.getChannels();c++){
          a[c] = pickChannel(&A,c);                                // assign the channel objects with
          b[c] = pickChannel(&B,c);                                // the channel data of the images
        }
        for(int x=0;x<A.getWidth();x++){
          for(int y=0;y<A.getHeight();y++){
            for(int c=0;c<A.getChannels();c++){                    // again use a dynamic channel loop, 
              a[c](x,y)=b[c](x,y);                                 // but make use of the channel struct
            }
          }
        }
        delete [] a;                                              // release the channel structs
        delete [] b;
      }
      \endcode
      In this example, the lower function is only 3 times faster, than the above one. 
      Here, the advantage is not as large as in the above examples, but an acceleration 
      factor of 3 should not be neglected.\n
      
      The ImgChannel<T> template class is typedef'd for all current Img<T> classes
      ImgChannel8u, ImgChannel16s, ImgChannel32s, ImgChannel32f, ImgChannel64f. 
      
  */
  template<class T>
  class ImgChannel{
    public:
    /// Empty constructor (create an invalid ImgChannel object)
    /** This constructor is necessary for the creation of uninitialized
        dynamic size arrays of ImgChannel objects */
    ImgChannel():m_ptData(0),m_iWidth(0),m_iHeight(0){}
    
    /// friend function to create ImgChannel objects (not const version)
    /** @see icl::pickChannel<T>(icl::Img<T>*,int) */
    template <class T2>
    friend ImgChannel<T2> pickChannel(Img<T2>*,int);

    /// friend function to create ImgChannel objects (const version)
    /** @see icl::pickChannel<T>(const icl::Img<T>*,int) */
    template <class T2>
    friend const ImgChannel<T2> pickChannel(const Img<T2>*,int);
    
    /// main working function: returns a reference to the pixel at position (x,y)
    /** The data address is calculated by x+ImageWidth*y. <b>Note:</b> no checks
        are performed to ensure, that the position is valid!
        @param x x-Position
        @param y y-Position
        @return reference to the Pixel at (x,y) */
    inline T &operator()(int x, int y){  return m_ptData[x+m_iWidth*y]; }
    
    /// main working function: returns const a reference to the pixel at position (x,y)
    /** The data address is calculated by x+ImageWidth*y. <b>Note:</b> no checks
        are performed to ensure, that the position is valid!
        @param x x-Position
        @param y y-Position
        @return reference to the Pixel at (x,y) */
    inline const T &operator()(int x, int y) const{  return m_ptData[x+m_iWidth*y]; }
    
    /// returns the corresponding data pointer 
    /** @return current wrapped data pointer */
    inline T *getData(){ return m_ptData; }

    /// returns the corresponding data pointer (const version) 
    /** @return current wrapped data pointer */
    inline const T *getData() const { return m_ptData; }
    
    /// returns the wrapped images width
    /** @return wrapped images width */
    inline int getWidth() const { return m_iWidth; }

    /// returns the wrapped images height
    /** @return wrapped images height */
    inline int getHeight() const { return m_iHeight; }
    
    
    private:    
    /// private constructor (called by the friend functions pickChannel)
    /** @see the pickChannel function available in the icl namespace
        @param image image wich's channel should be wrapped 
        @param channelIndex index of the channel */
    inline ImgChannel(const Img<T> *image,int channelIndex){
      ICLASSERT( image && image->getChannels() >channelIndex );
      m_ptData = const_cast<T*>(image->getData(channelIndex));
      m_iWidth = image->getWidth();
      m_iHeight = image->getHeight();
    }
    
    T *m_ptData;   /**< wrapped image data pointer */
    int m_iWidth;  /**< wrapped images width */
    int m_iHeight; /**< wrapped images height */
  };
  
  /// @{ @name ImgChannel<T> creator functions

  /// Creation function for ImgChannel objects (not const version)
  /** This function creates an ImgChannel object of a given image and channel index.
      @param image image which's channel should be wrapped 
      @param channelIndex index of the channel 
      @return ImgChannel object 
  */
  template<class T>
  inline ImgChannel<T> pickChannel(Img<T> *image,int channelIndex){
    return ImgChannel<T>(image,channelIndex);
  }
  
  /// Creation function for ImgChannel objects (const version)
  /** This function creates an ImgChannel object of a given image and channel index.
      @param image image wich's channel should be wrapped 
      @param channelIndex index of the channel 
      @return ImgChannel object 
  */
  template<class T>
  inline const ImgChannel<T> pickChannel(const Img<T> *image,int channelIndex){
    return ImgChannel<T>(image,channelIndex);
  }
  
  /// @} 
  
  
#define ICL_INSTANTIATE_DEPTH(D) typedef ImgChannel<icl##D> ImgChannel##D;
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  
}
#endif
