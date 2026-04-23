// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/detail/yaml/YamlEmitter.h>
#include <icl/utils/detail/yaml/YamlScalar.h>

#include <algorithm>
#include <cstdio>
#include <string>
#include <string_view>

namespace icl::utils::yaml::detail {
  namespace {

    // Characters that make a plain scalar dangerous at the start or
    // mid-scalar.  Keep conservative — when in doubt, quote.
    bool isPlainSafe(std::string_view s){
      if(s.empty()) return false;  // empty means Null in plain form

      // Forbid leading indicators per YAML 1.2 block-in rules.
      const char c0 = s.front();
      switch(c0){
        case '-': case '?': case ':': case ',':
        case '[': case ']': case '{': case '}':
        case '#': case '&': case '*': case '!':
        case '|': case '>': case '\'': case '"':
        case '%': case '@': case '`':
          return false;
        default:
          break;
      }
      // Forbid leading whitespace / trailing whitespace — YAML strips them.
      if(c0 == ' ' || c0 == '\t' || s.back() == ' ' || s.back() == '\t') return false;

      for(std::size_t i = 0; i < s.size(); ++i){
        const unsigned char c = static_cast<unsigned char>(s[i]);
        // Control chars → must be quoted+escaped
        if(c < 0x20 || c == 0x7F) return false;
        // ` #` (space before hash) looks like a comment
        if(c == '#' && i > 0 && (s[i-1] == ' ' || s[i-1] == '\t')) return false;
        // `: ` (colon followed by space) terminates a mapping key
        if(c == ':' && i + 1 < s.size() && (s[i+1] == ' ' || s[i+1] == '\t')) return false;
        if(c == ':' && i + 1 == s.size()) return false;  // trailing colon
      }
      return true;
    }

    // Emit a double-quoted scalar with standard escapes.
    void emitDoubleQuoted(std::string_view s, std::string &out){
      out.push_back('"');
      for(char ch : s){
        const unsigned char c = static_cast<unsigned char>(ch);
        switch(c){
          case '"':  out += "\\\""; break;
          case '\\': out += "\\\\"; break;
          case '\n': out += "\\n";  break;
          case '\t': out += "\\t";  break;
          case '\r': out += "\\r";  break;
          case '\0': out += "\\0";  break;
          default:
            if(c < 0x20 || c == 0x7F){
              char buf[8];
              std::snprintf(buf, sizeof(buf), "\\x%02X", c);
              out += buf;
            } else {
              out.push_back(ch);
            }
            break;
        }
      }
      out.push_back('"');
    }

    // Emit a single-quoted scalar (no escapes; ' is doubled).
    void emitSingleQuoted(std::string_view s, std::string &out){
      out.push_back('\'');
      for(char c : s){
        out.push_back(c);
        if(c == '\'') out.push_back('\'');
      }
      out.push_back('\'');
    }

    bool hasControlChars(std::string_view s){
      for(char c : s){
        const unsigned char u = static_cast<unsigned char>(c);
        if(u < 0x20 || u == 0x7F) return true;
      }
      return false;
    }

    // Effective scalar bytes: prefer the self-owned string (populated
    // by operator= / setOwnedScalar) over the view.
    std::string_view effectiveView(const Node::ScalarData &sd){
      return !sd.owned.empty() ? std::string_view(sd.owned) : sd.sv;
    }

    // Emit a scalar respecting its style hint (round-tripping), and
    // falling back to a safe quoting if the hinted plain form is unsafe
    // or would change the inferred kind.
    void emitScalar(const Node::ScalarData &sd, std::string &out){
      const std::string_view body = effectiveView(sd);
      // Explicit tag prefix — re-emit `!!name ` so round-trip preserves
      // the forced type.  Tag names must match the parser's vocabulary.
      if(sd.explicitTag){
        switch(*sd.explicitTag){
          case ScalarKind::String: out += "!!str ";   break;
          case ScalarKind::Int:    out += "!!int ";   break;
          case ScalarKind::Float:  out += "!!float "; break;
          case ScalarKind::Bool:   out += "!!bool ";  break;
          case ScalarKind::Null:   out += "!!null ";  break;
        }
      }
      switch(sd.style){
        case ScalarStyle::DoubleQuoted:
          emitDoubleQuoted(body, out);
          return;
        case ScalarStyle::SingleQuoted:
          // single-quoted can't carry control chars; re-promote to double if needed
          if(hasControlChars(body)) emitDoubleQuoted(body, out);
          else                      emitSingleQuoted(body, out);
          return;
        case ScalarStyle::Literal:
        case ScalarStyle::Folded:
          // Block-scalar round-trip is not supported for emission in
          // Phase 1 — we re-emit them double-quoted (safe, unambiguous).
          // A proper `|` / `>` emitter can be added later if needed.
          emitDoubleQuoted(body, out);
          return;
        case ScalarStyle::Plain:
        default:
          break;
      }

      // Plain style: only use it if (a) it's syntactically safe and
      // (b) it re-resolves to the same kind.  Otherwise single- or
      // double-quote.  Example: the value "42" as an intended string
      // must be quoted, else it would re-parse as Int.
      if(isPlainSafe(body)){
        out += body;
        return;
      }

      if(hasControlChars(body)) emitDoubleQuoted(body, out);
      else                      emitSingleQuoted(body, out);
    }

