// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Exception.h>
#include <icl/utils/Lockable.h>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace icl::utils {

  /// Hierarchical key/value config store, YAML-backed.
  ///
  /// Keys are dotted paths (`general.params.threshold`) that map to a
  /// nested YAML mapping at load/save time.  In memory the entries are
  /// held as a flat `std::map`; YAML is used only as the on-disk wire
  /// format.  The class is typeless on the wire: scalars are stored as
  /// raw text, and the caller's `get<T>` template parameter determines
  /// the parse target.  A bad parse throws the usual `ICLException`
  /// from `parse<T>`.
  ///
  /// Example:
  /// \code
  /// ConfigFile cfg;
  /// cfg["general.params.threshold"] = 7;
  /// cfg["general.params.value"]     = 6.45;
  /// cfg["general.params.filename"]  = std::string("./hallo.txt");
  /// cfg["special.hint"]             = 'a';
  /// cfg.save("config.yaml");
  /// \endcode
  ///
  /// produces:
  /// \code
  /// general:
  ///   params:
  ///     threshold: 7
  ///     value: 6.45
  ///     filename: ./hallo.txt
  /// special:
  ///   hint: a
  /// \endcode
  ///
  /// `setPrefix("general.params.")` then lets subsequent accesses omit
  /// the prefix.  There is no forced root element (the historical
  /// `config.` prefix from the old XML format is now just a convention).
  class ICLUtils_API ConfigFile : public Lockable {

    public:

    /// Thrown from `get<T>` if the requested key is not in the store.
    struct EntryNotFoundException : public ICLException {
      EntryNotFoundException(const std::string &entryName) :
        ICLException("Entry " + entryName + " could not be found!") {}
      virtual ~EntryNotFoundException() noexcept {}
    };

    /// Empty store.
    ConfigFile();

    /// Load from `filename` (YAML).  If loading fails the exception
    /// from the underlying parser is propagated unchanged.
    ConfigFile(const std::string &filename);

    /// Load from an input stream (YAML).
    ConfigFile(std::istream &stream);

    /// Load from `filename` — clears existing entries first.
    void load(const std::string &filename);

    /// Save the full entry set to `filename` as YAML.
    void save(const std::string &filename) const;

    /// Default prefix applied to every `operator[]` / `get` / `set`
    /// key.  e.g. after `setPrefix("general.params.")` the access
    /// `cfg["threshold"]` reads `general.params.threshold`.
    void setPrefix(const std::string &defaultPrefix) const;
    const std::string &getPrefix() const;

    /// Proxy returned by `operator[]` — assign to it to `set`, or
    /// copy/cast it to any type to `get`.
    class Data {
      std::string id;
      ConfigFile *cf;
      Data(const std::string &id, ConfigFile &cf);
    public:
      friend class ConfigFile;

      Data(const Data &) = default;            // quiet -Wdeprecated-copy

      template<class T>
      operator T() const { return cf->get<T>(id); }

      template<class T>
      T as() const { return cf->get<T>(id); }

      Data &operator=(const Data &d){
        cf = d.cf; id = d.id; return *this;
      }

      template<class T>
      Data &operator=(const T &t){
        cf->set(id, t);
        return *this;
      }
    };

    Data       operator[](const std::string &id);
    const Data operator[](const std::string &id) const;

    std::vector<Data>       find(const std::string &regex);
    const std::vector<Data> find(const std::string &regex) const {
      return const_cast<ConfigFile *>(this)->find(regex);
    }

    /// Store `val` under `id` as its stringified representation.
    template<class T>
    void set(const std::string &id, const T &val){
      set_internal(id, str(val));
    }

    /// Read `id` and parse its stored text as T.  Throws
    /// `EntryNotFoundException` if the key is missing; parse failure
    /// throws `ICLException` from `parse<T>`.
    template<class T>
    inline T get(const std::string &idIn) const {
      const std::string key = m_sDefaultPrefix + idIn;
      auto it = m_entries.find(key);
      if(it == m_entries.end()) throw EntryNotFoundException(key);
      return parse<T>(it->second.value);
    }

    /// As above, but returns `def` on missing key.
    template<class T>
    inline T get(const std::string &idIn, const T &def) const {
      auto it = m_entries.find(m_sDefaultPrefix + idIn);
      if(it == m_entries.end()) return def;
      return parse<T>(it->second.value);
    }

    /// Static singleton helpers.
    static void loadConfig(const std::string &filename);
    static void loadConfig(const ConfigFile &configFile);
    static const ConfigFile &getConfig(){ return s_oConfig; }

    template<class T>
    static inline T sget(const std::string &id){
      return getConfig().get<T>(id);
    }
    template<class T>
    static inline T sget(const std::string &id, const T &def){
      return getConfig().get<T>(id, def);
    }

    void listContents() const;

    bool contains(const std::string &id) const;

    /// Optional UI-hint metadata: a numeric range or enum value list.
    /// In-memory only — not persisted to the YAML file.
    struct KeyRestriction {
      inline KeyRestriction() : hasRange(false), hasValues(false) {}
      inline KeyRestriction(double min, double max) :
        hasRange(true), hasValues(false) {
          this->min = min; this->max = max;
      }
      inline KeyRestriction(const std::string &values) :
        values(values), hasRange(false), hasValues(true) {}
      ICLUtils_API std::string toString() const;

      double      min, max;
      std::string values;
      bool        hasRange;
      bool        hasValues;
    };

    void                 setRestriction(const std::string &id, const KeyRestriction &r);
    const KeyRestriction *getRestriction(const std::string &id) const;

    /// Single entry in the flat store.  `id` is the full dotted key
    /// (including any default prefix), `value` is the raw text.
    struct Entry {
      std::string                     id;
      std::string                     value;
      std::shared_ptr<KeyRestriction> restr;
      ConfigFile                      *parent = nullptr;

      std::string getRelID() const {
        const std::string &pfx = parent->getPrefix();
        if(!pfx.length()) return id;
        return id.substr(pfx.length());
      }
    };

    using const_iterator = std::map<std::string, Entry, std::less<>>::const_iterator;
    const_iterator begin() const { return m_entries.begin(); }
    const_iterator end()   const { return m_entries.end();   }

    const std::vector<const Entry *> getEntryList(bool /*relToPrefix*/ = false) const {
      std::vector<const Entry *> v;
      v.reserve(m_entries.size());
      for(const auto &kv : m_entries) v.push_back(&kv.second);
      return v;
    }

    /// Drop all entries.
    void clear();

    private:

    Entry       &get_entry_internal(const std::string &id);
    const Entry &get_entry_internal(const std::string &id) const;
    void         set_internal(const std::string &id, const std::string &val);

    static ConfigFile           s_oConfig;
    mutable std::string         m_sDefaultPrefix;
    std::map<std::string, Entry, std::less<>> m_entries;

    friend ICLUtils_API std::ostream &operator<<(std::ostream &, const ConfigFile &);
  };

  ICLUtils_API std::ostream &operator<<(std::ostream &s, const ConfigFile &cf);

  /// Char-array overloads for `Data = "literal"` — forwards to the
  /// `std::string` overload, since `char *` isn't stream-extractable.
  template<> inline ConfigFile::Data &ConfigFile::Data::operator=(char * const &t){
    return ConfigFile::Data::operator=(std::string(t));
  }
  template<> inline ConfigFile::Data &ConfigFile::Data::operator=(const char * const &t){
    return ConfigFile::Data::operator=(std::string(t));
  }

}  // namespace icl::utils
