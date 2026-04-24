// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/detail/xml/XPath.h>

#include <icl/utils/detail/xml/XmlParser.h>      // decodeEntities (for attr-eq)

#include <cctype>
#include <cstdlib>
#include <string>

namespace icl::utils::xml::detail {

  namespace {

    // =============================================================
    // Parser — combined lexer + recursive descent.
    //
    // Grammar (subset supported):
    //
    //   path     := '/'? '/'? step ('/' '/'? step)*
    //   step     := axis? nameTest predicate*
    //   axis     := '@' | 'self::'
    //               (full axis names are not needed in our subset —
    //                '@' introduces an attribute-axis step; 'self::'
    //                only appears inside predicates as 'self::name'.)
    //   nameTest := Name | '*'
    //   predicate:= '[' orExpr ']'
    //   orExpr   := andExpr ('or' andExpr)*
    //   andExpr  := primary ('and' primary)*
    //   primary  := '(' orExpr ')'
    //            | 'self::' Name                             (SelfIs)
    //            | '@' Name ('=' Literal)?                   (AttrEq / AttrExists)
    //            | Number                                    (Index, 1-based)
    //   Literal  := sqString | dqString
    //
    // Descendant step is introduced by a '//' separator — we model
    // it as a regular step with `axis = Descendant`.  For the first
    // step, a leading '//' flags the path absolute AND the first
    // step's axis descendant.
    // =============================================================

    class XPathParser {
    public:
      explicit XPathParser(std::string_view src) : m_src(src) {}

      Path parse(){
        Path out;
        skipWs();
        if(peek() == '/'){
          advance();
          out.absolute = true;
          if(peek() == '/'){
            advance();
            // first step is descendant
            out.steps.push_back(parseStep(Axis::Descendant));
          } else {
            out.steps.push_back(parseStep(Axis::Child));
          }
        } else {
          // relative
          out.steps.push_back(parseStep(Axis::Child));
        }
        while(true){
          skipWs();
          if(peek() != '/') break;
          advance();
          if(peek() == '/'){
            advance();
            out.steps.push_back(parseStep(Axis::Descendant));
          } else {
            out.steps.push_back(parseStep(Axis::Child));
          }
        }
        skipWs();
        if(!eof()) error("unexpected trailing input");
        return out;
      }

    private:
      std::string_view m_src;
      std::size_t      m_pos = 0;

      [[noreturn]] void error(const std::string &m){
        throw ParseError(1, m_pos + 1, "xpath: " + m);
      }
      bool eof() const { return m_pos >= m_src.size(); }
      char peek(std::size_t o = 0) const {
        return (m_pos + o < m_src.size()) ? m_src[m_pos + o] : '\0';
      }
      char advance(){ return m_src[m_pos++]; }
      void skipWs(){ while(!eof() && std::isspace(static_cast<unsigned char>(peek()))) advance(); }
      bool startsWith(std::string_view lit) const {
        return m_pos + lit.size() <= m_src.size() &&
               m_src.substr(m_pos, lit.size()) == lit;
      }
      bool consume(std::string_view lit){
        if(!startsWith(lit)) return false;
        m_pos += lit.size();
        return true;
      }
      static bool isNameStart(char c){
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == ':';
      }
      static bool isNameChar(char c){
        return isNameStart(c) || (c >= '0' && c <= '9') || c == '-' || c == '.';
      }

      std::string parseName(){
        if(!isNameStart(peek())) error("expected name");
        std::size_t s = m_pos;
        while(!eof() && isNameChar(peek())) advance();
        return std::string(m_src.substr(s, m_pos - s));
      }

      std::string parseStringLiteral(){
        char q = peek();
        if(q != '"' && q != '\'') error("expected string literal");
        advance();
        std::size_t s = m_pos;
        while(!eof() && peek() != q) advance();
        if(eof()) error("unterminated string literal");
        std::string raw(m_src.substr(s, m_pos - s));
        advance();                // consume closing quote
        // Decode entities so `@x='a &amp; b'` compares equal to the
        // attribute's decoded value (matches our documented semantics).
        std::string out;
        if(decodeEntities(raw, out)) return out;
        return raw;
      }

      Step parseStep(Axis axis){
        skipWs();
        Step step;
        step.axis = axis;
        // Attribute-axis terminal step: `@name`
        if(peek() == '@'){
          advance();
          step.axis      = Axis::Attribute;
          step.test.kind = NameTestKind::Name;
          step.test.name = parseName();
          parsePredicates(step);
          return step;
        }
        // Name test — either '*' or a Name.
        if(peek() == '*'){
          advance();
          step.test.kind = NameTestKind::Star;
        } else if(isNameStart(peek())){
          step.test.kind = NameTestKind::Name;
          step.test.name = parseName();
        } else {
          error("expected name, '*', or '@name'");
        }
        parsePredicates(step);
        return step;
      }

