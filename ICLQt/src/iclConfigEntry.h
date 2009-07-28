#ifndef ICL_CONFIG_ENTRY_H
#define ICL_CONFIG_ENTRY_H

#include <iclConfigFile.h>
#include <iclPoint.h>
#include <iclPoint32f.h>
#include <iclSize.h>
#include <iclSize32f.h>
#include <iclRect.h>
#include <iclRect32f.h>
#include <iclRange.h>

namespace icl{

  
  /// Utility class for referencing runtime-dynamic ConfigFile entries
  /** TODO: put in a use case here ... */
  template<class T>
  struct ConfigEntry{
    
    /// Create an empty default Entry referencing a default constructed T
    inline ConfigEntry():m_def(T()),m_key(&m_def),m_config(0){}
    
    /// Creates a new ConfigEntry instance
    /** @param key reference key
        @param default value that should be used if key is not found 
        @param cfg config file to use (note: each entry remains valid as long 
               its parent config file remains allocated */
    inline ConfigEntry(const std::string &key,
                       const T &def=T(),
                       const ConfigFile &cfg=ConfigFile::getConfig()) throw (ConfigFile::InvalidTypeException){
      
      m_config = const_cast<ConfigFile*>(&cfg);
      m_config->lock();
      if(cfg.contains(key)){
        T test = cfg[key]; // this causes an InvalidTypeException
        (void) test;
        m_key = key;
      }else{
        m_def = def;
      }
      m_config->unlock();
    }
    

    /// returns reference value
    operator T() const{
      ICLASSERT_RETURN_VAL(m_config,T());
      Mutex::Locker l(*m_config);
      return m_key.size() ? (*m_config)[m_key] : m_def;
    }
    void debug_show(const std::string &key="any name"){
      DEBUG_LOG("config_entry: " << key);
      DEBUG_LOG("def= " << m_def << "  entry_ptr = " << m_key << "  entry_val=" << (T)(*this) << "  config_ptr=" << m_config);
    }
  private:
    T m_def;
    std::string m_key;
    ConfigFile *m_config;
  };
  
#define ICL_INSTANTIATE_DEPTH(D) typedef ConfigEntry<icl##D> Cfg##D;
  ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
  typedef  ConfigEntry<Point> CfgPoint;
  typedef  ConfigEntry<Point32f> CfgPoint32f;
  typedef  ConfigEntry<Size> CfgSize;
  typedef  ConfigEntry<Size32f> CfgSize32f;
  typedef  ConfigEntry<Rect> CfgRect;
  typedef  ConfigEntry<Rect32f> CfgRect32f;
  typedef  ConfigEntry<Range32s> CfgRange32s;
  typedef  ConfigEntry<Range32f> CfgRange32f;
}


#endif
