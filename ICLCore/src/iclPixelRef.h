#ifndef ICL_PIXEL_REF_H
#define ICL_PIXEL_REF_H

#include <iclTypes.h>
#include <iclSmartPtr.h>
#include <iclException.h>
#include <iclMacros.h>
#include <iclFixedMatrix.h>
#include <iclClippedCast.h>

namespace icl{
  /// Pixel-Type class for copying image pixles to image pixels
  /** PixelRef instances are returned by an images operator()(int x, int y)
      It allows to write:
      <code>
      Img8u a = ...;
      Img8u b = ...;
      
      a(5,6) = b(3,2);
      </code>
      Furthermore it provides a list of setter functions, which allow to set up
      image pixels form other data types like vectors, ranges (iterator based)
      and even icl::FixedMatrix.

      Most of the functions are channel count save, i.e. they throw an ICLException
      if source channel count is not compatible.
  */
  template<class T>
  class PixelRef{
    
    /// Internal data
    std::vector<T*> m_data;
    
    public:
    
    /// Empty constructor, create a null pixel ref with 0 length
    inline PixelRef(){}
    
    /// returs whether this instance is null (created with the empty constructor)
    inline bool isNull() const { return !m_data->size(); }

    /// single constructor to create a pixelref instance
    /** This should not be used manually. Rather you should use Img<T>'s operator()(int x, int y) */
    inline PixelRef(int x, int y, int width, std::vector<SmartPtr<T> > &data):m_data(data.size()){
      int offs = x+width*y;
      for(unsigned int i=0;i<data.size();++i){
        this->m_data[i] = data[i].get()+offs;
      }
    }
    
    /// PixelRef copy constructor (copies the reference, not the values)
    inline PixelRef(const PixelRef &other):m_data(other.m_data){}

    /// assignment operator which copies the values (most common)
    /** This operator allows to write 
        <code>
        imageA(x,y) = imageB(a,b);
        </code>
    */
    inline PixelRef &operator=(const PixelRef &other)throw (ICLException){
      ICLASSERT_THROW(other.m_data.size() == m_data.size(),ICLException("incompatible channel count"));
      for(unsigned int i=0;i<m_data.size();++i){
        *m_data[i] = *other.m_data[i];
      }
      return *this;
    }
    
    /// assigns reference pixel values from vector data
    inline PixelRef &operator=(const std::vector<T> &vec)throw (ICLException){
      ICLASSERT_THROW(vec.size() == m_data.size(),ICLException("incompatible channel count"));
      for(unsigned int i=0;i<m_data.size();++i){
        *m_data[i] = vec[i];
      }
      return *this;
    }

    /// assigns reference pixel values from FixedMatrix data
    /** This can e.g. be used to assign an icl::Color value to an image pixel
        (Color is a part of the ICLCC package, and it is typedef'ed to some
        FixedMatrix type)
        
        <code>
        imageA(x,y) = Color(2,3,4);
        </code>
    */
    template<class MT,unsigned int COLS,unsigned int ROWS>
    inline PixelRef &operator=(const FixedMatrix<MT,COLS,ROWS> &mat)throw (ICLException){
      ICLASSERT_THROW((m_data.size() == FixedMatrix<MT,COLS,ROWS>::DIM), ICLException("channel count and matrix dim are incompatible"));
      for(unsigned int i=0;i<m_data.size();++i){
        *m_data[i] = clipped_cast<MT,T>(mat[i]);
      }
      return *this;
    }
    
    /// copies image data into a std::vector
    inline std::vector<T> asVec() const{
      std::vector<T> v(m_data.size());
      for(unsigned int i=0;i<m_data.size();++i){
        v[i] = *m_data[i];
      }
      return v;
    }
    
    /// sets up the first index (unsafe)
    inline void set(const T &v0) { *m_data[0] = v0; }

    /// sets up the first two indices (unsafe)
    inline void set(const T &v0, const T&v1) { set(v0); *m_data[1] = v1; }

    /// sets up the first three indices (unsafe)
    inline void set(const T &v0, const T&v1, const T&v2) { set(v0,v1); *m_data[2] = v2; }

    /// sets up the first four indices (unsafe)
    inline void set(const T &v0, const T&v1, const T&v2, const T &v3) { set(v0,v1,v2); *m_data[3] = v3; }
    
    /// assigns a ranges contents to the pixel data
    /** An exception is only thrown of the given range is too short*/
    template<class ForwardIterator>
    inline void setFromRange(ForwardIterator begin, ForwardIterator end) throw (ICLException){
      for(unsigned int i=0;i<m_data.size();++i,++begin){
        if(begin == end) throw ICLException("Range is longer then channel count");
        *m_data[i] = *begin;
      }
    }
    
    /// references a single element (safe)
    T &operator[](unsigned int channel) throw (ICLException){
      ICLASSERT_THROW(channel < m_data.size(),ICLException("invalid channel index"));
      return *m_data[channel];
    }
    
    /// references a single element (const) (safe)
    const T &operator[](unsigned int channel) const throw (ICLException){
      ICLASSERT_THROW(channel < m_data.size(),ICLException("invalid channel index"));
      return *m_data[channel];
    }
    
    /// returns the channel count
    int getChannels() const {
      return (int)m_data->size();
    }
  };
}
#endif
