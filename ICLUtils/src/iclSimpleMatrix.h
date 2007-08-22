#ifndef SIMPLE_MATRIX_H
#define SIMPLE_MATRIX_H

#include <iclSmartPtr.h>
#include <algorithm>

namespace icl{
  template<class T>
  struct NullSimpleMatrixAlloc{
    static T create() { return T(0); }
  };

  template<class T>
  struct DefSimpleMatrixAlloc{
    static T create() { return T(); }
  };
  
  /// A Low-Weight Matrix representation for block aligned data
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
    inline T *operator[](int x) const{ return m_oData.get()+m_iH*x; }

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

    
    inline T min() const { return *std::min_element(data(),data()+dim()); }
    inline T max() const { return *std::max_element(data(),data()+dim()); }

    private:
    /// Shared data pointer
    SmartPtr<T> m_oData;

    /// Matrix dimensions
    int m_iW,m_iH;
  };
  


}

#endif
