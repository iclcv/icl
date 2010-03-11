/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLCore module of ICL                         **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_CHANNEL_H
#define ICL_CHANNEL_H

#include <ICLCore/Types.h>
#include <ICLUtils/Size.h>
#include <ICLUtils/Rect.h>
#include <ICLCore/ImgIterator.h>

namespace icl{


  /// Utility helper class for faster and more convenient access to single channel image data \ingroup IMAGE
  /** Yet, the (x,x,c)-operator of the Img<T> class is slow, because of hight computational overhead 
      due to the nessity of a channel lookup
      The (x,y,c)-operator needs first to lookup for the channel c in the channel 
      array of the image, before it can address a pixels value by dereferencing the shared 
      pointers wrapped data pointer. In many cases, The user works on images with a fixed channel count
      (mostly 1 or 3), and
      needs a simple (x,y)-data access operator, which translates an (x,y) pixel address into
      the raw data pointer address dataPointer[x+width*y].\n
      The Channel class was designed to facilitate this procedure. To avoid the arise of an
      The following code example shows how to replace the Imgs (x,y,c)-operator by using the 
      Channel template class:
      \code
      inline void copy_image(Img32f &A,Img32f &B){
        for(int x=0;x<1000;x++){                     // default version for an image copy
          for(int y=0;y<1000;y++){                   // each pixel is addressed by the
            A(x,y,0)=B(x,y,0);                       // (x,y,c)-operator (image size is fixed)
          }
        }
      }
      inline void copy_image(Img32f &A,Img32f &B){
        Channel32f a = A.extractChannel(0);         // new faster version (about 6.5-times
        Channel32f b = B.extractChannel(0);         // faster, than the above function
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
        Channel32f a[3]; A.extractChannels(a);
        Channel32f b[3]; B.extractChannels(b);
          for(int x=0;x<1000;x++){
            for(int y=0;y<1000;y++){            // more than one channel can be
              a[0](x,y)=b[0](x,y);              // stored in a fixed size array of
              a[1](x,y)=b[1](x,y);              // Channel32f structs; this allows
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
        Channel32f *a = new Channel32f[A.getChannels()];     // pre allocate buffer for
        Channel32f *b = new Channel32f[A.getChannels()];     // the channel Objects 
        A.extractChannels(a);                                // assign the channel objects with
        B.extractChannels(b);                                // the channel data of the images

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
      
      The Channel<T> template class is typedef'd for all current Img<T> classes
      Channel8u, Channel16s, Channel32s, Channel32f, Channel64f. 
      
  */
  template<class T>
  class Channel{
    mutable T *m_data;   
    mutable Size m_size;
    mutable Rect m_roi;
    
    public:
    /// Empty constructor (create an invalid Channel object)
    /** This constructor is necessary for the creation of uninitialized
        dynamic size arrays of Channel objects */
    Channel():m_data(0){}

    /// Copy an image channel (this could be exploited to violate const concept)
    inline Channel(const Channel &other){
      *this = other;      
    }    

    /// assignmet operator
    Channel<T> &operator=(Channel<T> &other){
      m_data = other.m_data;
      m_size = other.m_size;
      m_roi = other.m_roi;
      return *this;
    }

    /// assign operator (also for const channels)
    /** assign operator is const, as we use to const-ness to deny changes
        on the underlying image, but not on the Channel struct itself. Hence
        all class data is mutable */
    const Channel<T> &operator=(const Channel<T> &other) const{
      return *const_cast<Channel*>(this) = const_cast<Channel<T>&>(other);
    }

    /// main working function: returns a reference to the pixel at position (x,y)
    /** The data address is calculated by x+ImageWidth*y. <b>Note:</b> no checks
        are performed to ensure, that the position is valid!
        @param x x-Position
        @param y y-Position
        @return reference to the Pixel at (x,y) */
    inline T &operator()(int x, int y){ 
      return m_data[x+m_size.width*y]; 
    }
    
    /// main working function: returns const a reference to the pixel at position (x,y)
    /** The data address is calculated by x+ImageWidth*y. <b>Note:</b> no checks
        are performed to ensure, that the position is valid!
        @param x x-Position
        @param y y-Position
        @return reference to the Pixel at (x,y) */
    inline const T &operator()(int x, int y) const{  
      return m_data[x+m_size.width*y]; 
    }
    
