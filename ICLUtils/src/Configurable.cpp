#include <ICLUtils/Configurable.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/ConfigFile.h>

namespace icl{

  namespace{
    struct DefInt{
      int value;
      DefInt():value(0){}
    };
  }
  
  std::string Configurable::create_default_ID(const std::string &prefix){
    static std::map<std::string,DefInt> numbers;
    DefInt &i = numbers[prefix];
    return prefix+str(i.value++);
  }
  
  Configurable::Property &Configurable::prop(const std::string &propertyName) throw (ICLException){
    std::vector<Property>::iterator it = std::find(m_properties.begin(),m_properties.end(),propertyName);
    if(it == m_properties.end()) throw ICLException("Property " + str(propertyName) + " is not supported");
    return *it;
  }

  const Configurable::Property &Configurable::prop(const std::string &propertyName) const throw (ICLException){
    return const_cast<Configurable*>(this)->prop(propertyName);
  }

  void Configurable::addProperty(const std::string &name, const std::string &type, 
                                 const std::string &info, const std::string &value, 
                                 int volatileness) throw (ICLException){
    try{
      prop(name);
      throw ICLException("Unable to add property " + name + " because it is already used");
    }catch(ICLException &ex){
      m_properties.push_back(Property(name,type,info,value,volatileness));
    }
  }
  

  const std::vector<std::string> Configurable::EMPTY_VEC;
  std::map<std::string,Configurable*> Configurable::m_instances;
  
  Configurable* Configurable::get(const std::string &id){
    std::map<std::string,Configurable*>::iterator it = m_instances.find(id);
    if(it != m_instances.end()){
      return it->second;
    }else{
      return 0;
    }
  }

  Configurable::Configurable(const std::string &ID) throw (ICLException) : m_ID(ID){
    if(ID.length()){
      if(get(ID)) throw ICLException(str("Configurable(")+ID+"): given ID is already used");
    }
    m_instances[ID] = this;
  }
    
  
  void Configurable::setConfigurableID(const std::string &ID) throw (ICLException){
    if(m_ID.length()){
      std::map<std::string,Configurable*>::iterator it = m_instances.find(m_ID);
      if(it == m_instances.end()){
        WARNING_LOG("this should not happen");
      }
      m_instances.erase(it);
    }
    m_ID = ID;
    m_instances[ID] = this;
  }
  
  std::vector<std::string> remove_by_filter(const std::vector<std::string> &ps, 
                                            const std::vector<std::string> &filter){
    std::vector<std::string> ps2;
    for(unsigned int i=0;i<ps.size();++i){
      const std::string &s = ps[i];
      if(std::find(filter.begin(),filter.end(),s) == filter.end()){
        ps2.push_back(s);
      }
    }
    return ps;
  }



  void Configurable::saveProperties(const std::string &filename, const std::vector<std::string> &filterOUT){
    ConfigFile f;
    f["config.title"] = std::string("Configuration file for Configurable ID: ") + m_ID;
    std::vector<std::string> psSupported = getPropertyList();
    
    if(filterOUT.size()){
      psSupported = remove_by_filter(psSupported,filterOUT);
    }

    /* // sometimes, property-name contains '.'-delimiters, then, we have deeper section also
        <config>
          <data id="property-name" type="float|string">PROPERTY_VALUE</data>
        </section>
          ...
        </config>
    */
    f.setPrefix("config.");
    for(unsigned int i=0;i<psSupported.size();++i){
      std::string &prop = psSupported[i];
      std::string type = getPropertyType(prop);
      if(type == "info") continue;
      if(type == "command") continue;

      std::string val = getPropertyValue(prop);
      
      if(type == "range" || type == "value-list"){
        f[prop] = parse<icl32f>(val);
      }else if(type == "menu"){
        f[prop] = val;
      }
    }
    f.save(filename);
  }

  
  
  void Configurable::loadProperties(const std::string &filename, const std::vector<std::string> &filterOUT){
    ConfigFile f(filename);
    f["config.title"] = std::string("Configuration file for Configurable ID:") + m_ID +  ")";
    std::vector<std::string> psSupported = getPropertyList();
    
    if(filterOUT.size()){
      psSupported = remove_by_filter(psSupported,filterOUT);
    }
    f.setPrefix("config.");
    for(unsigned int i=0;i<psSupported.size();++i){
      std::string &prop = psSupported[i];
      std::string type = getPropertyType(prop);
      if(type == "info") continue;
      if(type == "command") continue;

      if(type == "range" || type == "value-list"){
        try{
          setPropertyValue(prop,str((icl32f)f[prop]));
        }catch(...){}
      }else if(type == "menu"){
        try{
          setPropertyValue(prop,f[prop]);
        }catch(...){}
      }
    }
  }
}