      void parsePredicates(Step &step){
        while(true){
          skipWs();
          if(peek() != '[') break;
          advance();
          step.predicates.push_back(parsePredicateExpr());
          skipWs();
          if(peek() != ']') error("expected ']' in predicate");
          advance();
        }
      }

      PredicatePtr parsePredicateExpr(){      // orExpr
        skipWs();
        PredicatePtr lhs = parseAndExpr();
        while(true){
          skipWs();
          if(!consumeKeyword("or")) break;
          PredicatePtr rhs = parseAndExpr();
          auto n = std::make_unique<Predicate>();
          n->node = BoolOr{std::move(lhs), std::move(rhs)};
          lhs = std::move(n);
        }
        return lhs;
      }

      PredicatePtr parseAndExpr(){
        skipWs();
        PredicatePtr lhs = parsePrimary();
        while(true){
          skipWs();
          if(!consumeKeyword("and")) break;
          PredicatePtr rhs = parsePrimary();
          auto n = std::make_unique<Predicate>();
          n->node = BoolAnd{std::move(lhs), std::move(rhs)};
          lhs = std::move(n);
        }
        return lhs;
      }

      // Consume the keyword `kw` if it's next, but only when it's
      // not part of a longer name (i.e. surrounded by non-name
      // chars).  Leaves position unchanged otherwise.
      bool consumeKeyword(std::string_view kw){
        if(!startsWith(kw)) return false;
        char after = peek(kw.size());
        if(after && isNameChar(after)) return false;
        m_pos += kw.size();
        return true;
      }

      PredicatePtr parsePrimary(){
        skipWs();
        if(peek() == '('){
          advance();
          auto inner = parsePredicateExpr();
          skipWs();
          if(peek() != ')') error("expected ')'");
          advance();
          return inner;
        }
        if(startsWith("self::")){
          m_pos += 6;
          std::string n = parseName();
          auto p = std::make_unique<Predicate>();
          p->node = SelfIs{std::move(n)};
          return p;
        }
        if(peek() == '@'){
          advance();
          std::string n = parseName();
          skipWs();
          if(peek() == '='){
            advance();
            skipWs();
            std::string v = parseStringLiteral();
            auto p = std::make_unique<Predicate>();
            p->node = AttrEq{std::move(n), std::move(v)};
            return p;
          }
          auto p = std::make_unique<Predicate>();
          p->node = AttrExists{std::move(n)};
          return p;
        }
        if(std::isdigit(static_cast<unsigned char>(peek()))){
          std::size_t s = m_pos;
          while(!eof() && std::isdigit(static_cast<unsigned char>(peek()))) advance();
          std::string num(m_src.substr(s, m_pos - s));
          auto p = std::make_unique<Predicate>();
          p->node = Index{static_cast<std::size_t>(std::stoul(num))};
          return p;
        }
        error("expected predicate primary (self::, @, '(' or number)");
      }
    };

    // =============================================================
    // Evaluator
    // =============================================================

    bool nameTestMatches(const NameTest &nt, std::string_view name){
      if(nt.kind == NameTestKind::Star) return true;
      return nt.name == name;
    }

    // Predicate application.  `pos` is the 1-based position of
    // `el` within its parent's filtered-by-axis set (per XPath
    // semantics).  `totalSiblings` is included for completeness but
    // our predicate subset doesn't use it beyond the index check.
    bool evalPredicate(const Predicate &p, Element el, std::size_t pos);

    bool evalBool(const Predicate &p, Element el, std::size_t pos){
      return evalPredicate(p, el, pos);
    }

    bool evalPredicate(const Predicate &p, Element el, std::size_t pos){
      return std::visit([&](auto const &node) -> bool {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, SelfIs>){
          return el.name() == node.name;
        } else if constexpr (std::is_same_v<T, AttrEq>){
          auto a = el.attribute(node.name);
          if(!a) return false;
          return a.value() == node.value;
        } else if constexpr (std::is_same_v<T, AttrExists>){
          return (bool)el.attribute(node.name);
        } else if constexpr (std::is_same_v<T, Index>){
          return pos == node.pos;
        } else if constexpr (std::is_same_v<T, BoolOr>){
          return evalBool(*node.lhs, el, pos) || evalBool(*node.rhs, el, pos);
        } else if constexpr (std::is_same_v<T, BoolAnd>){
          return evalBool(*node.lhs, el, pos) && evalBool(*node.rhs, el, pos);
        } else {
          return false;
        }
      }, p.node);
    }

    // Descendant-or-self traversal in document order, excluding
    // the starting node.  (Used for `//` steps.)
    void collectDescendants(Element start, std::vector<Element> &out){
      for(auto c : start.children()){
        out.push_back(c);
        collectDescendants(c, out);
      }
    }

