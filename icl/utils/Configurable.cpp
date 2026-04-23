// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/Configurable.h>
#include <icl/utils/Macros.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/ConfigFile.h>
#include <icl/utils/prop/Constraints.h>
#include <mutex>
#include <variant>

namespace icl::utils {
  namespace{

    // Build a typed constraint + initial typed_value from the legacy
    // (type, info, value) string triple.  Covers every utils-level
    // property kind; unknown types ("color", "image" pre-typed-migration,
    // Pylon/OpenNI hardware-introspected strings, malformed input) fall
    // through to a `prop::Generic` that echoes the original type / info
    // verbatim.
    //
    // Post-step-6 all in-tree call sites use the typed addProperty
    // overload, so this dispatch is only exercised by:
    //   - external callers still on the legacy overload
    //   - the dynamic registration path (PylonCameraOptions etc.)
    //     that receives type/info/value as runtime strings
    //
    // Returns {constraint, typed_value} — both always populated.
    std::pair<std::any, std::any> buildConstraintFromLegacy(
        std::string_view type, std::string_view info,
        std::string_view value_str)
    {
      using namespace prop;

      // Generic fallback used by every non-structured path below.
      auto fallbackGeneric = [&]() -> std::pair<std::any, std::any> {
        return {std::any(Generic{.type_string = std::string(type),
                                 .info_string = std::string(info)}),
                std::any(std::string(value_str))};
      };

      try {
        if (type == "flag") {
          bool v = value_str.empty() ? false : parse<bool>(value_str);
          return {std::any(Flag{}), std::any(v)};
        }
        if (type == "info") {
          return {std::any(Info{}), std::any(std::string(value_str))};
        }
        if (type == "command") {
          return {std::any(Command{}), std::any(std::monostate{})};
        }
        if (type == "string") {
          int maxlen = info.empty() ? 0 : parse<int>(info);
          return {std::any(Text{.maxLength = maxlen}),
                  std::any(std::string(value_str))};
        }
        if (type == "menu" || type == "value-list") {
          // Numeric "value-list" also goes through Menu<string>; qt::Prop
          // renders either as a dropdown.  The stored value is kept as a
          // std::string in typed_value, matching historical behaviour.
          return {std::any(menuFromCsv(info)),
                  std::any(std::string(value_str))};
        }

        // Numeric range family — "range", "range:slider", "range:spinbox",
        // "float", "int".  info is `[A,B]` or `[A,B]:S`; tolerate ':' as
        // min/max separator too (legacy typo in a handful of sites).
        const bool is_spinbox =
            (type == "range:spinbox" || type == "int");
        const bool is_range =
            (type == "range" || type == "range:slider" || type == "float" ||
             is_spinbox);

        if (!is_range) return fallbackGeneric();

        auto lb = info.find('[');
        auto rb = info.find(']');
        if (lb == std::string_view::npos || rb == std::string_view::npos)
          return fallbackGeneric();

        std::string_view core = info.substr(lb + 1, rb - lb - 1);
        std::string_view step_str;
        if (rb + 1 < info.size() && info[rb + 1] == ':') {
          step_str = info.substr(rb + 2);
        }
        auto sep = core.find(',');
        if (sep == std::string_view::npos) sep = core.find(':');
        if (sep == std::string_view::npos) return fallbackGeneric();

        std::string_view min_s = core.substr(0, sep);
        std::string_view max_s = core.substr(sep + 1);

        // Int vs float heuristic: spinbox/int forces int; otherwise
        // inspect literals for decimal/exponent suffixes.
        auto looks_float = [](std::string_view s) {
          for (char c : s) if (c == '.' || c == 'e' || c == 'E' || c == 'f') return true;
          return false;
        };
        bool is_float = !is_spinbox && (looks_float(min_s) || looks_float(max_s) || looks_float(step_str));

        UI ui = is_spinbox ? UI::Spinbox : UI::Slider;

        if (is_float) {
          float mn = parse<float>(min_s);
          float mx = parse<float>(max_s);
          float st = step_str.empty() ? 0.f : parse<float>(step_str);
          float v  = value_str.empty() ? mn : parse<float>(value_str);
          return {std::any(Range<float>{.min = mn, .max = mx, .step = st, .ui = ui}),
                  std::any(v)};
        } else {
          int mn = parse<int>(min_s);
          int mx = parse<int>(max_s);
          int st = step_str.empty() ? 0 : parse<int>(step_str);
          int v  = value_str.empty() ? mn : parse<int>(value_str);
          return {std::any(Range<int>{.min = mn, .max = mx, .step = st, .ui = ui}),
                  std::any(v)};
        }
      } catch (...) {
        // Malformed numeric literals, etc. — preserve the legacy strings
        // through a Generic constraint so external introspection still
        // sees the original type/info.
        return fallbackGeneric();
      }
    }

