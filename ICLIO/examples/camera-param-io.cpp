/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/examples/camera-param-io.cpp                     **
** Module : ICLIO                                                  **
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

#include <ICLQuick/Common.h>
#include <ICLIO/GenericGrabber.h>

namespace{
  void write_spaces(int n, char c=' '){
    for(int i=0;i<n;++i){
      std::cout << c;
    }
  }
}
int main(int n, char **ppc){
  paex
  ("-s","set feature to value (e.g. -s gain 100). Incompatible to all others except -d.")
  ("-g","get a value. Incompatible to all others except -d.")
  ("-l","list features of device. Incompatible to all others except -d.")
  ("-d","grabber type (-d dc 0, unicap 0, pwc)")
  ("-i","use input xml-file to setup a list of parameters together. Incompatible to all others except -d.")
  ("-o","writes all parameters to given output xml file. Incompatible to all others except -d.");
  
  painit(n,ppc,"-set|-s(param,value) -get|-g(param) -device|-d(device-type=dc,device-params=0) "
         "-list|-l -input|-i(input-xml-file) -output|-o(output-xml-file)");

  bool s = pa("-s"), g=pa("-g"), l=pa("-l"), i=pa("-i"), o=pa("-o");
  if(!(s||g||l||i||o)){
    pausage("one arg of -s, -g, -l, -i or -o must be given");
    exit(-1);
  }else if(s+g+l+i+o > 1){
    pausage("invalid argument combination");
    exit(-1);
  }
  
  GenericGrabber grabber(FROM_PROGARG("-d"));

  if(s){
    grabber.setProperty(pa("-s",0),pa("-s",1));
  }else if(g){
    std::cout << grabber.getValue(pa("-g")) << std::endl;
  }else if(i){
    grabber.loadProperties(pa("-i"),false,true);
  }else if(o){
    grabber.saveProperties(pa("-o"),false,true);
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
      std::string v = grabber.getValue(s);
      std::cout << s;
      write_spaces(w-s.length());
      std::cout << v;
      write_spaces(w-v.length());
      std::cout << grabber.getInfo(s) << std::endl;
    }
  }  
}
