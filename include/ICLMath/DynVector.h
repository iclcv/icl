/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMath/DynVector.h                            **
** Module : ICLMath                                                **
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

#pragma once

#include <ICLMath/DynMatrix.h>

namespace icl{
  namespace math{
    
    /// Extension class for the DynMatrix<T> template, that restricts the the matrix column count to 'one'
    template<class T>
    struct DynColVector : public DynMatrix<T>{
      /// creates a column vector from given matrix column
      DynColVector(const typename DynMatrix<T>::DynMatrixColumn &column):
        DynMatrix<T>(column){}
  
      /// Default empty constructor creates a null-vector
      inline DynColVector():DynMatrix<T>(){};
  
      /// Creates a column vector with given dimension (and optional initialValue)
      inline DynColVector(unsigned int dim,const T &initValue=0) throw (InvalidMatrixDimensionException) :
        DynMatrix<T> (1,dim,initValue){}
  
      /// Create a column vector with given data
      /** Data can be wrapped deeply or shallowly. If the latter is true, given data pointer
          will not be released in the destructor, i.e. the data ownership is not passed to the 
          DynColumnVector instance*/
      inline DynColVector(unsigned int dim, T *data, bool deepCopy=true) throw (InvalidMatrixDimensionException) :
        DynMatrix<T>(1,dim,data,deepCopy){}
  
  
      /// Creates column vector with given data pointer and dimsion (const version: deepCopy only)
      inline DynColVector(unsigned int dim,const T *data) throw (InvalidMatrixDimensionException) :
        DynMatrix<T>(1,dim,data){}
  
      /// Default copy constructor (the source matrix column count must be 'one')
      inline DynColVector(const DynMatrix<T> &other) throw (InvalidMatrixDimensionException):
        DynMatrix<T>(other){
        ICLASSERT_THROW(DynMatrix<T>::cols() == 1, 
                        InvalidMatrixDimensionException("DynColVector(DynMatrix): source matrix has more than one column"));
      }
      /// assignment operator (the rvalue's column count must be one)
      inline DynColVector<T> &operator=(const DynMatrix<T> &other) throw (InvalidMatrixDimensionException){
        DynMatrix<T>::operator=(other);
        ICLASSERT_THROW(DynMatrix<T>::cols() == 1, 
                        InvalidMatrixDimensionException("DynColVector = DynMatrix: source matrix has more than one column"));
        return *this;
      }
      /// adapts the vector dimension 
      /** overwrites setBounds from the parent matrix class to prevent the vector from being resized to matrix bounds */
      inline void setBounds(unsigned int dim, bool holdContent=false, const T &initializer=0) throw (InvalidMatrixDimensionException){
        DynMatrix<T>::setBounds(1,dim,holdContent,initializer);
      }
  
      /// adapts the vector dimension 
      inline void setDim(unsigned int dim, bool holdContent= false, const T &initializer=0) throw (InvalidMatrixDimensionException){
        setBounds(dim,holdContent,initializer);
      }
    };
  
    template<class T>
    struct DynRowVector : public DynMatrix<T>{
      /// Default empty constructor creates a null-vector
      inline DynRowVector():DynMatrix<T>(){}
  
      /// Creates a row vector with given dimension (and optional initialValue)
      inline DynRowVector(unsigned int dim,const T &initValue=0) throw (InvalidMatrixDimensionException) :
        DynMatrix<T> (dim,1,initValue){}
  
      /// Create a row vector with given data
      /** Data can be wrapped deeply or shallowly. If the latter is true, given data pointer
          will not be released in the destructor, i.e. the data ownership is not passed to the 
          DynColumnVector instance*/
      inline DynRowVector(unsigned int dim, T *data, bool deepCopy=true) throw (InvalidMatrixDimensionException) :
        DynMatrix<T>(dim,1,data,deepCopy){}
  
  
      /// Creates column vector with given data pointer and dimsion (const version: deepCopy only)
      inline DynRowVector(unsigned int dim,const T *data) throw (InvalidMatrixDimensionException) :
        DynMatrix<T>(dim,1,data){}
  
      /// Default copy constructor (the source matrix row count must be 'one')
      inline DynRowVector(const DynMatrix<T> &other) throw (InvalidMatrixDimensionException):
        DynMatrix<T>(other){
        ICLASSERT_THROW(DynMatrix<T>::rows() == 1,
                        InvalidMatrixDimensionException("DynRowVector(DynMatrix): source matrix has more than one rows"));
      }
      /// assignment operator (the rvalue's column count must be one)
      inline DynRowVector<T> &operator=(const DynMatrix<T> &other) throw (InvalidMatrixDimensionException){
        DynMatrix<T>::operator=(other);
        ICLASSERT_THROW(DynMatrix<T>::rows() == 1,
                        InvalidMatrixDimensionException("DynRowVector = DynMatrix: source matrix has more than one rows"));
        return *this;
      }
  
      /// adapts the vector dimension 
      /** overwrites setBounds from the parent matrix class to prevent the vector from being resized to matrix bounds */
      inline void setBounds(unsigned int dim, bool holdContent=false, const T &initializer=0) throw (InvalidMatrixDimensionException){
        DynMatrix<T>::setBounds(dim,1,holdContent,initializer);
      }
  
      /// adapts the vector dimension 
      inline void setDim(unsigned int dim, bool holdContent= false, const T &initializer=0) throw (InvalidMatrixDimensionException){
        setBounds(dim,holdContent,initializer);
      }
  
    };
  } // namespace math
}