    struct DefInt{
      int value;
      DefInt():value(0){}
    };
  }

  std::string Configurable::create_default_ID(const std::string &prefix){
    static std::map<std::string,DefInt, std::less<>> numbers;
    DefInt &i = numbers[prefix];
    return prefix+str(i.value++);
  }

   Configurable::Property &Configurable::prop_storage(const std::string &propertyName){
    if(auto it = m_properties.find(propertyName); it == m_properties.end()){
      throw ICLException("Property " + str(propertyName) + " is not supported");
    } else {
      return it->second;
    }
  }

  void Configurable::addChildConfigurable(Configurable *configurable, const std::string &childPrefix){
    ICLASSERT_RETURN(configurable);
    std::string pfx = childPrefix;
    if(pfx.length() && pfx[pfx.length()-1] != '.'){
      pfx += '.';
    }
    m_childConfigurables[configurable] = pfx;
    const std::vector<std::string> ps = configurable->getPropertyListWithoutDeactivated();//getPropertyList();
    for(unsigned int i=0;i<ps.size();++i){
      // The imported Property is a facade — actual storage lives on the
      // child; prop lookups with the prefix'd name are forwarded to it
      // via Property::configurable.  We still cache the child's
      // constraint here so the parent's getPropertyType / getPropertyInfo
      // / qt::Prop dispatch can read the structured constraint locally
      // without chasing the configurable pointer.
      auto ch       = configurable->prop(ps[i]);
      Property p(configurable, pfx+ps[i], ch.tooltip);
      p.childPrefix = pfx;
      p.constraint  = ch.constraint;
      if(auto it = m_properties.find(p.name); it != m_properties.end()) throw ICLException("Property " + str(p.name) + "cannot be added from child configurable due to name conflicts");
      m_properties[p.name] = p;
      if(m_isOrdered) m_ordering[m_properties.size()] = p.name;
    }
    configurable -> m_elderConfigurable = this;
  }

  void Configurable::removeChildConfigurable(Configurable *configurable){
    ICLASSERT_RETURN(configurable);
    auto cit = m_childConfigurables.find(configurable);
    if (cit == m_childConfigurables.end()) return;
    const std::string pfx = cit->second;
    // Drop every property previously imported from this child (matched
    // by `(child*, prefix)` rather than by name to be defensive against
    // name collisions across siblings).
    for (auto it = m_properties.begin(); it != m_properties.end(); ) {
      if (it->second.configurable == configurable && it->second.childPrefix == pfx) {
        // Also rewrite the ordering map if used.
        if (m_isOrdered) {
          for (auto oit = m_ordering.begin(); oit != m_ordering.end(); ) {
            if (oit->second == it->first) oit = m_ordering.erase(oit);
            else ++oit;
          }
        }
        it = m_properties.erase(it);
      } else {
        ++it;
      }
    }
    m_childConfigurables.erase(cit);
    configurable->m_elderConfigurable = nullptr;
  }

  const Configurable::Property &Configurable::prop_storage(const std::string &propertyName) const{
    return const_cast<Configurable*>(this)->prop_storage(propertyName);
  }

  Configurable::Handle Configurable::prop(const std::string &propertyName){
    return Handle{this, prop_storage(propertyName)};
  }

