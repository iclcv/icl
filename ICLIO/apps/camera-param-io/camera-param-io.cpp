/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/apps/camera-param-io/camera-param-io.cpp         **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLQt/Common.h>
#include <ICLIO/GenericGrabber.h>

namespace{
  void write_spaces(int n, char c=' '){
    for(int i=0;i<n;++i){
      std::cout << c;
    }
  }
}
int main(int n, char **ppc){
  pa_explain
     ("-s","set feature to value (e.g. -s gain 100). Incompatible to all others except -d.")
     ("-g","get a value. Incompatible to all others except -d.")
     ("-l","list features of device. Incompatible to all others except -d.")
     ("-d","grabber type (-d dc 0, unicap 0, pwc)")
     ("-i","use input xml-file to setup a list of parameters together. "
      "Incompatible to all others except -d.")
     ("-o","writes all parameters to given output xml file. Incompatible to all others except -d.");
  
  pa_init(n,ppc,"-set|-s(param,value) -get|-g(param) -device|-d(device-type=dc,device-params=0) "
         "-list|-l -input|-i(input-xml-file) -output|-o(output-xml-file)");

  bool s = pa("-s"), g=pa("-g"), l=pa("-l"), i=pa("-i"), o=pa("-o");
  if(!(s||g||l||i||o)){
    pa_show_usage("one arg of -s, -g, -l, -i or -o must be given");
    exit(-1);
  }else if(s+g+l+i+o > 1){
    pa_show_usage("invalid argument combination");
    exit(-1);
  }
  
  GenericGrabber grabber;
  grabber.init(pa("-d"));

  if(s){
    std::string val = pa("-s",1);
    grabber.setPropertyValue(pa("-s",0),val);
  }else if(g){
    std::cout << grabber.getPropertyValue(pa("-g")) << std::endl;
  }else if(i){
    grabber.loadProperties(pa("-i"));
  }else if(o){
    grabber.saveProperties(pa("-o"));
  }else{
    static const int w = 35;
    std::vector<std::string> l = grabber.getPropertyList();
    std::cout << "camera interface provides " << l.size() << " features" << std::endl;
    std::cout << "feature";
    write_spaces(w-strlen("feature"));
    std::cout << "value";
    write_spaces(w-strlen("value"));
    std::cout << "info" << std::endl;
    write_spaces(3*w,'-');
    std::cout << std::endl;
    
    for(unsigned int i=0;i<l.size();++i){
      const std::string &s = l[i];
      std::string v = grabber.getPropertyValue(s);
      std::cout << s;
      write_spaces(w-s.length());
      std::cout << v;
      write_spaces(w-v.length());
      std::cout << grabber.getPropertyInfo(s) << std::endl;
    }
  }  
}
