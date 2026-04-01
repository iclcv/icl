// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>

namespace icl{
  namespace utils{
    /// Fixed C++-array wrapper class for data handling
    /** This class is specialized for the DIM values 1,2,3 and 4
        FixedArray<T,1> contains a union that shares the
        1-dim data with a single value x. FixedArray<T,2>
        has extra values x,y,  then x,y,z and finally x,y,z,w
        (w can also be called h)
    */
    template<class T,unsigned int DIM>
    struct FixedArray{
      T m_data[DIM];

      /// index access operator
      T &operator[](unsigned int idx) { return m_data[idx]; }

      /// index access operator (const)
      const T &operator[](unsigned int idx) const { return m_data[idx]; }
    };

    /// Specialization for 1D-vectors providing a value x
    template<class T>
    struct FixedArray<T, 1u>{
      union{
        T m_data[1];
        T x;
      };

      /// empty constructor (leaving data uninitialized)
      FixedArray(){}

      /// constructor with given value x
      FixedArray(const T &x) : x(x){}

      /// index access operator
      T &operator[](unsigned int idx) { return m_data[idx]; }

      /// index access operator (const)
      const T &operator[](unsigned int idx) const { return m_data[idx]; }

      /// reinterprect FixedArray<T,1u> as a scalar
      inline operator T() const{
        return x;
      }
    };

    /// Specialization for 2D-matrics providing direct access to values x, y
    template<class T>
    struct FixedArray<T, 2u>{
      union{
        T m_data[2];
        struct { T x; T y; };
      };

      /// empty constructor (leaving data uninitialized)
      FixedArray(){}

      /// constructor with given values, x and y
      FixedArray(const T &x, const T &y) : x(x),y (y){}

      /// index access operator
      T &operator[](unsigned int idx) { return m_data[idx]; }

      /// index access operator (const)
      const T &operator[](unsigned int idx) const { return m_data[idx]; }
    };


    /// Specialization for 3D-matrics providing direct access to values x, y, z
    template<class T>
    struct FixedArray<T, 3u>{

      /// empty constructor (leaving data uninitialized)
      FixedArray(){}

      /// constructor with given values, x, y and z
      FixedArray(const T &x, const T &y, const T &z) : x(x), y(y), z(z){}

      union{
        T m_data[3];
        struct { T x; T y; T z; };
      };
      /// index access operator
      T &operator[](unsigned int idx) { return m_data[idx]; }

      /// index access operator (const)
      const T &operator[](unsigned int idx) const { return m_data[idx]; }
    };

    /// Specialization for 4D-matrics providing direct access to values x, y, z and h
    template<class T>
    struct FixedArray<T, 4u>{

      /// empty constructor (leaving data uninitialized)
      FixedArray(){}

      /// constructor with given values, x, y, z and w
      FixedArray(const T &x, const T &y, const T &z, const T &w) : x(x), y(y), z(z), w(w){}

      union{
        T m_data[4];
        struct {
          T x;
          T y;
          T z;
          union {
            T h;
            T w;
          };
        };
      };

      /// index access operator
      T &operator[](unsigned int idx) { return m_data[idx]; }

      /// index access operator (const)
      const T &operator[](unsigned int idx) const { return m_data[idx]; }
    };

  } // namespace utils
} // namespace icl
