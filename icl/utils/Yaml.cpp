// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/Yaml.h>

#include <icl/utils/detail/yaml/YamlEmitter.h>
#include <icl/utils/detail/yaml/YamlParser.h>
#include <icl/utils/detail/yaml/YamlScalar.h>

#include <fstream>
#include <sstream>

namespace icl::utils::yaml {

  // ---------------------------------------------------------------------
  // Exception ctors
  // ---------------------------------------------------------------------

  ParseError::ParseError(std::size_t line, std::size_t col, const std::string &what)
    : ICLException("yaml::ParseError[" + std::to_string(line) + ":" + std::to_string(col) + "] " + what),
      m_line(line), m_col(col) {}

  TypeError::TypeError(const std::string &what)
    : ICLException("yaml::TypeError: " + what) {}

  // ---------------------------------------------------------------------
  // Node
  // ---------------------------------------------------------------------

  std::size_t Node::size() const noexcept {
    if(const auto *s = std::get_if<Sequence>(&m_value)) return s->size();
    if(const auto *m = std::get_if<Mapping>(&m_value))  return m->size();
    return 0;
  }

  ScalarKind Node::scalarKind() const {
    const auto *s = std::get_if<ScalarData>(&m_value);
    if(!s) throw TypeError("scalarKind: node is not a scalar");
    if(s->explicitTag){
      // Explicit tag overrides content-based resolution.
      return *s->explicitTag;
    }
    if(!s->kindResolved){
      // Use the effective view so self-owned scalars (from operator= /
      // setOwnedScalar) resolve their kind from the right bytes.
      const std::string_view body = !s->owned.empty() ? std::string_view(s->owned) : s->sv;
      s->cachedKind   = detail::resolveScalarKind(body, s->style);
      s->kindResolved = true;
    }
    return s->cachedKind;
  }

  ScalarStyle Node::scalarStyle() const {
    const auto *s = std::get_if<ScalarData>(&m_value);
    if(!s) throw TypeError("scalarStyle: node is not a scalar");
    return s->style;
  }

  bool Node::isQuoted() const {
    const auto *s = std::get_if<ScalarData>(&m_value);
    if(!s) return false;
    return s->style == ScalarStyle::SingleQuoted || s->style == ScalarStyle::DoubleQuoted;
  }

  std::string_view Node::scalarView() const {
    const auto *s = std::get_if<ScalarData>(&m_value);
    if(!s) throw TypeError("scalarView: node is not a scalar");
    // Self-owned scalars (from operator= / setOwnedScalar) take
    // precedence over the view.  This is re-derived each call so
    // moves of ScalarData never leave a dangling view.
    return !s->owned.empty() ? std::string_view(s->owned) : s->sv;
  }

  const Node& Node::at(std::size_t i) const {
    const auto *s = std::get_if<Sequence>(&m_value);
    if(!s) throw TypeError("at(size_t): node is not a sequence");
    if(i >= s->size()) throw TypeError("at(size_t): index out of range");
    return (*s)[i];
  }
  Node& Node::at(std::size_t i){
    auto *s = std::get_if<Sequence>(&m_value);
    if(!s) throw TypeError("at(size_t): node is not a sequence");
    if(i >= s->size()) throw TypeError("at(size_t): index out of range");
    return (*s)[i];
  }

  Node& Node::pushBack(Node n){
    if(isNull()) setSequence();
    auto *s = std::get_if<Sequence>(&m_value);
    if(!s) throw TypeError("pushBack: node is not a sequence");
    s->push_back(std::move(n));
    return s->back();
  }

  bool Node::contains(std::string_view key) const {
    const auto *m = std::get_if<Mapping>(&m_value);
    if(!m) return false;
    for(const auto &kv : *m) if(kv.first == key) return true;
    return false;
  }

  const Node* Node::find(std::string_view key) const {
    const auto *m = std::get_if<Mapping>(&m_value);
    if(!m) return nullptr;
    for(const auto &kv : *m) if(kv.first == key) return &kv.second;
    return nullptr;
  }
  Node* Node::find(std::string_view key){
    auto *m = std::get_if<Mapping>(&m_value);
    if(!m) return nullptr;
    for(auto &kv : *m) if(kv.first == key) return &kv.second;
    return nullptr;
  }