    void appendIndent(std::string &out, int spaces){
      out.append(static_cast<std::size_t>(spaces), ' ');
    }

    void emitNode(const Node &n, std::string &out, int level, const EmitOptions &opts);

    bool allScalars(const Node::Sequence &seq){
      for(const Node &e : seq) if(!e.isScalar()) return false;
      return true;
    }

    void emitSequenceFlow(const Node::Sequence &seq, std::string &out){
      out.push_back('[');
      for(std::size_t i = 0; i < seq.size(); ++i){
        if(i) out += ", ";
        emitScalar(std::get<Node::ScalarData>(seq[i].value()), out);
      }
      out.push_back(']');
    }

    void emitSequenceBlock(const Node::Sequence &seq, std::string &out, int level, const EmitOptions &opts){
      if(seq.empty()){ out += "[]"; return; }
      for(std::size_t i = 0; i < seq.size(); ++i){
        if(i) out.push_back('\n');  // separator between entries; caller owns any leading newline
        appendIndent(out, level * opts.indent);
        out += "- ";
        const Node &e = seq[i];
        if(e.isScalar()){
          emitScalar(std::get<Node::ScalarData>(e.value()), out);
        } else if(e.isNull()){
          out += "~";
        } else {
          // For mappings/sequences the body goes on the next line with
          // deeper indent.  Exception: for a mapping, it's idiomatic to
          // place the first key on the same line as the `- `.  We use
          // the simpler always-newline form for predictability.
          out.push_back('\n');
          emitNode(e, out, level + 1, opts);
        }
      }
    }

    void emitMappingBlock(const Node::Mapping &map, std::string &out, int level, const EmitOptions &opts){
      if(map.empty()){ out += "{}"; return; }

      // Build an iteration order — insertion-order by default, sorted
      // lexicographically by key when `opts.sortKeys` is set.
      std::vector<std::size_t> order;
      order.reserve(map.size());
      for(std::size_t i = 0; i < map.size(); ++i) order.push_back(i);
      if(opts.sortKeys){
        std::sort(order.begin(), order.end(),
                  [&](std::size_t a, std::size_t b){ return map[a].first < map[b].first; });
      }

      for(std::size_t ii = 0; ii < order.size(); ++ii){
        const std::size_t i = order[ii];
        if(ii) out.push_back('\n');  // separator between entries; caller owns any leading newline
        appendIndent(out, level * opts.indent);
        // Keys: we require keys to be string-ish and plain-safe.  If a
        // key contains `:`/`#`/etc. we single-quote it.
        const std::string_view key = map[i].first;
        if(isPlainSafe(key) && !key.empty()){
          out += key;
        } else {
          if(hasControlChars(key)) emitDoubleQuoted(key, out);
          else                     emitSingleQuoted(key, out);
        }
        out.push_back(':');

        const Node &v = map[i].second;
        if(v.isNull()){
          out += " ~";
        } else if(v.isScalar()){
          out.push_back(' ');
          emitScalar(std::get<Node::ScalarData>(v.value()), out);
        } else if(v.isSequence()){
          const Node::Sequence &seq = std::get<Node::Sequence>(v.value());
          if(seq.empty()){
            out += " []";
          } else if(allScalars(seq) && static_cast<int>(seq.size()) <= opts.flowThreshold){
            out.push_back(' ');
            emitSequenceFlow(seq, out);
          } else {
            // block sequence on next line, indented one level.  YAML
            // technically allows same-indent ("`-` under a mapping key
            // at zero extra indent"), but round-tripping through our
            // parser is unambiguous only at level+1.
            out.push_back('\n');
            emitSequenceBlock(seq, out, level + 1, opts);
          }
        } else if(v.isMapping()){
          const Node::Mapping &mm = std::get<Node::Mapping>(v.value());
          if(mm.empty()){
            out += " {}";
          } else {
            out.push_back('\n');
            emitMappingBlock(mm, out, level + 1, opts);
          }
        }
      }
    }

    void emitNode(const Node &n, std::string &out, int level, const EmitOptions &opts){
      if(n.isNull()){
        appendIndent(out, level * opts.indent);
        out += "~";
      } else if(n.isScalar()){
        appendIndent(out, level * opts.indent);
        emitScalar(std::get<Node::ScalarData>(n.value()), out);
      } else if(n.isSequence()){
        const Node::Sequence &seq = std::get<Node::Sequence>(n.value());
        if(seq.empty()){
          appendIndent(out, level * opts.indent);
          out += "[]";
        } else if(level == 0 && allScalars(seq) && static_cast<int>(seq.size()) <= opts.flowThreshold){
          emitSequenceFlow(seq, out);
        } else {
          emitSequenceBlock(seq, out, level, opts);
        }
      } else if(n.isMapping()){
        const Node::Mapping &map = std::get<Node::Mapping>(n.value());
        if(map.empty()){
          appendIndent(out, level * opts.indent);
          out += "{}";
        } else {
          emitMappingBlock(map, out, level, opts);
        }
      }
    }

  }  // anonymous

  std::string emit(const Node &n, const EmitOptions &opts){
    std::string out;
    emitNode(n, out, 0, opts);
    if(!out.empty() && out.back() != '\n') out.push_back('\n');
    return out;
  }

}  // namespace icl::utils::yaml::detail
