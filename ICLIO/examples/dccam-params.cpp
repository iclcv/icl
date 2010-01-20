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
  pa_explain("-s","set feature to value (e.g. -s gain 100)");
  pa_explain("-g","get a value");
  pa_explain("-l","list features of device");
  pa_explain("-d","grabber type (-d dc 0, unicap 0, pwc)");
  
  pa_init(n,ppc,"-s(2) -g(1) -d(2) -l");

  if(!pa_defined("-s") && ! pa_defined("-g") && !pa_defined("-l")){
    ERROR_LOG("please define either -s for set a parameter or\n"
              "or -g for get a parameter");
    exit(-1);
  }else if(pa_defined("-s") && pa_defined("-g")){
    ERROR_LOG("please define either -s for set a parameter or\n"
              "or -g for get a parameter (not both)");
    exit(-1);
  }
  
  GenericGrabber g(pa_subarg<std::string>("-d",0,"dc"),
                   pa_subarg<std::string>("-d",0,"dc")+"="+pa_subarg<std::string>("-d",1,"0"));
  
  


  if(pa_defined("-s")){
    std::string f = pa_subarg<std::string>("-s",0,"");
    std::string v = pa_subarg<std::string>("-s",1,"");
    g.setProperty(f,v);
  }else if(pa_defined("-g")){
    std::string f = pa_subarg<std::string>("-g",0,"");
    std::cout << g.getValue(f) << std::endl;
  }else{
    static const int w = 35;
    std::vector<std::string> l = g.getPropertyList();
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
      std::string v = g.getValue(s);
      std::cout << s;
      write_spaces(w-s.length());
      std::cout << v;
      write_spaces(w-v.length());
      std::cout << g.getInfo(s) << std::endl;
    }
  }  
}