  Configurable::Handle Configurable::prop(const std::string &propertyName) const{
    // Const overload — writes through the resulting handle's PropertyValueRef
    // still flow through setPropertyValueTyped on a non-const *this (via the
    // const_cast below).  Matches the existing semantic where const-ness of a
    // Configurable doesn't gate property writes.
    return Handle{const_cast<Configurable*>(this),
                  const_cast<Property&>(prop_storage(propertyName))};
  }

  std::string Configurable::Handle::type() const {
    // Delegate to the virtual getter so subclasses that override
    // getPropertyType see their override.  Default impl synthesizes via
    // the constraint adapter.
    return m_conf->getPropertyType(name);
  }

  std::string Configurable::Handle::info() const {
    return m_conf->getPropertyInfo(name);
  }

  std::vector<std::string> Configurable::getPropertyList() const{
    std::vector<std::string> v(m_properties.size());
    if(!m_isOrdered){
      int i=0;
      for(PropertyMap::const_iterator it=m_properties.begin();it!=m_properties.end();++it){
        v[i++] = it->second.name;
      }
    } else {
      std::map<int,std::string>::const_iterator it;
      int i = 0;
      for(it=m_ordering.begin();it!=m_ordering.end();++it){
        v[i++] = it->second;
      }
    }
    return v;
  }


  void Configurable::addProperty(const std::string &name, const std::string &type,
                                 const std::string &info, const AutoParse<std::string> &value,
                                 const std::string &tooltip){
    try{
      prop_storage(name);
      throw ICLException("Unable to add property " + name + " because it is already used");
    }catch(ICLException &){
      Property p(this, name, tooltip);
      // Derive constraint + typed_value from the legacy string triple
      // so downstream consumers (qt::Prop, getPropertyType, getPropertyInfo,
      // getPropertyValue, ConfigFile) see identical state regardless of
      // which addProperty overload registered the property.  Unknown
      // type strings fall through to prop::Generic which echoes the
      // legacy (type, info) verbatim — every Property has a populated
      // constraint, so the synthesizers in Configurable.h always
      // succeed.
      auto [constraint, typed] = buildConstraintFromLegacy(type, info, value.str());
      p.constraint  = std::move(constraint);
      p.typed_value = std::move(typed);
      m_properties[name] = std::move(p);
      if(m_isOrdered) m_ordering[m_properties.size()] = name;
    }
  }


  const std::vector<std::string> Configurable::EMPTY_VEC;
  std::map<std::string,Configurable*, std::less<>> Configurable::m_instances;

  Configurable* Configurable::get(const std::string &id) {
    if(auto it = m_instances.find(id); it != m_instances.end()){
      return it->second;
    }else{
      return 0;
    }
  }

  Configurable::Configurable(const std::string &ID, bool ordered)
    : m_isOrdered(ordered), m_elderConfigurable(nullptr), m_ID(ID){
    if(ID.length()){
      if(get(ID)) throw ICLException(str("Configurable(")+ID+"): given ID is already used");
    }
    m_instances[ID] = this;
  }

  Configurable::Configurable(const Configurable &other){
    m_properties = other.m_properties;
    m_isOrdered = other.m_isOrdered;
    m_ordering = other.m_ordering;
    for(PropertyMap::iterator it=m_properties.begin();it!=m_properties.end();++it){
      if(it->second.configurable == &other){
        it->second.configurable = this;
      }
    }
    m_childConfigurables = other.m_childConfigurables;
    m_elderConfigurable = other.m_elderConfigurable;
    m_ID = "";
  }

  Configurable &Configurable::operator=(const Configurable &other) {
    setConfigurableID("");
    m_properties = other.m_properties;
    m_isOrdered = other.m_isOrdered;
    m_ordering = other.m_ordering;
    for(PropertyMap::iterator it=m_properties.begin();it!=m_properties.end();++it){
      if(it->second.configurable == &other){
        it->second.configurable = this;
      }
    }
    m_childConfigurables = other.m_childConfigurables;
    m_elderConfigurable = other.m_elderConfigurable;
    return *this;
  }


