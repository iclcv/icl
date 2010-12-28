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
    std::map<std::string,Property>::iterator it = m_properties.find(propertyName);
    if(it == m_properties.end()) throw ICLException("Property " + str(propertyName) + " is not supported");
    return it->second;
  }

  void Configurable::addChildConfigurable(Configurable *configurable, const std::string &childPrefix){
    ICLASSERT_RETURN(configurable);
    m_childConfigurables.push_back(configurable);
    std::string pfx = childPrefix;
    if(pfx.length() && pfx[pfx.length()-1] != '.'){
      pfx += '.';
    }
    const std::vector<std::string> ps = configurable->getPropertyList();
    for(unsigned int i=0;i<ps.size();++i){
      Property p = Property(configurable, pfx+ps[i],configurable->getPropertyType(ps[i]),
                            configurable->getPropertyInfo(ps[i]),configurable->getPropertyValue(ps[i]),
                            configurable->getPropertyVolatileness(ps[i]));
      p.childPrefix = pfx;
      std::map<std::string,Property>::iterator it = m_properties.find(p.name);
      if(it != m_properties.end()) throw ICLException("Property " + str(p.name) + "cannot be added from child configurable due to name conflicts");
      m_properties[p.name] = p;
    }
  }

  const Configurable::Property &Configurable::prop(const std::string &propertyName) const throw (ICLException){
    return const_cast<Configurable*>(this)->prop(propertyName);
  }

  std::vector<std::string> Configurable::getPropertyList(){
    std::vector<std::string> v(m_properties.size());
    int i=0;
    for(PropertyMap::iterator it=m_properties.begin();it!=m_properties.end();++it){
      v[i++] = it->second.name;
    }
    return v;
  }


  void Configurable::addProperty(const std::string &name, const std::string &type, 
                                 const std::string &info, const Any &value, 
                                 int volatileness) throw (ICLException){
    try{
      prop(name);
      throw ICLException("Unable to add property " + name + " because it is already used");
    }catch(ICLException &ex){
      m_properties[name]= Property(this,name,type,info,value,volatileness);
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
  
  Configurable::Configurable(const Configurable &other){
    m_properties = other.m_properties;
    for(PropertyMap::iterator it=m_properties.begin();it!=m_properties.end();++it){
      if(it->second.configurable == &other){
        it->second.configurable = this;
      }
    }
    m_childConfigurables = other.m_childConfigurables;
    m_ID = "";
  }
  
  Configurable &Configurable::operator=(const Configurable &other) {
    setConfigurableID("");
    m_properties = other.m_properties;
    for(PropertyMap::iterator it=m_properties.begin();it!=m_properties.end();++it){
      if(it->second.configurable == &other){
        it->second.configurable = this;
      }
    }
    m_childConfigurables = other.m_childConfigurables;
    return *this;
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
    if(ID != ""){
      m_instances[ID] = this;
    }
  }

  void Configurable::call_callbacks(const std::string &propertyName){
    if(!callbacks.size()) return;
    std::string type = getPropertyType(propertyName);
    std::string value = getPropertyValue(propertyName);
    for(std::vector<PropertyChangedCallbackPtr>::iterator it=callbacks.begin();it!=callbacks.end();++it){
      (*it)->propertyChanged(propertyName,type,value);
    }
  }

  void Configurable::removedCallback(const PropertyChangedCallback *cb){
    for(std::vector<PropertyChangedCallbackPtr>::iterator it=callbacks.begin();it!=callbacks.end();++it){
      if( it->get() == cb ){
        callbacks.erase(it);
        break;
      }
    }
  }
  Any Configurable::getPropertyValue(const std::string &propertyName){
    Property &p = prop(propertyName);
    if(p.configurable != this){
      return p.configurable->getPropertyValue(propertyName.substr(p.childPrefix.length()));
    }else{
      return p.value;
    }
  }
  
  bool Configurable::supportsProperty(const std::string &propertyName){
    std::vector<std::string> l = getPropertyList();
    return find(l.begin(),l.end(),propertyName) != l.end();
  }

  void Configurable::setPropertyValue(const std::string &propertyName, const Any &value) throw (ICLException){
    Property &p = prop(propertyName);
    if(p.configurable != this){
      p.configurable->setPropertyValue(propertyName.substr(p.childPrefix.length()),value);
    }else{
      p.value = value;
    }
    call_callbacks(propertyName);
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
    std::vector<std::string> psSupported = getPropertyListWithoutDeactivated();
    
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
      
      if(type == "range" || type == "value-list" || type == "range:slider" || type == "range:spinbox"){
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
    std::vector<std::string> psSupported = getPropertyListWithoutDeactivated();
    
    if(filterOUT.size()){
      psSupported = remove_by_filter(psSupported,filterOUT);
    }
    f.setPrefix("config.");
    for(unsigned int i=0;i<psSupported.size();++i){
      std::string &prop = psSupported[i];
      std::string type = getPropertyType(prop);
      if(type == "info") continue;
      if(type == "command") continue;

      if(type == "range" || type == "value-list" || type == "range:slider" || type == "range:spinbox"){
        try{
          setPropertyValue(prop,str((icl32f)f[prop]));
        }catch(...){}
      }else if(type == "menu"){
        try{
          setPropertyValue(prop,f[prop].as<Any>());
        }catch(...){}
      }
    }
  }

  void Configurable::deactivateProperty(const std::string &pattern){
    m_deactivated.push_back(pattern);
  }

  void Configurable::deleteDeactivationPattern(const std::string &pattern){
    std::vector<std::string>::iterator it = std::find(m_deactivated.begin(),m_deactivated.end(),pattern);
    if(it == m_deactivated.end()){
      ERROR_LOG("unable to deactivate pattern '" << pattern << "' because this pattern cannot be found"
                "in the current list of deactivated patterns");
    }
  }
    
  std::vector<std::string> Configurable::getPropertyListWithoutDeactivated(){
    if(!m_deactivated.size()) return getPropertyList();
    std::vector<std::string> passed;
    std::vector<std::string> ps = getPropertyList();
    for(unsigned int i=0;i<ps.size();++i){
      bool found = false;
      for(unsigned int j=0;j<m_deactivated.size();++j){
        if(icl::match(ps[i],m_deactivated[j])){
          found=true;
          break;
        }
      }
      if(!found){
        passed.push_back(ps[i]);
      }
    }
    return passed;
  }

  void Configurable::adaptProperty(const std::string &name,const std::string &newType,const std::string &newInfo) throw (ICLException){
    Property &p = prop(name);
    if(p.configurable != this){
      p.configurable->adaptProperty(name.substr(p.childPrefix.length()),newType,newInfo);
    }else{
      p.type = newType;
      p.info = newInfo;
    }
  }

}
