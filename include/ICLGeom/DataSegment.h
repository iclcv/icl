/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/DataSegment.h                          **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
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

#ifndef ICL_DATA_SEGMENT_H
#define ICL_DATA_SEGMENT_H

#include <ICLUtils/FixedVector.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLGeom/DataSegmentBase.h>

namespace icl{
  

  /// The DataSegment class defines a strided data segment (or 1D or 2D ordred array of vectors)
  /** Each data segment is defined by 
      - the type of it's elements (template parameter T)
      - the dimension of single vectors
      - the number of vector elements 
      - a flag, whether the the elements are 2D organized or not 
  
      Each single vector is assumed to be packed in memory (e.g. the vector element 
      stride is always <tt>sizeof(T)</tt>) the distance between two vector entries
      can be set to an arbitrary constant value (stride). Optionally, an organizedWidth
      value > 0 can be given, which set up the DataSegement to be 2D-organized.
      
      \section _IN_ Integration with the DataSegment base class
      
      The DataSegmentBase class is used as a generic interface for
      arbitrary DataSegment implementations. Since the DataSegment templates have no
      own data, a DataSegmentBase instance can simply be reinterpreted as a specific
      DataSegment-template version. For type-safe converions, the DataSegmentBase::as
      method can be used. This will reinterpret itself as a special DataSegment-template
      version and it will also use the runtime-information of the DataSegmentBase to
      check whether this conversion is allowed.
      
      \section _1D_ 1D Vector types
      
      The DataSegment class is specialized for 1D-vector types. In
      this case the access operators (index-operator for linear
      access, the (x,y)-operator for 2D-organized access), are
      adapted to not return T-references directly rather than
      <tt>FixedColVector<T,1></tt> references.
      
      \section _EX_ Examples
      
      Here are two examples that show some basic use of the DataSegment class.
      Please note, that usually, the programmes does not need to create DataSegments
      manually. Instead, e.g. the PointCloudObject-classes provide select-methods,
      that automatically create DataSegment instances.

      \code
#include <ICLQuick/Common.h>
#include <ICLGeom/GeomDefs.h>
#include <DataSegment.h>


int main(){
  Vec data[5]; // linear data xyzh,xyzh,...
  
  // wrapper segment for the first 3 floats of every entry/row
  DataSegment<float,3> xyz(&data[0][0], sizeof(Vec), 5);

  // special segment for the last float of the segments
  DataSegment<float,1> h(&data[0][3], sizeof(Vec), 5);
  
  // xyz.getDim() returns the number of wrapped elements
  for(int i=0;i<xyz.getDim();++i){
    xyz[i] = FixedColVector<float,3>(1,2,3); // vector based assginment
    h[i] = 1;       // vector dim. is 1 -> scalar assignment
  }
  
  for(int i=0;i<5;++i){
    std::cout << "data[" << i << "]: " << data[i].transp() << std::endl;
  }
}
      \endcode 
      Here is a more complex example, that shows how to use
      2D-organized data segments:
      
      \code
#include <ICLQuick/Common.h>
#include <ICLGeom/GeomDefs.h>
#include <DataSegment.h>

// utility structure used for demonstration
struct DataPoint{
  Vec xyz;        // 4 floats xyzh
  icl8u r,g,b,a;  // 4 single uchars for r,g,b and alpha
  Vec normal;     // 4 floats for a normal vector
  int label;      // a single int vector
};

int main(){
  Array2D<DataPoint> array(4,3);

  DataSegment<float,3> xyz(&array(0,0).xyz[0],sizeof(DataPoint), 12, 4);
  DataSegment<float,1> h(&array(0,0).xyz[3],sizeof(DataPoint), 12, 4);
  DataSegment<icl8u,4> rgba(&array(0,0).r,sizeof(DataPoint), 12, 4);
  DataSegment<float,4> n(&array(0,0).normal[0],sizeof(DataPoint), 12, 4);
  DataSegment<int,1>   l(&array(0,0).label,sizeof(DataPoint), 12, 4);

  for(int y=0;y<3;++y){    
    for(int x=0;x<4;++x){
      xyz(x,y) = FixedColVector<float,3>(x,y,0);
      h(x,y) = 1;              // scalar assignment
      rgba(x,y) = FixedColVector<icl8u,4>(x,y,x+y,255);
      n(x,y) = Vec(1,0,0,1);
      l(x,y) = x > y ? 1 : 0;  // scalar assignment
    }
  }
}
      \endcode
      
      
  */
  template<class T,int N>
  struct DataSegment : public DataSegmentBase{
    /// vector typedef
    typedef FixedColVector<T,N> VectorType;
    
    /// Constructor (basically passes all parameters to the Base class)
    inline DataSegment(T *data=0, size_t stride=0, size_t numElements=0, icl32s organizedWidth=-1):
    DataSegmentBase(data,stride,numElements,organizedWidth,icl::getDepth<T>(),N){}
    
    /// linear index operator
    inline FixedColVector<T,N> &operator[](int idx) {
      return *reinterpret_cast<FixedColVector<T,N>*>(data +idx*stride);
    }
    
    /// linear index operator (const)
    inline const FixedColVector<T,N> &operator[](int idx) const{
      return const_cast<DataSegment<T,N> *>(this)->operator[](idx);
    }
    
    /// 2D-index operator (only for organized data segments)
    inline FixedColVector<T,N> &operator()(int x, int y) {
     return operator[](x + organizedWidth * y );
    }
    
    /// 2D-index operator (only for organized data segments, const)
    inline const FixedColVector<T,N> &operator()(int x, int y) const{
      return operator[](x + organizedWidth * y );
    }
  };

  /// template specialization for data-segments, where each entry is just 1D
  /** If the vector entries are 1D only, no extra vector struct is
      created and returned for the single vector elements. Instead,
      all access functions <tt>operator[idx]</tt> and <tt>operator(x,y)</tt> are
      will just return T-references instead of FixedColVector<T,1> */
  template<class T>
  struct DataSegment<T,1> : public DataSegmentBase{
    /// vector typedef
    typedef T VectorType;

    /// Constructor (basically passes all parameters to the Base class)
    inline DataSegment(T *data=0, size_t stride=0, size_t numElements=0, icl32s organizedWidth=-1):
    DataSegmentBase(data,stride,numElements,organizedWidth,icl::getDepth<T>(),1){}

    /// linear index operator (specialized to return a T& directly)
    inline T &operator[](int idx) {
      return *reinterpret_cast<T*>(data +idx*stride);
    }
    
    /// linear index operator (specialized to return a T& directly, const)
    inline const T &operator[](int idx) const{
      return const_cast<DataSegment<T,1> &>(this)->operator[](idx);
    }

    /// 2D-index operator (only for organized data segments, specialized to return a T& directly)
    inline T &operator()(int x, int y) {
     return operator[](x + organizedWidth * y );
    }
    
    /// 2D-index operator (only for organized data segments, specialized to return a T& directly, const)
    inline const T &operator()(int x, int y) const{
      return operator[](x + organizedWidth * y );
    }
  };

  /** \cond */
  template<class T, int N>
  const DataSegment<T,N> &DataSegmentBase::as() const{
    if(dataDepth != icl::getDepth<T>()) throw ICLException("invalid cast of data segment (depth is wrong)");
    if(elemDim != N) throw ICLException("invalid cast of data segment (dimension is wrong)");
    return (DataSegment<T,N> &)(*this);
  }
  /** \endcond */

}

#endif

