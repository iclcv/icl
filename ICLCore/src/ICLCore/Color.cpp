/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/Color.cpp                          **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
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

#include <ICLCore/Color.h>
#include <map>
#include <ctype.h>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;
using namespace icl::math;

namespace icl{
  namespace core{

    namespace{
      struct ColorMap : public std::map<std::string,Color>{
        ColorMap(){
          ColorMap &t = *this;
          t["black"] = Color(0,0,0);
          t["white"] = Color(255,255,255);

          t["red"] = Color(255,0,0);
          t["green"] = Color(0,255,0);
          t["blue"] = Color(0,0,255);


          t["cyan"] = Color(0,255,255);
          t["magenta"] = Color(255,0,255);
          t["yellow"] = Color(255,255,0);

          t["gray200"] = Color(200,200,200);
          t["gray150"] = Color(150,150,150);
          t["gray100"] = Color(100,100,100);
          t["gray50"] = Color(50,50,50);
        }
      };
    }

    static const Color *create_named_color(std::string str){
      toLowerI(str);
      static ColorMap cm;
      ColorMap::iterator it = cm.find(str);
      if(it != cm.end()) return  &it->second;
      else return 0;
    }

    Color color_from_string(const std::string &name){
      const Color *col = create_named_color(name);
      if(col) return *col;
      return parse<Color>(name);
    }



  } // namespace core
}
