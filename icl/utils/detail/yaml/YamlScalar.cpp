// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/detail/yaml/YamlScalar.h>

#include <cctype>

namespace icl::utils::yaml::detail {
  namespace {

    // Small equality helpers — avoid pulling Boost/StringUtils for such
    // tiny fixed vocabulary.  Case matching follows YAML 1.2 core schema:
    // `null/Null/NULL`, `true/True/TRUE`, `false/False/FALSE`.

    bool isNullLiteral(std::string_view s){
      return s.empty() || s == "~" || s == "null" || s == "Null" || s == "NULL";
    }

    bool isBoolLiteral(std::string_view s){
      return s == "true"  || s == "True"  || s == "TRUE"
          || s == "false" || s == "False" || s == "FALSE";
    }

    bool isDigit(char c){ return c >= '0' && c <= '9'; }
    bool isHexDigit(char c){
      return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }
    bool isOctalDigit(char c){ return c >= '0' && c <= '7'; }

    // YAML 1.2 core-schema integer grammar: [-+]? ( 0 | [1-9][0-9]* | 0x[0-9a-fA-F]+ | 0o[0-7]+ )
    bool isIntLiteral(std::string_view s){
      if(s.empty()) return false;
      std::size_t i = 0;
      if(s[0] == '+' || s[0] == '-') ++i;
      if(i >= s.size()) return false;
      // hex / oct prefix?
      if(s.size() - i >= 2 && s[i] == '0' && (s[i+1] == 'x' || s[i+1] == 'X')){
        i += 2;
        if(i >= s.size()) return false;
        for(; i < s.size(); ++i) if(!isHexDigit(s[i])) return false;
        return true;
      }
      if(s.size() - i >= 2 && s[i] == '0' && (s[i+1] == 'o' || s[i+1] == 'O')){
        i += 2;
        if(i >= s.size()) return false;
        for(; i < s.size(); ++i) if(!isOctalDigit(s[i])) return false;
        return true;
      }
      // plain decimal: 0 | [1-9][0-9]*
      if(s[i] == '0'){
        // "0" alone is Int; "0x" handled above; "0." would be a float, caught later.
        return i + 1 == s.size();
      }
      if(!isDigit(s[i])) return false;
      for(; i < s.size(); ++i) if(!isDigit(s[i])) return false;
      return true;
    }

    // Case-insensitive compare of `s` starting at `i` against a lowercase literal.
    bool iEqualsFrom(std::string_view s, std::size_t i, std::string_view lit){
      if(s.size() - i != lit.size()) return false;
      for(std::size_t k = 0; k < lit.size(); ++k){
        char c = s[i + k];
        if(c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
        if(c != lit[k]) return false;
      }
      return true;
    }

    // YAML 1.2 core-schema float grammar (approximate — we accept a
    // slightly broader set and let `parse<float/double>` do final
    // validation if the user calls as<float>()):
    //   [-+]? ( \d+\.\d* | \.\d+ | \d+(\.\d*)?[eE][-+]?\d+ | \.inf | \.nan )
    bool isFloatLiteral(std::string_view s){
      if(s.empty()) return false;
      std::size_t i = 0;
      if(s[0] == '+' || s[0] == '-') ++i;
      if(i >= s.size()) return false;

      // .inf / .nan (case-insensitive)
      if(s[i] == '.'){
        if(iEqualsFrom(s, i + 1, "inf")) return true;
        if(iEqualsFrom(s, i + 1, "nan")) return true;
      }

      // Need at least one digit before or after a '.'
      const std::size_t start = i;
      bool sawDigit = false;
      while(i < s.size() && isDigit(s[i])){ ++i; sawDigit = true; }
      bool sawDot = false;
      if(i < s.size() && s[i] == '.'){
        sawDot = true;
        ++i;
        while(i < s.size() && isDigit(s[i])){ ++i; sawDigit = true; }
      }
      if(!sawDigit) return false;
      // optional exponent
      if(i < s.size() && (s[i] == 'e' || s[i] == 'E')){
        ++i;
        if(i < s.size() && (s[i] == '+' || s[i] == '-')) ++i;
        if(i >= s.size() || !isDigit(s[i])) return false;
        while(i < s.size() && isDigit(s[i])) ++i;
      } else if(!sawDot){
        // Integer-shaped: leave to isIntLiteral to claim.  "42" is Int, not Float.
        // But "42e3" above IS Float (we already consumed the exponent).
        return false;
      }
      (void)start;
      return i == s.size();
    }

  }  // anonymous

  ScalarKind resolveScalarKind(std::string_view sv, ScalarStyle style){
    if(style != ScalarStyle::Plain){
      return ScalarKind::String;  // quoting forces String
    }
    if(isNullLiteral(sv))  return ScalarKind::Null;
    if(isBoolLiteral(sv))  return ScalarKind::Bool;
    if(isIntLiteral(sv))   return ScalarKind::Int;
    if(isFloatLiteral(sv)) return ScalarKind::Float;
    return ScalarKind::String;
  }

}  // namespace icl::utils::yaml::detail
