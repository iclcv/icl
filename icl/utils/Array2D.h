// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Size.h>
#include <icl/utils/Point.h>
#include <algorithm>
#include <memory>

namespace icl::utils {
  /// Simple 2D-Array class that provides shallow copy per default
  /** This class replaces the former SimpleMatrix class

      In contrast to the DynMatrix<T>-class template, the Array2D
      is designed for simple 2D data storage. The internal data
      layout is row-major i.e. the data is stored row-by-row.
      Array2D instances can be set up to have their own data, that
      is managed internally using a std::shared_ptr<T[]>, or they
      can be wrapped around an existing data data pointer. In the
      latter case, the given data is not copied deeply which
      implicates that the data must remain valid.

      \section SIMPLE Simplicity
      In order to keep the class interface simple, Array2D instances
      cannot be resized. Simply assign a new instance with new size.
      Due to the internally used smart pointer, this entails only
      a very small and constant overhead
  */
  template<class T>
  class Array2D{

    /// current dimension
    Size m_size;

    /// current data
    std::shared_ptr<T[]> m_data;

    public:

    /// iterator type (just a T*)
    using iterator = T*;

    /// const iterator type (just a const T*)
    using const_iterator = const T*;

    /// Creates null instance
    inline Array2D(){}

    /// Creates an uninitialized matrix of given size
    inline Array2D(int w,int h):
    m_size(w,h),m_data(new T[w*h]){}

    /// Creates an uninitialized matrix of given size
    inline Array2D(const Size &s):
    m_size(s),m_data(new T[s.getDim()]){}

    /// Creates an initialized matrix with given initializer
    template<class Init>
    inline Array2D(int w, int h, Init init):
    m_size(w,h),m_data(new T[w*h]){
      fill(init);
    }

    /// Creates an initialized matrix with given initializer
    template<class Init>
    inline Array2D(const Size &s, Init init):
    m_size(s),m_data(new T[s.getDim()]){
      fill(init);
    }

    /// Creates a matrix of size w x h, using given (optionally shared) data
    inline Array2D(int w, int h, T *data, bool deepCopy=false):
    m_size(w,h),m_data(deepCopy ? std::shared_ptr<T[]>(new T[w*h])
                                : std::shared_ptr<T[]>(data, [](T*){})){
      if(deepCopy) assign(data,data+w*h);
    }

    /// Creates a matrix of given Size, using given (optionally shared) data
    inline Array2D(const Size &s, T *data, bool deepCopy=false):
    m_size(s),m_data(deepCopy ? std::shared_ptr<T[]>(new T[s.getDim()])
                              : std::shared_ptr<T[]>(data, [](T*){})){
      if(deepCopy) assign(data,data+s.getDim());
    }

    /// Creates a matrix of size w x h, using given const data (always deep copy)
    inline Array2D(int w, int h,const T *data):
    m_size(w,h),m_data(new T[w*h]){
      assign(data,data+w*h);
    }

    /// Creates a matrix of given Size using given const data (always deep copy)
    inline Array2D(const Size &s,const T *data):
    m_size(s),m_data(new T[s.getDim()]){
      assign(data,data+s.getDim());
    }

    /// Creates a matrix of size w x h, initialized with content from given range
    template<class Iterator>
    inline Array2D(int w, int h,Iterator begin, Iterator end):
    m_size(w,h),m_data(new T[w*h]){
      assign(begin,end);
    }

    /// Creates a matrix of size w x h, initialized with content from given range
    template<class Iterator>
    inline Array2D(const Size &s,Iterator begin, Iterator end):
    m_size(s),m_data(new T[s.getDim()]){
      assign(begin,end);
    }


    /// fills the matrix with given value
    template<class Value>
    inline void fill(Value val){
      std::fill(begin(),end(),val);
    }

    /// Assigns the matrix from given range
    template<class Iterator>
    inline void assign(Iterator begin, Iterator end){
      std::copy(begin,end,this->begin());
    }

    /// returns the matrix width
    inline int getWidth() const { return m_size.width; }

    /// returns the matrix height
    inline int getHeight() const { return m_size.height; }

    /// returns the matrix dimension (width*height)
    inline int getDim() const { return m_size.getDim(); }

    /// returns the matrix size
    inline const Size &getSize() const { return m_size; }


    /// returns element at given linear index
    inline T &operator[](int idx) { return m_data.get()[idx]; }

    /// returns element at given linear index (const)
    inline const T &operator[](int idx) const{ return m_data.get()[idx]; }

    /// returns element at given x,y position
    inline T &operator()(int x,int y) { return m_data.get()[x+m_size.width*y]; }

    /// returns element at given x,y position (const)
    inline const T &operator()(int x, int y) const{ return m_data.get()[x+m_size.width*y]; }


    /// upper left matrix element iterator
    inline iterator begin() { return m_data.get(); }

    /// upper left matrix element iterator (const)
    inline const_iterator begin() const { return m_data.get(); }

    /// upper left matrix element iterator
    inline iterator end() { return m_data.get()+getDim(); }

    /// upper left matrix element iterator
    inline const_iterator end() const{ return m_data.get()+getDim(); }

    /// returns a deep copy of this matrix
    inline Array2D<T> deepCopy() const{
      return Array2D<T>(getSize(),begin(),end());
    }

    /// ensures that the contained data is not shared by other instances
    inline void detach(){
        if(m_data.use_count() > 1){
          std::shared_ptr<T[]> det(new T[getDim()]);
          std::copy(begin(),end(),det.get());
          m_data = det;
        }
    }

    /// returns the data pointer
    inline T* data() { return begin(); }

    /// returns the data pointer (const version)
    inline const T* data() const { return begin(); }

    /// returns the minumum element of the matrix (operator < must be defined on T)
    inline const T &minElem(Point *pos=0) const {
      int idx = static_cast<int>(std::min_element(begin(),end()) - begin());
      if(pos) *pos = Point(idx%getWidth(),idx/getWidth());
      return data()[idx];
    }

    /// returns the maximum element of the matrix (operator < must be defined on T)
    inline const T &maxElem(Point *pos=0) const {
      int idx = static_cast<int>(std::max_element(begin(),end()) - begin());
      if(pos) *pos = Point(idx%getWidth(),idx/getWidth());
      return data()[idx];
    }

    /// sets a new size
    void setSize(const Size &size){
      if(getSize() == size) return;
      m_size = size;
      m_data.reset(new T[size.getDim()]);
    }

    /// sets size and fills with new entries
    template<class Init>
    void setSize(const Size &size, const Init &init){
      setSize(size);
      fill(init);
    }
  };



  } // namespace icl::utils