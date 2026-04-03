// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLUtils/ProgArg.h>
#include <ICLIO/GenericGrabber.h>

using namespace icl::utils;
using namespace icl::io;

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
     ("-p","grabber type (-d dc 0, unicap 0, pwc)")
     ("-p","use input xml-file to setup a list of parameters together. "
      "Incompatible to all others except -d.")
     ("-o","writes all parameters to given output xml file. Incompatible to all others except -d.")
     ("-g","makes the grabber to grab an image before and after loading all properties\n"
      "for some devices (in particular some dc cameras), this is necessary to actually\n"
      "store the new parameters on the device");

  pa_init(n,ppc,"-set|-s(param,value) -get|-g(param) [m]-input|-i(devicetype,devicespec) "
         "-list|-l -parameter-file|-p(input-xml-file) -output|-o(output-xml-file) -grab-once|-go");

  bool s = pa("-s"), g=pa("-g"), l=pa("-l"), p=pa("-p"), o=pa("-o");
  if(!(s||g||l||p||o)){
    pa_show_usage("one arg of -s, -g, -l, -p or -o must be given");
    exit(-1);
  }else if(s+g+l+p+o > 1){
    pa_show_usage("invalid argument combination");
    exit(-1);
  }

  GenericGrabber grabber;
  grabber.init(pa("-i"));

  if(s){
    std::string val = pa("-s", 1).as<std::string>();
    if(pa("-go")) grabber.grabImage();
    grabber.setPropertyValue(pa("-s",0),val);
    if(pa("-go")) grabber.grabImage();
  }else if(g){
    std::cout << grabber.getPropertyValue(pa("-g")) << std::endl;
  }else if(p){
    if(pa("-go")) grabber.grabImage();
    grabber.loadProperties(pa("-p"));
    if(pa("-go")) grabber.grabImage();
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
