/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/FourCC.h                                 **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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

#ifndef ICL_FOUR_CC_H
#define ICL_FOUR_CC_H

#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/Exception.h>
#include <string>
#include <iostream>

namespace icl{
  namespace io{
    
    /// Wrapper class for fourcc color codes
    /** @see ColorFormatDecoder */
    class FourCC{
      icl32s key; //!< internally a fourcc is represented by an u-int value
        
      public:
      /// create a null FourCC (key = 0)
      FourCC():key(0){}
      
      /// create a FourCC with given key
      FourCC(icl32s key):key(key){} 
      
      /// create a fourCC from given string color code (e.g. Y422 or MJPG)
      FourCC(const std::string &key){
        init(key);
      }
      
      /// create a fourCC from given string color code (cstring style)
      FourCC(const char *key){
        init(key);
      }
  
      /// initialization utility method
      void init(const std::string &key) throw (ICLException){
        if(key.length() != 4) throw ICLException("FourCC::FourCC(string): invalid fourcc code " + key);
        if(key == "null") this->key = 0;
        else this->key = (((icl32u)(key[0])<<0)|((icl32u)(key[1])<<8)|((icl32u)(key[2])<<16)|((icl32u)(key[3])<<24));
      }
      
      /// int-assignment
      inline FourCC &operator=(icl32s key) { return *this=FourCC(key); }
  
      /// std::string-assignment
      inline FourCC &operator=(const std::string &key) { return *this=FourCC(key); }
      
      /// convert to string
      std::string asString() const {
        if(!key) return "null";
        int tmp[2] = {key,0};
        return std::string((char*)tmp);
      }
      
      /// obtain current key (uint)
      icl32s asInt() const {
        return key;
      }
      
      /// check whether it is null
      operator bool() { return key != 0; }
  
      /// implicit conversion to int
      operator int() { return asInt(); }
    };
  
    /// overloaded ostream operator for type fourCC
    inline std::ostream &operator<<(std::ostream &stream, const FourCC &fourCC){
      return stream << "FourCC(" << fourCC.asString() << ")" << std::endl;
    }
  
  } // namespace io
}

#endif
