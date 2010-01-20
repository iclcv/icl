#include <ICLIO/DCGrabber.h>
#include <ICLIO/DCDeviceFeatures.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/StringUtils.h>

using namespace icl;
using namespace std;


namespace{
  std::vector<string> remove_size(const vector<string> &v){
    vector<string> r;
    for(unsigned int i=0;i<v.size();++i){
      if(v[i] != "size") r.push_back(v[i]);
    }
    return r;
  }  
}

int main(int n, char **ppc){
  pa_explain("-d","select device index (default is 0)");
  pa_explain("-ld","list all devices");
  pa_explain("-lf","list all features of given device");
  pa_explain("-s","show device identifier");
  pa_explain("-o","writes current camera config to disk\n"
                  "using given output xml file");
  pa_explain("-i","input xml config file");
  
  pa_init(n,ppc,"-d(1) -ld -s -lf -o(1) -i(1)");
  
  std::vector<DCDevice> devs = DCGrabber::getDeviceList();
  if(!devs.size()){
    ERROR_LOG("no device found!");
    return -1;
  }
  
  if(pa_defined("-ld")){
    for(unsigned int i=0;i<devs.size();++i){
      devs[i].show("Device " + str(i));
    }
    return 0;
  }
  
  unsigned int devIdx = pa_subarg<unsigned int>("-d",0,0);
  if(devIdx >= devs.size()){
    ERROR_LOG("device index "<< devIdx << "is invalid (allowed is 0-" << devs.size()-1 << ")" );
    return -1;
  }
  
  DCDevice dev = devs[devIdx];
  
  if(pa_defined("-s")){
    printf("Device: %s \n",(dev.getModelID()+"(Vendor:"+dev.getVendorID()+")").c_str());
  }
  
  DCDeviceFeatures f(dev);
  if(pa_defined("-lf")){
    f.show();
  }
  
  if(pa_defined("-o")){
    ConfigFile config;
    config["config.title"] = std::string("Camera Configuration File");
    config.set("config.camera.modelID",dev.getModelID());    
    config.set("config.camera.vendorID",dev.getVendorID());
    
    DCGrabber g(dev);
    std::vector<string> ps = remove_size(g.getPropertyList());
    static const string prefix = "config.params.";
    for(unsigned int i=0;i<ps.size();++i){
      string &prop = ps[i];
      string type = g.getType(prop); 
      if(type == "range" || type == "value-list"){
        config.set(prefix+prop,to32f(g.getValue(prop)));
      }else if(type == "menu"){
        config.set(prefix+prop,g.getValue(prop));
      }
    }
    config.save(pa_subarg<std::string>("-o",0,""));
  }
  
  if(pa_defined("-i")){
    ConfigFile config(pa_subarg<string>("-i",0,""));
    config.listContents();

    string configFileModel = config.get<string>("config.camera.modelID");
    string configFileVendor = config.get<string>("config.camera.vendorID");
    
    if(dev.getModelID() != configFileModel){
      ERROR_LOG("model-ID does not match! (config:" << configFileModel << " cam:" << dev.getModelID() << ")");
      return -1;
    }
    if(dev.getVendorID() != configFileVendor){
      ERROR_LOG("vendor-ID does not match! (config:" << configFileVendor << " cam:" << dev.getVendorID() << ")");
      return -1;
    }
    
    DCGrabber g(dev);
    std::vector<string> ps = remove_size(g.getPropertyList());
    static const string prefix = "config.params.";
    for(unsigned int i=0;i<ps.size();++i){
      string &prop = ps[i];
      string type = g.getType(prop); 
      string propName = prefix+prop;
      if(type == "range" || type == "value-list"){
        if(config.contains(propName)){
          g.setProperty(prop,str(config.get<float>(propName)));
        }
      }else if(type == "menu"){
        if(config.contains(propName)){
          g.setProperty(prop,config.get<string>(propName));
        }
      }
    }

  
  }
  
  
  return 0;
}