    // Expand one step over a set of context nodes.
    std::vector<Element> expandStep(const Step &step,
                                     const std::vector<Element> &ctx){
      std::vector<Element> out;
      for(auto &node : ctx){
        std::vector<Element> stage;
        switch(step.axis){
          case Axis::Child:
            for(auto c : node.children()){
              if(nameTestMatches(step.test, c.name())) stage.push_back(c);
            }
            break;
          case Axis::Descendant: {
            std::vector<Element> all;
            collectDescendants(node, all);
            for(auto &c : all){
              if(nameTestMatches(step.test, c.name())) stage.push_back(c);
            }
            break;
          }
          case Axis::Self:
            if(nameTestMatches(step.test, node.name())) stage.push_back(node);
            break;
          case Axis::Attribute:
            // Element-typed step does not expand attributes into
            // the Element set — the final-attribute case is handled
            // at the top level in `evaluateXPathAttr`.
            break;
        }
        // Apply predicates with 1-based positional indexing over
        // `stage`.
        for(std::size_t i = 0; i < stage.size(); ++i){
          Element el = stage[i];
          bool keep = true;
          for(const auto &p : step.predicates){
            if(!evalPredicate(*p, el, i + 1)){ keep = false; break; }
          }
          if(keep) out.push_back(el);
        }
      }
      return out;
    }

  }  // anonymous namespace

  Path parseXPath(std::string_view src){
    XPathParser p(src);
    return p.parse();
  }

  namespace {
    // Rooted context = absolute-path starting node (doc root).
    Element rootOf(Element start){
      auto root = start;
      while(root){
        auto p = root.rawNode() ? root.rawNode()->parent : nullptr;
        if(!p) break;
        root = Element(p, root.document());
      }
      return root;
    }

    // Evaluate a slice of the step list over a context.  Returns the
    // element node-set after applying steps [begin, end).
    std::vector<Element> evalSteps(const Step *begin, const Step *end,
                                    std::vector<Element> ctx){
      for(const Step *s = begin; s != end; ++s){
        if(s->axis == Axis::Attribute){
          // Attribute-axis step encountered inside an element-valued
          // evaluation — can't carry attributes forward in the
          // element set.  Returns empty to signal "no match" for
          // selectAll; the attribute-terminal case is caught by
          // evaluateXPathAttr before reaching here.
          return {};
        }
        ctx = expandStep(*s, ctx);
        if(ctx.empty()) break;
      }
      return ctx;
    }
  }

  namespace {
    // Consume the first step of an absolute path.  XPath semantics:
    // `/` is the document node, whose only element child is the
    // document root.  So `/name` matches the root iff the name
    // matches; `//name` matches root and any descendant by name.
    // Returns the (filtered, predicated) context after the first
    // step, and advances `*outRest` to point at the remaining steps.
    std::vector<Element> consumeAbsoluteFirstStep(const Path &path,
                                                   Element start,
                                                   const Step **outRest,
                                                   const Step **outEnd){
      Element r = rootOf(start);
      const Step &first = path.steps.front();
      std::vector<Element> stage;
      if(first.axis == Axis::Descendant){
        stage.push_back(r);
        collectDescendants(r, stage);
      } else {
        stage.push_back(r);
      }
      // Filter by name test.
      std::vector<Element> filtered;
      for(auto &e : stage){
        if(nameTestMatches(first.test, e.name())) filtered.push_back(e);
      }
      // Apply predicates 1-based.
      std::vector<Element> out;
      for(std::size_t i = 0; i < filtered.size(); ++i){
        Element el = filtered[i]; bool keep = true;
        for(const auto &p : first.predicates){
          if(!evalPredicate(*p, el, i + 1)){ keep = false; break; }
        }
        if(keep) out.push_back(el);
      }
      *outRest = path.steps.data() + 1;
      *outEnd  = path.steps.data() + path.steps.size();
      return out;
    }
  }

  std::vector<Element> evaluateXPath(const Path &path, Element start){
    if(!start || path.steps.empty()) return {};
    if(path.absolute){
      const Step *rest = nullptr, *end = nullptr;
      auto ctx = consumeAbsoluteFirstStep(path, start, &rest, &end);
      return evalSteps(rest, end, std::move(ctx));
    }
    std::vector<Element> ctx;
    ctx.push_back(start);
    return evalSteps(path.steps.data(),
                     path.steps.data() + path.steps.size(),
                     std::move(ctx));
  }

  Attribute evaluateXPathAttr(const Path &path, Element start){
    if(path.steps.empty() || path.steps.back().axis != Axis::Attribute){
      return {};
    }
    std::vector<Element> elements;
    if(path.absolute){
      const Step *rest = nullptr, *end = nullptr;
      auto ctx = consumeAbsoluteFirstStep(path, start, &rest, &end);
      // Evaluate everything up to but not including the final
      // (attribute) step.
      elements = evalSteps(rest, end - 1, std::move(ctx));
    } else {
      std::vector<Element> ctx;
      ctx.push_back(start);
      elements = evalSteps(
          path.steps.data(),
          path.steps.data() + path.steps.size() - 1,
          std::move(ctx));
    }
    const auto &attrStep = path.steps.back();
    for(auto &el : elements){
      Attribute a = el.attribute(attrStep.test.name);
      if(a) return a;
    }
    return {};
  }

}  // namespace icl::utils::xml::detail