  void Configurable::setConfigurableID(const std::string &ID){
    if(m_ID.length()){
      if(auto it = m_instances.find(m_ID); it == m_instances.end()){
        WARNING_LOG("this should not happen");
      } else {
        m_instances.erase(it);
      }
    }
    m_ID = ID;
    if(ID != ""){
      m_instances[ID] = this;
    }
  }

  void Configurable::call_callbacks(
      const std::string &propertyName, const Configurable* caller) const
  {
  std::string propname = propertyName;
  if(m_childConfigurables.contains(caller)){
    propname = m_childConfigurables.find(caller)->second + propname;
  }
  try{
    if(callbacks.size()){
      const Property &p = prop_storage(propname);
      std::vector<Callback>::const_iterator it;
      for(it=callbacks.begin();it!=callbacks.end();++it){
        (*it)(p);
      }
    }
  }catch (ICLException &e){
    DEBUG_LOG("caught " << e.what());
    return;
  }
  if(m_elderConfigurable){
    m_elderConfigurable -> call_callbacks(propname, this);
  }
  }

  void Configurable::removedCallback(const Callback &){
    // std::function does not support equality comparison;
    // this method was never functional (empty body) and is retained
    // only for ABI compatibility.
  }
  AutoParse<std::any> Configurable::getPropertyValue(const std::string &propertyName) const{
    const Property &p = prop_storage(propertyName);
    if(p.configurable != this){
      return p.configurable->getPropertyValue(propertyName.substr(p.childPrefix.length()));
    }
    std::scoped_lock<std::recursive_mutex> lock(m_mutex);
    // Prefer the typed payload: `T x = c.getPropertyValue(name)` then
    // hits `AutoParse<any>`'s exact any_cast (or numeric-widen) fast
    // path with no parse.  Legacy-registered properties (those that
    // went through the string-taking addProperty and whose constraint
    // is therefore empty) have typed_value empty too — fall back to
    // wrapping the string value in std::any.  The cascade still handles
    // those: parse<T>(string) for numeric T, identity for std::string.
    //
    // Per-kind string formatting conventions (e.g. Flag's legacy
    // "on"/"off" vs AutoParse's generic `str(bool)` "true"/"false") are
    // not preserved through `.str()` on typed-registered properties.
    // Callers that compared the result to "on"/"off" should migrate to
    // typed reads (`if (c.getPropertyValue("flag"))` via implicit
    // bool cast).  Command properties hold `std::monostate`, which is
    // intentionally not stringifiable — `.str()` throws, which is
    // semantically correct for a button-style property.
    // Property::value retired in step 9 commit 4 — typed_value is the
    // sole storage.  For every property registered via either the
    // typed addProperty<C> overload or the legacy string-taking
    // overload (which unifies through buildConstraintFromLegacy),
    // typed_value is populated at registration time and maintained by
    // setPropertyValue / setPropertyValueTyped.
    return AutoParse<std::any>(p.typed_value);
  }

  bool Configurable::supportsProperty(const std::string &propertyName) const{
    std::vector<std::string> l = getPropertyList();
    return find(l.begin(),l.end(),propertyName) != l.end();
  }

  void Configurable::setPropertyValue(const std::string &propertyName, const AutoParse<std::string> &value){
    Property &p = prop_storage(propertyName);
    if(p.configurable != this){
      p.configurable->setPropertyValue(propertyName.substr(p.childPrefix.length()),value);
    }else{
      std::scoped_lock<std::recursive_mutex> lock(m_mutex);
      // Parse the incoming string into the declared C++ type via the
      // constraint's adapter.  Properties without a constraint
      // (core-level types like "color"/"image" registered via the
      // legacy overload, or unknown types) store the raw string in
      // typed_value.
      if(p.constraint.has_value()){
        const auto &a = prop::lookupAdapter(p.constraint.type());
        p.typed_value = a.fromString(p.constraint, value.str());
      }else{
        p.typed_value = std::any(value.str());
      }
    }
    call_callbacks(propertyName, this);
  }

