/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/SimpleMatrix.h                        **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef SIMPLE_MATRIX_H
#define SIMPLE_MATRIX_H

#include <ICLUtils/SmartPtr.h>
#include <algorithm>

namespace icl{
  
  /// Template parameter class for the SimpleMatrix template \ingroup UTILS
  template<class T>
  struct NullSimpleMatrixAlloc{
    static T create() { return T(0); }
  };

  /// Template parameter class for the SimpleMatrix template \ingroup UTILS
  template<class T>
  struct DefSimpleMatrixAlloc{
    static T create() { return T(); }
  };
  
  /// A Low-Weight Matrix representation for block aligned data \ingroup UTILS
  /** <h1>General</h1>
      The SimpleMatrix class is a utility class, for organizing
      a linear aligned data block, whiches content is associated
      with a matrix of column vectors. It provides a <b>very
      convenient access </b> to the data elements using the 
      "[][]-operator". Although the "[][]"-operator is a combined
      operator, consisting of two successive "[]"-operators, it
      is possible to call this operator because the array-operator 
      "[]" is defined for the SimpleMatrix class to return a pointer 
      to the according column vector (as T*), which can then again
      be accessed using another "[]" operator.

      <h1>Example</h1>
      <pre>
      typedef SimpleMatrix<float> mat;

      mat transpose(mat &src){
          ICLASSERT_RETURN( src.w() = src.h() ); // test for w==h
          mat dst(src.w(),src.h());              // ensures dst. size
          for(int x=0;x<src.w();x++){            // transposed copy
             for(int x=0;x<src.w();x++){
                dst[x][y] = src[y][x];
             }
          }
      }
      
      </pre>
  
      <h1>Performance</h1>
      Because the SimpleMatrix class is pure inline, an as the data
      access operator can easily be optimized to "pointer[y+h*x]",
      is is not slower - but much more convenient - to work with it.
  
      <h1>Data alignment</h1>
      <b>In contrast to ICL Image class Img, the data is aligned 
      horizontally</b> \n
      The following ascii image demonstrates this:
      <pre>
         +             +     +  -+-  -+-  -+-  +              +    +
         | m00 m10 m20 |     |   |    |    |   |              | mi0| 
      M =| m01 m11 m21 |  =  |   v0   v1   v2  |    with vi = | mi1|  
         | m02 m12 m22 |     |   |    |    |   |              | mi2|
         +             +     +  -+-  -+-  -+-  +              +    +
            matrix           set of column vectors
  
       corresponding data array:  data = [ |-v0-|  |-v1-|  |-v2-| ]     
                                       = [ m00 m01 m02 m10 .. m22 ] 
      </pre>

      <h1>Shared data concept</h1>
      As the ICL-image class Img, the SimpleMatrix class uses a 
      smart pointer (see SmartPtr class in the ICLUtils package)
      to facilitate an efficient memory management. All default
      copy mechanisms (copy constructor and assignment operator) 
      will only perform a shallow copy of the source SimpleMatrix
      object. If an explicit "deep copy" is demanded, one of the
      two <em>deepCopy</em>-functions must be used. 
  */
  template<class T, class Alloc=NullSimpleMatrixAlloc<T> >
  struct SimpleMatrix{
    
    /// Creates an empty SimpleMatrix w=h=0
    inline SimpleMatrix():
      m_iW(0),m_iH(0){}
    
    /// Creates a square matrix of size dim x dim filled with 0
    inline SimpleMatrix(int dim): 
      m_oData(new T[dim*dim]),m_iW(dim), m_iH(dim){
      std::fill(m_oData.get(),m_oData.get()+this->dim(),Alloc::create());
    }
  
    /// Creates a matrix of size w x h filled with 0
    inline SimpleMatrix(int w, int h): 
      m_oData(new T[w*h]),m_iW(w), m_iH(h){
      std::fill(m_oData.get(),m_oData.get()+dim(),Alloc::create());
    }
  
    /// Creates a matrix of size w x h, using given shared data
    inline SimpleMatrix(int w, int h, T *sharedData):
      m_oData(sharedData,false), m_iW(w), m_iH(h) {
    }
    
    /// returning a pointer to the column vector at given x offset
    inline T *operator[](int x){ return m_oData.get()+m_iH*x; }

    /// returning a pointer to the column vector at given x offset
    inline const T *operator[](int x) const{ return m_oData.get()+m_iH*x; }

    /// performs a deep copy of the matrix into a destination matrix
    inline void deepCopy(SimpleMatrix &m) const{
      m.m_iW = m_iW;
      m.m_iH = m_iH;
      m.m_oData = SmartPtr<T>(new T[dim()]);
      for(int i=0;i<dim();i++){
        m.data()[i] = T(data()[i]); // explicit call to the copy constructor
      }
    }
    
    /// creates a deep copy of the SimpleMatrix
    inline SimpleMatrix deepCopy() const{
      SimpleMatrix m;
      deepCopy(m);
      return m;
    }
    
    /// returns the data pointer
    inline T* data() { return m_oData.get(); }

    /// returns the data pointer (const version)
    inline const T* data() const { return m_oData.get(); }

    /// width of the matrix (max x index is w()-1
    inline int w() const { return m_iW; }

    /// height of the matrix (max y index is h()-1
    inline int h() const { return m_iH; }

    /// returns the number of matrix elements w*h
    inline int dim() const { return w()*h(); }

    /// returns the minumum element of the matrix (operator < must be defined on T)
    inline T min() const { return *std::min_element(data(),data()+dim()); }

    /// returns the maximum element of the matrix (operator < must be defined on T)
    inline T max() const { return *std::max_element(data(),data()+dim()); }

    /// iterator type used for begin() and end()
    typedef T* iterator;

    /// const iterator type used for begin() const and end() const
    typedef const T* const_iterator;
    
    /// returns an iterator to the first data element (note again: data layout is column major here)
    iterator begin() { return data(); }

    /// returns an iterator behind the last data element (note again: data layout is column major here)
    iterator end() { return data()+dim(); }

    /// returns an iterator to the first data element (note again: data layout is column major here) (const)
    const_iterator begin() const { return data(); }

    /// returns an iterator behind the last data element (note again: data layout is column major here) (const)
    const_iterator end() const { return data()+dim(); }
    
    private:
    /// Shared data pointer
    SmartPtr<T> m_oData;

    /// Matrix dimensions
    int m_iW,m_iH;
  };
  


}

#endif
