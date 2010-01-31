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
