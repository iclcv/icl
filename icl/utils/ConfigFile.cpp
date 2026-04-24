// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/ConfigFile.h>
#include <icl/utils/Macros.h>
#include <icl/utils/Range.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Yaml.h>

#include <fstream>
#include <sstream>

namespace icl::utils {

  // Singleton shared across the process.
  ConfigFile ConfigFile::s_oConfig;

  std::string ConfigFile::KeyRestriction::toString() const {
    if(hasRange)  return str(Range32f(min, max));
    if(hasValues) return "[" + values + "]";
    return "invalid key restriction";
  }

  // ---------------------------------------------------------------------
  // Load / save — YAML is the wire format, m_entries is canonical.
  // ---------------------------------------------------------------------

  namespace {

    // Walks `n`, joining each path segment with '.', and inserts a leaf
    // entry for every scalar reached.  Sequences inside mappings are
    // rejected (ConfigFile has always been a dotted-path flat model; a
    // synthetic `.0` / `.1` index would be a semantic invention).
    void flatten_into(const yaml::Node &n, std::string prefix,
                      std::map<std::string, ConfigFile::Entry, std::less<>> &out,
                      ConfigFile *parent){
      if(n.isScalar()){
        ConfigFile::Entry e;
        e.id     = prefix;
        e.value  = std::string(n.scalarView());
        e.parent = parent;
        out[prefix] = std::move(e);
      } else if(n.isMapping()){
        for(const auto &kv : n.mapping()){
          std::string child = prefix.empty()
              ? std::string(kv.first)
              : prefix + "." + std::string(kv.first);
          flatten_into(kv.second, std::move(child), out, parent);
        }
      } else if(n.isSequence()){
        throw InvalidFileFormatException(
          "ConfigFile: YAML sequences at '" + prefix + "' are not supported");
      }
      // Null nodes are silently skipped (e.g. an empty mapping value).
    }

    // Splits `id` on '.' and walks / auto-vivifies nested mappings,
    // returning a reference to the leaf node where the scalar goes.
    yaml::Node &walk_and_create(yaml::Document &d, const std::string &id){
      yaml::Node *cur = &d.root();
      std::size_t start = 0;
      while(start < id.size()){
        std::size_t dot = id.find('.', start);
        std::string seg = (dot == std::string::npos)
            ? id.substr(start)
            : id.substr(start, dot - start);
        cur = &(*cur)[seg];
        if(dot == std::string::npos) break;
        start = dot + 1;
      }
      return *cur;
    }

  }  // anonymous

  ConfigFile::ConfigFile() = default;

  ConfigFile::ConfigFile(const std::string &filename){
    load(filename);
  }

  ConfigFile::ConfigFile(std::istream &stream){
    std::ostringstream oss;
    oss << stream.rdbuf();
    auto doc = yaml::Document::own(oss.str());
    m_entries.clear();
    flatten_into(doc.root(), "", m_entries, this);
  }

  void ConfigFile::load(const std::string &filename){
    auto doc = yaml::Document::file(filename);
    m_entries.clear();
    flatten_into(doc.root(), "", m_entries, this);
  }

  void ConfigFile::save(const std::string &filename) const {
    auto doc = yaml::Document::empty();
    for(const auto &[id, e] : m_entries){
      walk_and_create(doc, id) = e.value;
    }
    doc.emitToFile(filename);
  }

  std::ostream &operator<<(std::ostream &s, const ConfigFile &cf){
    auto doc = yaml::Document::empty();
    for(const auto &[id, e] : cf.m_entries){
      walk_and_create(doc, id) = e.value;
    }
    s << doc.emit();
    return s;
  }

  // ---------------------------------------------------------------------
  // Static singleton helpers
  // ---------------------------------------------------------------------

  void ConfigFile::loadConfig(const std::string &filename){
    s_oConfig.load(filename);
  }

  void ConfigFile::loadConfig(const ConfigFile &configFile){
    s_oConfig = configFile;
  }

  // ---------------------------------------------------------------------
  // Entry mutation / query
  // ---------------------------------------------------------------------

  void ConfigFile::set_internal(const std::string &idIn, const std::string &val){
    const std::string id = m_sDefaultPrefix + idIn;
    Entry &e = m_entries[id];
    e.parent = this;
    e.id     = id;
    e.value  = val;
  }

  ConfigFile::Entry &ConfigFile::get_entry_internal(const std::string &id){
    auto it = m_entries.find(id);
    if(it == m_entries.end()) throw EntryNotFoundException(id);
    return it->second;
  }

  const ConfigFile::Entry &ConfigFile::get_entry_internal(const std::string &id) const {
    auto it = m_entries.find(id);
    if(it == m_entries.end()) throw EntryNotFoundException(id);
    return it->second;
  }

  void ConfigFile::setRestriction(const std::string &id, const KeyRestriction &r){
    get_entry_internal(m_sDefaultPrefix + id).restr = std::make_shared<KeyRestriction>(r);
  }

  const ConfigFile::KeyRestriction *ConfigFile::getRestriction(const std::string &id) const {
    return get_entry_internal(m_sDefaultPrefix + id).restr.get();
  }

  bool ConfigFile::contains(const std::string &id) const {
    return m_entries.contains(m_sDefaultPrefix + id);
  }

  void ConfigFile::clear(){
    m_entries.clear();
    m_sDefaultPrefix.clear();
  }

  void ConfigFile::listContents() const {
    std::cout << "config file entries:\n";
    for(const auto &[key, e] : m_entries){
      std::cout << e.id << "\t" << e.value;
      if(e.restr) std::cout << "\t(restriction: " << e.restr->toString() << ")";
      std::cout << "\n";
    }
  }

  // ---------------------------------------------------------------------
  // Prefix + operator[] + Data proxy + find
  // ---------------------------------------------------------------------

  void ConfigFile::setPrefix(const std::string &defaultPrefix) const {
    m_sDefaultPrefix = defaultPrefix;
  }

  const std::string &ConfigFile::getPrefix() const {
    return m_sDefaultPrefix;
  }

  ConfigFile::Data::Data(const std::string &id, ConfigFile &cf) : id(id), cf(&cf) {}

  ConfigFile::Data ConfigFile::operator[](const std::string &id){
    return Data(id, *this);
  }

  const ConfigFile::Data ConfigFile::operator[](const std::string &id) const {
    if(!contains(id)) throw EntryNotFoundException(m_sDefaultPrefix + id);
    return Data(id, const_cast<ConfigFile &>(*this));
  }

  std::vector<ConfigFile::Data> ConfigFile::find(const std::string &regex){
    std::vector<Data> ret;
    for(const auto &[key, _] : m_entries){
      if(match(key, regex)) ret.push_back(Data(key, *this));
    }
    return ret;
  }

}  // namespace icl::utils
