/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLCore/DataSegmentBase.h                  **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
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
#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/Exception.h>
#include <algorithm>

namespace icl{
  //forward declare PointCloudObjectBase to friend later
  namespace geom {
  class PointCloudObjectBase;
  }
  namespace core{

    /** \cond */
    template<class T,int N> struct DataSegment;
    /** \endcond */

    /// Abstract data segment class
    /** The DataSegmentBase class is a generic base class for interleaved
        data sets. It is use as a base class for all versions of
        the DataSegment class template. While the the latter provides
        typesave access to all data entries, the DataSegmentBase class
        provides runtime information about the actual type. The DataSegmentBase::as
        template method can be used for a type-checked cast into an actual DataSegment
        type, which provides type-safe access to the reference data.

        \section _DS_ Data Segments

        Data segments allow for an easy access to different types of
        interleaved data. Internally a single stride value is used to
        allow for wrapping strided data as well. Examples are given in
        the DataSegment<T> template class. The following ascii art
        graphics illustrates the Idea of the data segment:

        <pre>
        consider a data set as a table
           |  x    y    z  timestamp    r    g   b
        -------------------------------------------
         0 | 2.0  4.3  5.5  3478932    255   0   0
         1 | 2.1  3.3  6.4  3478933     0   255  0
        ...
        </pre>

        The a DataSegment could e.g. reference the columns x,y and z
        Where each entry is a 3D vector with float elements. The
        internal stride is defined by the size of a whole row. <b>But
        please note</b> that the DataSegment does not provide an
        internal stride, i.e. the elements of a single vector (here x,y,z)
        must always be packed in memory.

        \section _MORE_ Further Ideas
        DataSegments are only shallow wrappers around data that is mananged
        externally. Therefore, copying (by copy constructor or assignment) DataSegment
        instances always leeds to shallow copier.

        \section _ORG_ 2D Organization
        Data segments can be either 1D or 2D organized. In case of 2D organization,
        an positive "organizedWidth" value has to be given to the constructor.

        \section _BY_ Binary Data Access
        Since the DataSegmentBase does not provice type-save data access
        directly, it provides binary data access operators that return
        instances of the internal untility class
        DataSegmentBase::Bytes. These can be copied deeply (by simple
        assignment) and the DataSegmentBase::Bytes class does also provide
        iterator based access to every byte element
    */
    struct ICLGeom_API DataSegmentBase{
      /// for easier integration with the pointcloud object base class
      friend class geom::PointCloudObjectBase;

    protected:
      /// associcates a core::depth values and the (byte)-size of the corresponding type
      static inline int getSizeOf(core::depth d){
        static const int lens[] = { 1, 2, 4, 4, 8 };
        return lens[(int)d];
      }

      /// data pointer orign
      icl8u *data;

      /// stride between elements
      /** The stride defines the number of bytes, one has to jump, to get from one
          vector start to the next start. The unit is byte E.g.
          \code
          float data[2*4] = {x,y,z,_, x,y,z, _}; // -> stride = 4*sizeof(float)
          \endcode
      */
      size_t stride;

      /// number of vector elements contained
      size_t numElements;

      /// if > 0 , the data is 2D-organized
      icl32s organizedWidth; // -1 if unorganized

      /// underlying data depth
      core::depth dataDepth;

      /// vector element dim
      size_t elemDim;     // number of elements (in Type units)

    public:
      /// returns, whether the segment is 2D-organized
      inline bool isOrganized() const {
        return organizedWidth > 0;
      }

      /// returns the ordred size of the segment of utils::Size::null if it's not organized
      inline utils::Size getSize() const {
        if(!isOrganized()) return utils::Size::null;
        else return utils::Size(organizedWidth, numElements/organizedWidth);
      }

      /// returns the internal stride used
      inline int getStride() const {
        return stride;
      }

      /// returns the number of elements
      /** If the data is 2D-organized, than width*height is returned */
      inline int getDim() const{
        return numElements;
      }

      /// returns the actual core::depth (data type) of the entries
      inline core::depth getDepth() const{
        return dataDepth;
      }

      /// returns the dimension of the contained elements
      inline int getElemDim() const{
        return elemDim;
      }

      /// returns the internal (strided) data pointer
      /** only for those who know what they are doing */
      icl8u *getDataPointer() { return data; }

      /// returns the internal (strided) data pointer
      /** only for those who know what they are doing */
      const icl8u *getDataPointer() const { return data; }

      /// Constructor with given parameters
      inline DataSegmentBase(void *data=0, size_t stride=0,
                             size_t numElements=0, icl32s organizedWidth=-1,
                             core::depth dataDepth=core::depth8u, size_t elemDim=0):
        data((icl8u*)data),stride(stride),numElements(numElements),
        organizedWidth(organizedWidth),dataDepth(dataDepth),elemDim(elemDim){}

      /// shallow and save cast from data segment base to special data segment version
      /** This function performs a save shallow cast, by evaluating runtime core::depth and dim parameters
          to throw an instance of utils::ICLException if the targetet type is not compatible

          <b>Please note:</b> this function can only be defined after
          the DataSegment template class definition is available, so you
          can find it after that class,
      */
      template<class T, int N>
      const DataSegment<T,N> &as() const;

      /// Very simple Byte Vector class to provide binary access to DataSegmentBase data
      class ICLGeom_API Bytes{
        icl8u *data; //!< data pointer (shallowly wrapped)
        int len;     //!< number of byte elements
        /// constructor (private)
        Bytes(icl8u *data=0, int len=0):data(data),len(len){}
        /// copy constructor (also provivate)
        Bytes(const Bytes &other){}
        friend struct DataSegmentBase; //!< only the DataSegmentBase can create these
        public:

        /// returns the number of contained byte elements
        inline int getDim() const{ return len; }

        /// index operator
        icl8u &operator[](int idx) { return data[idx]; }

        /// index operator (const)
        const icl8u operator[](int idx) const { return data[idx]; }

        /// deep copy assignment operator
        /** if source and destination lengths differ, an exception is thrown */
        inline Bytes &operator=(const Bytes &other) throw (utils::ICLException){
          if(len != other.len){
            throw utils::ICLException("unable to assign DataSegmentBase::Bytes: lengths differ!");
          }
          std::copy(other.data,other.data+len,data);
          return *this;
        }

        /// iterator based access to the data begin
        icl8u *begin() { return data; }

        /// iterator based access to the data end
        icl8u *end() { return data+len; }

        /// iterator based access to the data begin (const)
        const icl8u *begin() const { return data; }

        /// iterator based access to the data end (const)
        const icl8u *end() const { return data+len; }

      };

      /// linear index operator
      inline Bytes operator[](int idx) {
        return Bytes(data+idx*stride*getSizeOf(dataDepth), elemDim*getSizeOf(dataDepth));
      }

      /// linear index operator (const)
      inline const Bytes operator[](int idx) const{
        return const_cast<DataSegmentBase*>(this)->operator[](idx);
      }

      /// 2D-index operator (only for organized data segments)
      inline Bytes operator()(int x, int y) {
       return operator[](x + organizedWidth * y );
      }

      /// 2D-index operator (only for organized data segments, const)
      inline const Bytes operator()(int x, int y) const{
        return operator[](x + organizedWidth * y );
      }
    };

  } // namespace core
}