  const Node& Node::get(std::string_view key) const {
    const Node *n = find(key);
    if(!n) throw TypeError("get: mapping has no such key (or node is not a mapping)");
    return *n;
  }
  Node& Node::get(std::string_view key){
    Node *n = find(key);
    if(!n) throw TypeError("get: mapping has no such key (or node is not a mapping)");
    return *n;
  }

  Node& Node::getOrInsert(std::string_view key){
    if(isNull()) setMapping();
    auto *m = std::get_if<Mapping>(&m_value);
    if(!m) throw TypeError("operator[](key): node is not a mapping");
    for(auto &kv : *m) if(kv.first == key) return kv.second;
    m->emplace_back(key, Node{});
    return m->back().second;
  }

  const Node::Mapping& Node::mapping() const {
    const auto *m = std::get_if<Mapping>(&m_value);
    if(!m) throw TypeError("mapping(): node is not a mapping");
    return *m;
  }
  Node::Mapping& Node::mapping(){
    auto *m = std::get_if<Mapping>(&m_value);
    if(!m) throw TypeError("mapping(): node is not a mapping");
    return *m;
  }
  const Node::Sequence& Node::sequence() const {
    const auto *s = std::get_if<Sequence>(&m_value);
    if(!s) throw TypeError("sequence(): node is not a sequence");
    return *s;
  }
  Node::Sequence& Node::sequence(){
    auto *s = std::get_if<Sequence>(&m_value);
    if(!s) throw TypeError("sequence(): node is not a sequence");
    return *s;
  }

  void Node::setScalar(std::string_view sv, ScalarStyle style){
    ScalarData sd;
    sd.sv    = sv;
    sd.style = style;
    m_value = std::move(sd);
  }

  void Node::setOwnedScalar(std::string s, ScalarStyle style){
    ScalarData sd;
    sd.style = style;
    // An empty owned string would collide with the "view-mode" sentinel,
    // so represent explicit empty as the empty view instead.
    if(!s.empty()) sd.owned = std::move(s);
    m_value = std::move(sd);
  }

  // Assignment operators — all owning, always safe.
  Node& Node::operator=(const char *s){         setOwnedScalar(std::string(s ? s : "")); return *this; }
  Node& Node::operator=(const std::string &s){  setOwnedScalar(s);               return *this; }
  Node& Node::operator=(std::string &&s){       setOwnedScalar(std::move(s));    return *this; }
  Node& Node::operator=(std::string_view sv){   setOwnedScalar(std::string(sv)); return *this; }
  Node& Node::operator=(bool b){                setOwnedScalar(b ? "true" : "false"); return *this; }

  // ---------------------------------------------------------------------
  // Document
  // ---------------------------------------------------------------------

  Document Document::empty(){
    return Document{};  // default-initialized: null root, empty source
  }

  Document Document::view(std::string_view src){
    Document d;
    d.m_view = src;
    detail::parseInto(src, d);
    return d;
  }

  Document Document::own(std::string src){
    Document d;
    d.m_source = std::make_unique<std::string>(std::move(src));
    d.m_view   = *d.m_source;  // view into stable heap buffer
    detail::parseInto(d.m_view, d);
    return d;
  }

  Document Document::file(const std::string &path){
    std::ifstream f(path, std::ios::binary);
    if(!f) throw ICLException("yaml::Document::file: could not open '" + path + "'");
    std::ostringstream oss; oss << f.rdbuf();
    return Document::own(oss.str());
  }

  std::string Document::emit(EmitOptions opts) const {
    return detail::emit(m_root, opts);
  }

  void Document::emitToFile(const std::string &path, EmitOptions opts) const {
    std::ofstream f(path);
    if(!f) throw ICLException("yaml::Document::emitToFile: could not open '" + path + "'");
    f << detail::emit(m_root, opts);
  }

  std::string_view Document::intern(std::string s){
    m_arena.emplace_back(std::move(s));
    return m_arena.back();
  }

}  // namespace icl::utils::yaml
