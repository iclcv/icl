// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/Macros.h>
#include <icl/utils/Any.h>
#include <functional>
#include <map>

namespace icl::utils {
  /// Utility structure that utilizes an std::map as parameter list
  /** The ParamList is supposed to be used in function interfaces
      for passing several parameters as one argument. Usually this
      should only be used where efficiency is not compulsory */
  struct ParamList : public std::map<std::string, Any, std::less<>>{

    /// The Key type (std::string)
    using Key = std::string;

    /// The Value type (icl::Any)
    using Value = Any;

    /// creates an empty param list instance
    inline ParamList(){}

    /// creates a param list from a single given string
    /** The string is a comma seperated list of key=value tokens. Both
        comma and the '='-character can be escaped using backslash */
    inline ParamList(const std::string &commaSepKeyValueString){
      init(commaSepKeyValueString);
    }

    /// this allows for implicit creation of a ParamList instance from a given const char *
    inline ParamList(const char *commaSepKeyValueString){
      init(commaSepKeyValueString);
    }

    inline void init(const std::string commaSepKeyValueString){
      std::vector<std::string> ts = tok(commaSepKeyValueString,",",true,'\\');
      for(size_t i=0;i<ts.size();++i){
        std::vector<std::string> kv = tok(ts[i],"=",true,'\\');
        ICLASSERT_THROW(kv.size() == 2, ICLException("ParamList(string): invalid token :'"
                                                    + ts[i] + "'"));
        const_cast<ParamList*>(this)->operator[](kv[0]) = kv[1];
      }
    }


    /// Constructor, that can get up to 10 key-value pairs
    /** zero-length keys are skipped! */
    inline ParamList(const Key &key0, const Value &value0,
                     const Key &key1="", const Value &value1="",
                     const Key &key2="", const Value &value2="",
                     const Key &key3="", const Value &value3="",
                     const Key &key4="", const Value &value4="",
                     const Key &key5="", const Value &value5="",
                     const Key &key6="", const Value &value6="",
                     const Key &key7="", const Value &value7="",
                     const Key &key8="", const Value &value8="",
                     const Key &key9="", const Value &value9="" ){
      if(key0.length()) operator[](key0) = value0;
      if(key1.length()) operator[](key1) = value1;
      if(key2.length()) operator[](key2) = value2;
      if(key3.length()) operator[](key3) = value3;
      if(key4.length()) operator[](key4) = value4;
      if(key5.length()) operator[](key5) = value5;
      if(key6.length()) operator[](key6) = value6;
      if(key7.length()) operator[](key7) = value7;
      if(key8.length()) operator[](key8) = value8;
      if(key9.length()) operator[](key9) = value9;
    }

    /// returns whether the map contains the given key
    /** This is just a shortcut to the find()-method that is provided by the std::map */
    inline bool hasKey(const Key &key) const { return find(key) != end(); }

    /// removes the given key from the map
    /** This is just a shortcut to the find()-method that is provided by the std::map */
    inline void removeKey(const Key &key){
      iterator it = find(key);
      if(it != end()) erase(it);
    }

    /// extension for the unconst operator that is provided by the std::map class
    inline const Any &operator[](const Key &key) const{
      const_iterator it = find(key);
      if(it == end()) throw ICLException("error in ParamList::operator[](key) with key=" + key + ": key not found!");
      return it->second;
    }

    /// also provides the map's not-const index operator
    using std::map<std::string,Any, std::less<>>::operator[];
  };


  } // namespace icl::utils