  void Configurable::setPropertyValueTyped(const std::string &propertyName, std::any v){
    Property &p = prop_storage(propertyName);
    if(p.configurable != this){
      p.configurable->setPropertyValueTyped(propertyName.substr(p.childPrefix.length()),
                                            std::move(v));
    }else{
      std::scoped_lock<std::recursive_mutex> lock(m_mutex);
      // Stores the caller's typed value directly; no serialize/parse
      // round-trip.  For properties with a constraint, the caller is
      // expected to provide a value matching value_type so downstream
      // reads any_cast<T> cleanly.  Mismatched types still work via
      // AutoParse<any>'s cascade on the read side (numeric widen /
      // parse-if-string), just with a slow path.
      p.typed_value = std::move(v);
    }
    call_callbacks(propertyName, this);
  }

  void Configurable::setPropertyValueSilent(const std::string &propertyName, std::any v){
    // Mirror of setPropertyValueTyped minus the call_callbacks() tail.
    // See the declaration in Configurable.h for rationale (volatile
    // Info properties polled by VolatileUpdater don't need callbacks,
    // and firing them from a Grabber's acquireImage() under
    // m_grabMutex risks lock inversion with qt::Prop's execMutex).
    Property &p = prop_storage(propertyName);
    if(p.configurable != this){
      p.configurable->setPropertyValueSilent(propertyName.substr(p.childPrefix.length()),
                                             std::move(v));
    }else{
      std::scoped_lock<std::recursive_mutex> lock(m_mutex);
      p.typed_value = std::move(v);
    }
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



  void Configurable::saveProperties(const std::string &filename, const std::vector<std::string> &filterOUT) const{
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

      if(type == "range" || type == "value-list" || type == "range:slider" || type == "range:spinbox" || type == "float"){
        f[prop] = parse<icl32f>(val);
      }else if(type == "int"){
        f[prop] = parse<int>(val);
      }else if(type == "string"){
        f[prop] = val;
      }else if(type == "menu"){
        f[prop] = val;
      }else if(type == "flag"){
        f[prop] = parse<bool>(val) ? static_cast<bool>(1) : static_cast<bool>(0);
      }else if(type == "color"){
        f[prop] = val;
      }
    }
    f.save(filename);
  }

  // Loads properties from `f` into `conf`, dispatching on the property's
  // legacy type tag (synthesized from its constraint).  Kept as a
  // file-local helper (called twice from loadProperties below — once
  // before and once after the load pass).
  void setOptions(Configurable* conf, ConfigFile &f, const std::vector<std::string> &supported){
    for(unsigned int i=0;i<supported.size();++i){
      const std::string &propName = supported[i];
      auto h = conf->prop(propName);
      std::string type = h.type();
      if(type == "info" || type == "command") continue;
      try{
        if(type == "range" || type == "value-list" || type == "range:slider" ||
           type == "range:spinbox" || type == "float"){
          h.value = f[propName].as<float>();
        }else if(type == "int"){
          h.value = f[propName].as<int>();
        }else if(type == "string" || type == "menu" || type == "color"){
          h.value = f[propName].as<std::string>();
        }else if(type == "flag"){
          h.value = f[propName].as<bool>();
        }
      }catch(...){}
    }
  }

