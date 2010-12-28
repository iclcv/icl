/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/Any.h                                 **
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

#ifndef ICL_ANY_H
#define ICL_ANY_H

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
  };
      
    
}

#endif