    /// working function for linear pixel array access ( not const version)
    /** @param idx pixel array index 
        @return reference to the pixel at linear pixel offfset idx 
    **/
    inline T &operator[](int idx) { 
      return m_data[idx]; 
    }
    
    /// working function for linear pixel array access (const version)
    /** @param idx pixel array index 
        @return reference to the pixel at linear pixel offfset idx 
    **/
    inline const T &operator[](int idx) const { 
      return m_data[idx]; 
    }

    typedef T* iterator;

    /// const iterator type (just a const pointer)
    typedef const T* const_iterator;

    /// type definition for ROI iterator
    typedef ImgIterator<T> roi_iterator;

    /// type definition for a const ROI iterator
    typedef const ImgIterator<T> const_roi_iterator;
    // old    typedef constConstImgIterator<Type> const_iterator;
    


    /// returns the image iterator (equal to getData(channel))
    iterator begin(){
      return m_data;
    }
    
    /// returns the image iterator (equal to getData(channel)) (const)
    const_iterator begin() const{
      return m_data;
    }

    /// returns the image end-iterator (equal to getData(channel)+getDim())
    iterator end(){
      return m_data+getDim();
    }

    /// returns the image end-iterator (const)
    const_iterator end() const{
      return m_data+getDim();
    }

    /// returns the iterator for an images ROI
    inline roi_iterator beginROI(){
      return roi_iterator(m_data,getWidth(),getROI());
    } 

    /// returns the iterator for an images ROI (const)
    inline const_roi_iterator beginROI() const{
      return const_cast<Channel<T>*>(this)->beginROI();
    } 
    
    /// returns the end-iterator for an images ROI
    /** the returned iterator must not be incremented or decremented! */
    inline roi_iterator endROI() {
      return roi_iterator::create_end_roi_iterator(m_data,getWidth(),getROI());
    }

    /// returns the end-iterator for an images ROI (const)
    inline const_roi_iterator endROI() const{
      return const_roi_iterator::create_end_roi_iterator(m_data,getWidth(),getROI());
    }

    bool isNull() const { return !m_data; }
    
    /// returns the wrapped images width
    /** @return wrapped images width */
    inline int getWidth() const { return m_size.width; }

    /// returns the wrapped images height
    /** @return wrapped images height */
    inline int getHeight() const { return m_size.height; }

    /// retusn the wrapped images size
    inline const Size& getSize() const { return m_size; }
    
    /// returns the wrapped images dim = width*height
    /** @return wrapped images pixel count */
    inline int getDim() const { return m_size.getDim(); }
    
    /// returns the channel ROI
    inline const Rect &getROI() const { return m_roi; }

    /// returns the channel ROI offset
    inline Point getROIOffset() const { return m_roi.ul(); }

    /// returns the channel ROI size
    inline Size getROISize() const { return m_roi.getSize(); }

    /// returns the channel ROI width
    inline const Rect &getROIWidth() const { return m_roi.width; }

    /// returns the channel ROI height
    inline const Rect &getROIHeight() const { return m_roi.height; }

    /// returns the channel ROI X-Offset
    inline const Rect &getROIXOffset() const { return m_roi.x; }

    /// returns the channel ROI Y-Offset
    inline const Rect &getROIYOffset() const { return m_roi.y; }

    /// Sets a new ROI to this image channel (this does not affect the underlying images ROI)
    /** @param newROI new channel roi */
    void redefineROI(const Rect &newROI) const{
      m_roi = newROI;
    }
    
    /// mades Img<T> a friend to allow it construction of ImgChannels
    friend class Img<T>;

    private:    
    /// private constructor
    /** @param image image wich's channel should be wrapped 
        @param channelIndex index of the channel */
    inline Channel(const T *data, const Size &size, const Rect &roi):
    m_data(data),m_size(size),m_roi(roi){}

    inline Channel(T *data, const Size &size, const Rect &roi):
    m_data(data),m_size(size),m_roi(roi){}

  };
  
  /** \cond */
#define ICL_INSTANTIATE_DEPTH(D) typedef Channel<icl##D> Channel##D;
  ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
  /** \endcond */
}
#endif