  void Configurable::loadProperties(const std::string &filename, const std::vector<std::string> &filterOUT){
    ConfigFile f(filename);
    f["config.title"] = std::string("Configuration file for Configurable ID:") + m_ID +  ")";
    std::vector<std::string> psSupported = getPropertyListWithoutDeactivated();

    if(filterOUT.size()){
      psSupported = remove_by_filter(psSupported,filterOUT);
    }
    f.setPrefix("config.");
    /*for(unsigned int i=0;i<psSupported.size();++i){
      std::string &prop = psSupported[i];
      std::string type = getPropertyType(prop);
      if(type == "info") continue;
      if(type == "command") continue;

      if(type == "range" || type == "value-list" || type == "range:slider" || type == "range:spinbox" || type == "float"){
        try{
          setPropertyValue(prop,str(f[prop].as<float>()));
        }catch(...){}
      }else if(type == "int"){
        try{
          setPropertyValue(prop,str(f[prop].as<int>()));
        }catch(...){}
      }else if(type == "string"){
        try{
          setPropertyValue(prop,f[prop].as<std::string>());
        }catch(...){}
      }else if(type == "menu"){
        try{
          setPropertyValue(prop,f[prop].as<std::string>());
        }catch(...){}
      }else if(type == "flag"){
        try{
          setPropertyValue(prop,f[prop].as<bool>());
        }catch(...){}
      }else if(type == "color"){
        try{
          setPropertyValue(prop,f[prop].as<std::string>());
        }catch(...){}
      }
    }*/
    // set Options twice to ensure setting of dependent properties
    setOptions(this, f, psSupported);
    setOptions(this, f, psSupported);
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

  std::vector<std::string> Configurable::getPropertyListWithoutDeactivated() const{
    if(!m_deactivated.size()) return getPropertyList();
    std::vector<std::string> passed;
    std::vector<std::string> ps = getPropertyList();
    for(unsigned int i=0;i<ps.size();++i){
      bool found = false;
      for(unsigned int j=0;j<m_deactivated.size();++j){
        if(icl::utils::match(ps[i],m_deactivated[j])){
			  //DEBUG_LOG("matched property -" << ps[i] << "- and regex -" << m_deactivated[j] << "- so the propertie is deactivated then");
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

  void Configurable::adaptProperty(const std::string &name,const std::string &newType,
                                   const std::string &newInfo, const std::string &newToolTip){
    Property &p = prop_storage(name);
    if(p.configurable != this){
      p.configurable->adaptProperty(name.substr(p.childPrefix.length()),newType,newInfo, newToolTip);
    }else{
      // Rebuild the constraint from the new (type, info) strings.
      // typed_value is left alone — adapters are expected to be called
      // for range-tightening, not kind-changing.  If a caller does flip
      // the kind (e.g. "range" → "menu") the stored typed_value may no
      // longer match the new constraint's value_type; downstream reads
      // fall through AutoParse<any>'s cascade (parse-if-string,
      // stringify-if-numeric) which is the same behavior as before
      // step 8 (p.value was the string, read-back went via parse<T>).
      auto [constraint, _] = buildConstraintFromLegacy(newType, newInfo, "");
      p.constraint = std::move(constraint);
      p.tooltip = newToolTip;
    }
  }

  typedef std::map<std::string, std::function<Configurable*()>, std::less<>> CRM;

  static CRM &get_configurable_registration_map(){
    static std::shared_ptr<CRM> crm(new CRM);
    return *crm;
  }

  void Configurable::register_configurable_type(const std::string &classname,
                                                std::function<Configurable*()> creator){
    CRM &crm = get_configurable_registration_map();
    if(auto it = crm.find(classname); it != crm.end()) throw ICLException("unable to register configurable " + classname + ": name already in use");
    crm[classname] = creator;
  }

  std::vector<std::string> Configurable::get_registered_configurables(){
    std::vector<std::string> all;
    CRM &crm = get_configurable_registration_map();
    for(CRM::iterator it = crm.begin(); it != crm.end(); ++it){
      all.push_back(it->first);
    }
    return all;
  }

  Configurable *Configurable::create_configurable(const std::string &classname){
    CRM &crm = get_configurable_registration_map();
    if(auto it = crm.find(classname); it == crm.end()){
      throw ICLException("unable to create configurable " + classname + ": name not registered");
    } else {
      return it->second();
    }
  }


  void Configurable::syncChangesTo(Configurable *others, int num){
    Configurable *src = this;
    registerCallback([src, others, num](const Configurable::Property &p){
      // Read into a string-backed AutoParse for the downstream setters;
      // getPropertyValue now returns AutoParse<std::any>, so go through
      // its .str() shim (stringifies via the cascade when the payload
      // is non-string, identity when it is).
      std::string val = src->prop(p.name).value.str();
      for(int i=0;i<num;++i){
        others[i].prop(p.name).value = val;
      }
    });
  }

  } // namespace icl::utils