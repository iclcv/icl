/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/Channel.h                          **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Types.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLUtils/Size.h>
#include <ICLUtils/Point.h>
#include <ICLUtils/Rect.h>
#include <ICLCore/ImgIterator.h>

namespace icl{
  namespace core{


    /// Utility helper class for faster and more convenient access to single channel image data \ingroup IMAGE
    /** Yet, the (x,x,c)-operator of the Img<T> class is slow, because of hight computational overhead
        due to the necessity of a channel lookup
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
      mutable utils::Size m_size;
      mutable utils::Rect m_roi;

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

      /// convenience function for point-based index access
      /** calls operator()(p.x,p.y) */
      inline T &operator()(const utils::Point &p){
        return operator()(p.x,p.y);
      }

      /// convenience function for point-based index access (const)
      /** calls operator()(p.x,p.y) const*/
      inline const T &operator()(const utils::Point &p) const{
        return operator()(p.x,p.y);
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

      /// index operator with linear interpolation
      /** The given source type must provide an index operator (so does e.g.
          icl::Point, icl::Point32f, icl::FixedMatrix and icl::FixedVector).
          There is no internal check so take care yourself that the given
          position is within the image rectangle
      */
      template<class Vec2D>
      inline T operator()(const Vec2D &p) const{
        float fX0 = p[0] - floor(p[0]), fX1 = 1.0 - fX0;
        float fY0 = p[1] - floor(p[1]), fY1 = 1.0 - fY0;
        const T* pLL = &operator()((int)p[0],(int)p[1]);
        float a = *pLL;        //  a b
        float b = *(++pLL);    //  c d
        pLL += getWidth();
        float d = *pLL;
        float c = *(--pLL);
        return fX1 * (fY1*a + fY0*c) + fX0 * (fY1*b + fY0*d);
      }

      /// typedef for a normal iterator (just a pointer)
      typedef T* iterator;

      /// const iterator type (just a const pointer)
      typedef const T* const_iterator;

      /// type definition for ROI iterator
      typedef ImgIterator<T> roi_iterator;

      /// type definition for a const ROI iterator
      typedef const ImgIterator<T> const_roi_iterator;

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
      inline const utils::Size& getSize() const { return m_size; }

      /// returns the wrapped images dim = width*height
      /** @return wrapped images pixel count */
      inline int getDim() const { return m_size.getDim(); }

      /// returns the channel ROI
      inline const utils::Rect &getROI() const { return m_roi; }

      /// returns the channel ROI offset
      inline utils::Point getROIOffset() const { return m_roi.ul(); }

      /// returns the channel ROI size
      inline utils::Size getROISize() const { return m_roi.getSize(); }

      /// returns the channel ROI width
      inline int getROIWidth() const { return m_roi.width; }

      /// returns the channel ROI height
      inline int getROIHeight() const { return m_roi.height; }

      /// returns the channel ROI X-Offset
      inline int getROIXOffset() const { return m_roi.x; }

      /// returns the channel ROI Y-Offset
      inline int getROIYOffset() const { return m_roi.y; }

      /// Sets a new ROI to this image channel (this does not affect the underlying images ROI)
      /** @param newROI new channel roi */
      void redefineROI(const utils::Rect &newROI) const{
        m_roi = newROI;
      }

      /// deeply copies the channel data (no roi support here)
      void deepCopy(Channel<T> &other) const throw (utils::ICLException){
        if(m_size != other.m_size){
          throw utils::ICLException("Channel::deepCopy: sizes differ!");
        }
        icl::core::copy(m_data,m_data+m_size.getDim(), other.m_data);
      }

      /// deeply converts the channel data (no roi support here)
      template<class OtherT>
      void convert(Channel<OtherT> &other) const throw(utils::ICLException){
        if(m_size != other.m_size){
          throw utils::ICLException("Channel::convert: sizes differ!");
        }
        icl::core::convert(m_data,m_data+m_size.getDim(), other.m_data);
      }


      /// mades Img<T> a friend to allow it construction of ImgChannels
      friend class Img<T>;

      private:
      /// private constructor
      /** @param data data pointer (image) wich's channel should be wrapped
          @param size size of the data
	  @param roi roi of the data (image)*/
      inline Channel(const T *data, const utils::Size &size, const utils::Rect &roi):
      m_data(data),m_size(size),m_roi(roi){}

      inline Channel(T *data, const utils::Size &size, const utils::Rect &roi):
      m_data(data),m_size(size),m_roi(roi){}

    };

    /** \cond */
  #define ICL_INSTANTIATE_DEPTH(D) typedef Channel<icl##D> Channel##D;
    ICL_INSTANTIATE_ALL_DEPTHS
  #undef ICL_INSTANTIATE_DEPTH
    /** \endcond */
  } // namespace core
}
