// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/Xml.h>

#include <icl/utils/detail/xml/XPath.h>
#include <icl/utils/detail/xml/XmlEmitter.h>
#include <icl/utils/detail/xml/XmlParser.h>

#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace icl::utils::xml {

  // ---------------------------------------------------------------------
  // Exception ctors
  // ---------------------------------------------------------------------

  ParseError::ParseError(std::size_t line, std::size_t col, const std::string &what)
    : ICLException("xml::ParseError[" + std::to_string(line) + ":" +
                   std::to_string(col) + "] " + what),
      m_line(line), m_col(col) {}

  // ---------------------------------------------------------------------
  // ElementRange — linked-list iteration with optional name filter.
  // ---------------------------------------------------------------------

  namespace {
    inline bool elementMatches(const detail::ElementNode *n, std::string_view filter){
      return filter.empty() || n->name == filter;
    }
    inline detail::ElementNode* advanceToMatch(detail::ElementNode *n,
                                               std::string_view filter){
      while(n && !elementMatches(n, filter)) n = n->nextSibling;
      return n;
    }
  }

  Element ElementRange::Iterator::operator*() const noexcept {
    return Element(m_n, m_doc);
  }
  ElementRange::Iterator& ElementRange::Iterator::operator++() noexcept {
    if(m_n) m_n = advanceToMatch(m_n->nextSibling, m_filter);
    return *this;
  }
  ElementRange::Iterator ElementRange::begin() const noexcept {
    return Iterator(advanceToMatch(m_first, m_filter), m_doc, m_filter);
  }

  // ---------------------------------------------------------------------
  // Attribute
  // ---------------------------------------------------------------------

  std::string_view Attribute::name() const noexcept {
    return m_node ? m_node->name : std::string_view{};
  }

  std::string_view Attribute::valueRaw() const noexcept {
    return m_node ? m_node->valueRaw : std::string_view{};
  }

  std::string_view Attribute::value() const {
    if(!m_node) return {};
    if(m_node->valueDecodedResolved) return m_node->valueDecodedCache;
    // Fast path: no entities — decoded view IS the raw view.
    if(m_node->valueRaw.find('&') == std::string_view::npos){
      m_node->valueDecodedCache    = m_node->valueRaw;
      m_node->valueDecodedResolved = true;
      return m_node->valueDecodedCache;
    }
    std::string decoded;
    if(!detail::decodeEntities(m_node->valueRaw, decoded)){
      // Malformed entity — return raw; const accessor can't raise.
      m_node->valueDecodedCache    = m_node->valueRaw;
      m_node->valueDecodedResolved = true;
      return m_node->valueDecodedCache;
    }
    auto *ncDoc = const_cast<Document*>(m_doc);
    m_node->valueDecodedCache    = ncDoc->intern(std::move(decoded));
    m_node->valueDecodedResolved = true;
    return m_node->valueDecodedCache;
  }

  int Attribute::asInt(int fb) const noexcept {
    if(!m_node) return fb;
    try {
      std::string_view v = value();
      if(v.empty()) return fb;
      std::string tmp(v);
      char *end = nullptr;
      errno = 0;
      long r = std::strtol(tmp.c_str(), &end, 10);
      if(errno != 0 || end == tmp.c_str()) return fb;
      return static_cast<int>(r);
    } catch(...){ return fb; }
  }

  float Attribute::asFloat(float fb) const noexcept {
    if(!m_node) return fb;
    try {
      std::string_view v = value();
      if(v.empty()) return fb;
      std::string tmp(v);
      char *end = nullptr;
      errno = 0;
      float r = std::strtof(tmp.c_str(), &end);
      if(errno != 0 || end == tmp.c_str()) return fb;
      return r;
    } catch(...){ return fb; }
  }

  double Attribute::asDouble(double fb) const noexcept {
    if(!m_node) return fb;
    try {
      std::string_view v = value();
      if(v.empty()) return fb;
      std::string tmp(v);
      char *end = nullptr;
      errno = 0;
      double r = std::strtod(tmp.c_str(), &end);
      if(errno != 0 || end == tmp.c_str()) return fb;
      return r;
    } catch(...){ return fb; }
  }

  bool Attribute::asBool(bool fb) const noexcept {
    if(!m_node) return fb;
    try {
      std::string_view v = value();
      if(v == "true"  || v == "1" || v == "yes" || v == "on")  return true;
      if(v == "false" || v == "0" || v == "no"  || v == "off") return false;
      return fb;
    } catch(...){ return fb; }
  }

  // ---------------------------------------------------------------------
  // Element
  // ---------------------------------------------------------------------

  std::string_view Element::name() const noexcept {
    return m_node ? m_node->name : std::string_view{};
  }

  Element Element::firstChild() const noexcept {
    return (m_node && m_node->firstChild) ? Element(m_node->firstChild, m_doc) : Element{};
  }

  Element Element::nextSibling() const noexcept {
    return (m_node && m_node->nextSibling) ? Element(m_node->nextSibling, m_doc) : Element{};
  }

  Element Element::child(std::string_view n) const noexcept {
    if(!m_node) return {};
    for(auto *c = m_node->firstChild; c; c = c->nextSibling){
      if(c->name == n) return Element(c, m_doc);
    }
    return {};
  }

  Element Element::nextSibling(std::string_view n) const noexcept {
    if(!m_node) return {};
    for(auto *s = m_node->nextSibling; s; s = s->nextSibling){
      if(s->name == n) return Element(s, m_doc);
    }
    return {};
  }

  ElementRange Element::children() const noexcept {
    return ElementRange(m_node ? m_node->firstChild : nullptr, m_doc, {});
  }
  ElementRange Element::children(std::string_view n) const noexcept {
    return ElementRange(m_node ? m_node->firstChild : nullptr, m_doc, n);
  }

  Attribute Element::attribute(std::string_view n) const noexcept {
    if(!m_node) return {};
    for(auto *a = m_node->firstAttribute; a; a = a->next){
      if(a->name == n) return Attribute(a, m_doc);
    }
    return {};
  }

  std::size_t Element::attributeCount() const noexcept {
    if(!m_node) return 0;
    std::size_t n = 0;
    for(auto *a = m_node->firstAttribute; a; a = a->next) ++n;
    return n;
  }

  Attribute Element::attributeAt(std::size_t i) const noexcept {
    if(!m_node) return {};
    std::size_t k = 0;
    for(auto *a = m_node->firstAttribute; a; a = a->next, ++k){
      if(k == i) return Attribute(a, m_doc);
    }
    return {};
  }

  std::string_view Element::text() const noexcept {
    return m_node ? m_node->text : std::string_view{};
  }

  std::string_view Element::textDecoded() const {
    if(!m_node) return {};
    if(m_node->textDecodedResolved) return m_node->textDecodedCache;
    if(m_node->text.find('&') == std::string_view::npos){
      m_node->textDecodedCache    = m_node->text;
      m_node->textDecodedResolved = true;
      return m_node->textDecodedCache;
    }
    std::string decoded;
    if(!detail::decodeEntities(m_node->text, decoded)){
      m_node->textDecodedCache    = m_node->text;
      m_node->textDecodedResolved = true;
      return m_node->textDecodedCache;
    }
    auto *ncDoc = const_cast<Document*>(m_doc);
    m_node->textDecodedCache    = ncDoc->intern(std::move(decoded));
    m_node->textDecodedResolved = true;
    return m_node->textDecodedCache;
  }

  // ---------------------------------------------------------------------
  // Element — mutation
  // ---------------------------------------------------------------------

  Element Element::appendChild(std::string_view n){
    if(!m_node) return {};
    auto *ncDoc = const_cast<Document*>(m_doc);
    detail::ElementNode *child = ncDoc->allocElement();
    child->name   = ncDoc->intern(std::string(n));
    child->parent = m_node;
    if(!m_node->firstChild){
      m_node->firstChild = child;
      m_node->lastChild  = child;
    } else {
      m_node->lastChild->nextSibling = child;
      m_node->lastChild              = child;
    }
    return Element(child, m_doc);
  }

  void Element::setAttribute(std::string_view n, std::string_view v){
    if(!m_node) return;
    auto *ncDoc = const_cast<Document*>(m_doc);
    std::string_view internedValue = ncDoc->intern(std::string(v));
    // Update in place if the attribute already exists.
    for(auto *a = m_node->firstAttribute; a; a = a->next){
      if(a->name == n){
        a->valueRaw              = internedValue;
        a->valueDecodedResolved  = false;
        a->valueDecodedCache     = {};
        return;
      }
    }
    // Append a fresh node.
    detail::AttributeNode *a = ncDoc->allocAttribute();
    a->name     = ncDoc->intern(std::string(n));
    a->valueRaw = internedValue;
    if(!m_node->firstAttribute){
      m_node->firstAttribute = a;
      m_node->lastAttribute  = a;
    } else {
      m_node->lastAttribute->next = a;
      m_node->lastAttribute       = a;
    }
  }

  void Element::setText(std::string_view v){
    if(!m_node) return;
    auto *ncDoc = const_cast<Document*>(m_doc);
    m_node->text                = ncDoc->intern(std::string(v));
    m_node->textDecodedResolved = false;
    m_node->textDecodedCache    = {};
  }

  // ---------------------------------------------------------------------
  // Element — XPath
  // ---------------------------------------------------------------------

  NodeSet Element::selectAll(std::string_view xpath) const {
    if(!m_node) return NodeSet{};
    auto path = detail::parseXPath(xpath);
    return NodeSet(detail::evaluateXPath(path, *this));
  }

  Element Element::selectOne(std::string_view xpath) const {
    if(!m_node) return {};
    auto path = detail::parseXPath(xpath);
    auto v = detail::evaluateXPath(path, *this);
    return v.empty() ? Element{} : v.front();
  }

  Attribute Element::selectAttr(std::string_view xpath) const {
    if(!m_node) return {};
    auto path = detail::parseXPath(xpath);
    return detail::evaluateXPathAttr(path, *this);
  }

  // ---------------------------------------------------------------------
  // Document
  // ---------------------------------------------------------------------

  Document::Document() = default;
  Document::~Document() = default;
  Document::Document(Document&&) noexcept = default;
  Document& Document::operator=(Document&&) noexcept = default;

  Document Document::empty(){
    return Document{};
  }

  Document Document::parse(std::string_view src){
    Document d;
    d.m_view = src;
    detail::parseInto(src, d);
    return d;
  }

  Document Document::parseOwned(std::string src){
    Document d;
    d.m_source = std::make_unique<std::string>(std::move(src));
    d.m_view   = *d.m_source;
    detail::parseInto(d.m_view, d);
    return d;
  }

  Document Document::parseFile(const std::string &path){
    std::ifstream f(path, std::ios::binary);
    if(!f) throw ICLException("xml::Document::parseFile: could not open '" + path + "'");
    std::ostringstream oss; oss << f.rdbuf();
    return Document::parseOwned(oss.str());
  }

  Element Document::root() noexcept            { return Element(m_root, this); }
  Element Document::root() const noexcept      { return Element(m_root, this); }
  bool    Document::hasRoot() const noexcept   { return m_root != nullptr; }

  Element Document::setRoot(std::string_view name){
    if(m_root) throw ICLException("xml::Document::setRoot: root already set");
    detail::ElementNode *e = allocElement();
    e->name = intern(std::string(name));
    m_root  = e;
    return Element(e, this);
  }

  std::string Document::emit(EmitOptions opts) const {
    return detail::emit(m_root, opts);
  }

  void Document::emitToFile(const std::string &path, EmitOptions opts) const {
    std::ofstream f(path);
    if(!f) throw ICLException("xml::Document::emitToFile: could not open '" + path + "'");
    f << detail::emit(m_root, opts);
  }

  std::string_view Document::intern(std::string s){
    m_stringArena.emplace_back(std::move(s));
    return m_stringArena.back();
  }

  detail::ElementNode* Document::allocElement(){
    return m_nodeArena.alloc<detail::ElementNode>();
  }

  detail::AttributeNode* Document::allocAttribute(){
    return m_nodeArena.alloc<detail::AttributeNode>();
  }

  void Document::setRootNode(detail::ElementNode *root) noexcept {
    m_root = root;
  }

}  // namespace icl::utils::xml
