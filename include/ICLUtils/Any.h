/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/Any.h                                 **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                       q                                          **
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

#include <ICLUtils/StringUtils.h>

namespace icl{
  
  /// Simple generic data type implementation that uses a string based data representation
  /** Instances of class Any can simply be created from any
      data type that implements the ostream-operator<< and
      it can be converted implicitly to any data type that
      implements the istream-operator>>. 
      For cases, where C++ cannot infer the correct type an 
      explicit conversion template-method as<T>() is also 
      provided. Any does not allow for type checks.
      
      \section STR std::string representation
      Internally, an instance of type Any contains not more than
      the string representation of the serialized type. Since
      Any inherits the std::string class, the string-serialized
      version is also available.
      
      \section PTR Representing Pointers
      The Any class can also be used to represent pointers. 
      though the default machanism (i.e. operators >> and <<) 
      could be overloaded to work with ascii-encoded pointers 
      as well. The Any class provides a more efficient implementation
      for this.
      The static template functions Any::ptr can be used
      to create a binary encoded pointer representation.\n
      If you want to use any instance of Any to pass a pointer, it is
      strongly recommended to use the binary representation instead of
      using ascii encoded pointers. The only drawback for the binary
      encoded pointers is that they cannot be used as std::string
      anymore.
      \subsection BENCH Benchmarks:
      
      Times were taken on an Intel 1.6GHz Core2Duo (ULV) laptop on 32Bit ubuntu. 
      The times are given per million calls, and we did not use
      gcc optimizations because most operations were optimized out
      in this case. However the speed advatange seemed to less then
      about 30%.
      
      * ASCII Encoded Pointers\n
        <tt>int *p = new int[5]; Any a(int)p; int *p2 = (int*)a.as<int>(); </tt>\n
        <b>Time: ~ 1/250 ms</b>\n
      
      * Binary Encoded Pointers\n
        <tt>int *p = new int[5]; Any a = Any::ptr(p); int *p2 = Any::ptr<int>(a); </tt>\n
        <b>Time: ~ 1/3000 ms</b>\n
        Here, we also benchmarked the steps separatly and we found out, that the creation of
        the any instance took more than 99.9% of the time. Therefore, we can conclude that
        parsing a binary endocded Any instance is close the using a raw-pointer instead:

      * C++ Pointers: \n
        <b>Time: ~ 1/10000 ms</b>\n
  */
  struct Any : public std::string{
    /// Empty constructor
    inline Any(){}
    
    /// real constructor with any given source type
    /** This constructor can only be used for types that implement
        the ostream-operator<<. If this is not true, you can either
        overload the ostream-operator>> for instances of type T, or
        you can pass an already serialized (as std::string) version 
        of T. */
    template<class T>
    inline Any(const T &t):std::string(str(t)){}
    
    /// implicit cast operator that allows for casts into any type
    /** This operator does only work if the istream-operator>> is
        overloded for instances of type T. If this operator does not exist, 
        you can either implement it, or parse this instance of type
        Any, which is also an instance of type std::string manually. */
    template<class T>
    inline operator T() const{ 
      return as<T>();
    }
    
    /// explict cast method (if implicit cast is ambiguous)
    /** In many situations, C++ can not automatically infer the correct
        destination type if the implicit cast operator is implemented
        as a template method. E.g.:
        \code
        Any myInt(7);
        int i = myInt; // Works -> implicit cast to int
        int j;
        j = myInt; // Error: cast is ambiguous
        // instead:
        j = myInt.as<int>(); // Works -> explicit cast to int
        \endcode
    */
    template<class T>
    inline T as() const{
      return parse<T>(*this);
    }
    
    /// remembers the ostream-operator<< that Any is of type std::string
    friend inline std::ostream& operator<<(std::ostream &s, const Any &any){
      return s << (const std::string&)any;
    }

    /// remembers the istream-operator<< that Any is of type std::string
    friend inline std::istream& operator>>(std::istream &s, Any &any){
      return s >> (std::string&)any;
    }

    private:
    /// for direct creation of the parent string (used in the static ptr methods)
    Any(size_t n, char c):std::string(n,c){}

    public:

    /// this method can be used to extract a pointer that was encoded as Any
    /** @see \ref PTR */
    template<class T>
    static T* ptr(const Any &any){
      return *(T**)(&any[0]);
    }

    /// this method can be used to create an any that contains a binary encoded pointer
    /** @see \ref PTR */
    template<class T>
    static Any ptr(const T *p){
      Any any(std::string(sizeof(void*),'\0'));
      *((const void**)&any[0]) = p;
      return any;
    }
    
  };
      
    
}

