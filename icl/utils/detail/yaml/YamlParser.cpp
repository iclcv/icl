// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/detail/yaml/YamlParser.h>

#include <cctype>
#include <cstdio>
#include <optional>
#include <string>
#include <string_view>

namespace icl::utils::yaml::detail {
  namespace {

    // ----------------------------------------------------------------
    // Parser — single-pass recursive descent.
    //
    // Design notes:
    //   * Plain and quoted scalars are single-line in this subset.
    //     Multi-line quoted / folded plain scalars are deferred.  Block
    //     scalars (|, >) ARE supported and are the intended multi-line
    //     mechanism.
    //   * Tabs in indentation are a ParseError.
    //   * Mapping key duplication is a ParseError (YAML 1.2 requires it).
    //   * Flow-style collections may span lines; whitespace (incl. \n)
    //     between flow tokens is ignored.
    //   * YAML 1.1 bool aliases (yes/no/on/off) are NOT honored — they
    //     resolve as plain strings per YAML 1.2 core schema.
    // ----------------------------------------------------------------

    class Parser {
    public:
      Parser(std::string_view src, Document &doc) : m_src(src), m_doc(doc) {}

      Node parse(){
        skipBomAndBlanks();
        // optional leading --- (directives end marker)
        if(match("---")){
          if(!eof() && !isLineEnd(peek())){
            // "--- " or "---\n" both fine; anything else is a hard error
            if(peek() != ' ' && peek() != '\t'){
              error("unexpected characters after '---'");
            }
          }
          skipInlineRest();
          skipBomAndBlanks();
        }
        if(eof()) return Node{};  // null root

        const int rootIndent = static_cast<int>(m_col) - 1;
        Node root = parseNode(rootIndent, /*inFlow=*/false, /*ownerIndent=*/-1);

        // Trailing blanks + optional `...` end marker
        skipBomAndBlanks();
        if(match("...")){ skipInlineRest(); skipBomAndBlanks(); }
        if(!eof()){
          error("unexpected trailing content");
        }
        return root;
      }

    private:
      std::string_view m_src;
      Document&        m_doc;
      std::size_t      m_pos  = 0;
      std::size_t      m_line = 1;
      std::size_t      m_col  = 1;  // 1-based for user-facing diagnostics

      // --------------------------- cursor ---------------------------

      bool eof() const { return m_pos >= m_src.size(); }
      char peek(std::size_t off = 0) const {
        return (m_pos + off < m_src.size()) ? m_src[m_pos + off] : '\0';
      }

      char advance(){
        char c = m_src[m_pos++];
        if(c == '\n'){ ++m_line; m_col = 1; }
        else         { ++m_col; }
        return c;
      }

      bool match(std::string_view lit){
        if(m_pos + lit.size() > m_src.size()) return false;
        if(m_src.substr(m_pos, lit.size()) != lit) return false;
        for(std::size_t i = 0; i < lit.size(); ++i) advance();
        return true;
      }

      static bool isLineEnd(char c){ return c == '\n' || c == '\0'; }

      // Skip a UTF-8 BOM if present at the current (always start) position.
      void skipBom(){
        if(m_src.size() >= 3 &&
           static_cast<unsigned char>(m_src[0]) == 0xEF &&
           static_cast<unsigned char>(m_src[1]) == 0xBB &&
           static_cast<unsigned char>(m_src[2]) == 0xBF &&
           m_pos == 0){
          m_pos = 3; m_col = 1;
        }
      }

      // Consume spaces+tabs up to a newline/content, then an optional
      // `# comment` to end of line.  Does NOT cross newlines.
      void skipInlineWsAndComment(){
        while(!eof() && (peek() == ' ' || peek() == '\t')) advance();
        if(!eof() && peek() == '#'){
          while(!eof() && peek() != '\n') advance();
        }
      }

      // Consume the rest of the current line (including trailing ws /
      // comment) up to and including the line-terminating \n (or EOF).
      void skipInlineRest(){
        skipInlineWsAndComment();
        if(!eof() && peek() == '\n') advance();
      }

      // Skip BOM, blank lines, and comment-only lines.  Leaves m_pos at
      // the first non-blank, non-comment character, with m_col reflecting
      // its column.  Errors if a content line starts with a tab in its
      // leading whitespace — YAML 1.2 forbids tabs in indentation.
      void skipBomAndBlanks(){
        if(m_pos == 0) skipBom();
        while(!eof()){
          bool sawTab = false;
          while(!eof() && (peek() == ' ' || peek() == '\t')){
            if(peek() == '\t') sawTab = true;
            advance();
          }
          if(eof()) return;
          if(peek() == '\n'){
            advance(); continue;     // blank line — tab tolerated
          }
          if(peek() == '#'){
            while(!eof() && peek() != '\n') advance();
            if(!eof()) advance();
            continue;                // comment-only line — tab tolerated
          }
          if(sawTab) error("tab character in indentation");
          return;
        }
      }

      [[noreturn]] void error(const std::string &msg){
        throw ParseError(m_line, m_col, msg);
      }

      // --------------------------- nodes ----------------------------

      // Parse a single YAML node whose first content character is at
      // m_pos.  `indent` is the column of that first character (0-based
      // for block decisions).  `inFlow` toggles flow-context scalar
      // termination rules.  `ownerIndent` is the indent of the containing
      // block structure (mapping/sequence), used as the "must-exceed"
      // threshold for block-scalar content; -1 means no containing block.
      Node parseNode(int indent, bool inFlow, int ownerIndent){
        // Optional YAML 1.2 core-schema tag prefix (`!!str` / `!!int` /
        // `!!bool` / `!!float` / `!!null`).  Applied to the scalar that
        // follows; silently ignored on non-scalar results.
        std::optional<ScalarKind> explicitTag = parseTagPrefix();

        Node result = parseNodeInner(indent, inFlow, ownerIndent);

        if(explicitTag && result.isScalar()){
          auto &sd = std::get<Node::ScalarData>(result.value());
          sd.explicitTag = explicitTag;
        }
        return result;
      }

      // Try to consume a `!!name ` prefix at current position.  Returns
      // the resolved ScalarKind on success (and leaves the cursor past
      // the trailing inline whitespace).  Returns nullopt and leaves the
      // cursor untouched otherwise.  Throws on unknown tag name.
      std::optional<ScalarKind> parseTagPrefix(){
        if(m_pos + 2 > m_src.size()) return std::nullopt;
        if(peek() != '!' || peek(1) != '!') return std::nullopt;
        const std::size_t save_pos = m_pos;
        const std::size_t save_line = m_line;
        const std::size_t save_col  = m_col;
        advance(); advance();  // consume `!!`
        const std::size_t nameStart = m_pos;
        while(!eof()){
          char h = peek();
          if((h >= 'a' && h <= 'z') || (h >= 'A' && h <= 'Z') ||
             (h >= '0' && h <= '9') || h == '_') advance();
          else break;
        }
        const std::string_view tagName = m_src.substr(nameStart, m_pos - nameStart);
        if(tagName.empty()){
          m_pos = save_pos; m_line = save_line; m_col = save_col;
          return std::nullopt;
        }
        // Must be followed by whitespace / EOL / EOF.
        if(!eof() && peek() != ' ' && peek() != '\t' && peek() != '\n'){
          m_pos = save_pos; m_line = save_line; m_col = save_col;
          return std::nullopt;
        }
        while(!eof() && (peek() == ' ' || peek() == '\t')) advance();

        if(tagName == "str")   return ScalarKind::String;
        if(tagName == "int")   return ScalarKind::Int;
        if(tagName == "float") return ScalarKind::Float;
        if(tagName == "bool")  return ScalarKind::Bool;
        if(tagName == "null")  return ScalarKind::Null;
        error("unknown tag: !!" + std::string(tagName));
      }

      // Original parseNode body, now parameterized by tag handling in
      // parseNode above.
      Node parseNodeInner(int indent, bool inFlow, int ownerIndent){
        if(eof()) return Node{};
        const char c = peek();

        if(c == '['){ return parseFlowSequence(); }
        if(c == '{'){ return parseFlowMapping();  }

        // Block scalar prefix — must use ownerIndent as the
        // "must-exceed" threshold, NOT the column of the indicator.
        if(!inFlow && (c == '|' || c == '>')){
          return parseBlockScalar(c, ownerIndent);
        }

        // Block sequence? `- ` (or `-` at EOL).
        if(!inFlow && c == '-' && (peek(1) == ' ' || peek(1) == '\t' || isLineEnd(peek(1)))){
          return parseBlockSequence(indent);
        }

        // Quoted scalar: ' or "
        if(c == '\'' || c == '"'){
          // Might be a key in a block mapping — peek past the close
          // quote and whitespace to see if `:` follows.
          std::size_t saved_pos = m_pos, saved_line = m_line, saved_col = m_col;
          ScalarStyle style{};
          std::string_view v = parseQuotedScalar(style);
          // In flow context, we never build a block mapping from here.
          if(!inFlow && peekColonAfterWs()){
            // Rewind and let parseBlockMapping handle the key; that
            // mirrors the plain-scalar path for symmetry.
            m_pos = saved_pos; m_line = saved_line; m_col = saved_col;
            (void)ownerIndent;
            return parseBlockMapping(indent);
          }
          Node n; n.setScalar(v, style);
          return n;
        }

        // Otherwise: plain scalar.  In block context, detect mapping by
        // `: ` on the same line; in flow context, leave detection to
        // parseFlowMapping's driver.
        if(!inFlow){
          // Peek forward for `:` on same line to decide mapping vs bare scalar.
          if(hasMappingColonOnCurrentLine()){
            return parseBlockMapping(indent);
          }
          ScalarStyle style = ScalarStyle::Plain;
          std::string_view v = parsePlainScalar(/*inFlow=*/false);
          Node n; n.setScalar(v, style);
          return n;
        }

        // flow-context plain scalar
        ScalarStyle style = ScalarStyle::Plain;
        std::string_view v = parsePlainScalar(/*inFlow=*/true);
        Node n; n.setScalar(v, style);
        return n;
      }

      // Scan forward on the current logical line (no newlines) to check
      // whether a `:` appears that would make this a mapping key.  Does
      // NOT mutate parser state.
      //
      // Quote handling only engages at key-start position: a `"` or `'`
      // mid-line (inside what would be a plain scalar's bytes, like the
      // `"` in `a!"#$...: safe`) is NOT a quoted-string delimiter.
      bool hasMappingColonOnCurrentLine() const {
        std::size_t p = m_pos;
        bool atKeyStart = true;
        while(p < m_src.size()){
          char c = m_src[p];
          if(c == '\n') return false;
          if(atKeyStart && (c == '\'' || c == '"')){
            char qc = c;
            ++p;
            while(p < m_src.size() && m_src[p] != qc){
              if(m_src[p] == '\n') return false;
              if(qc == '"' && m_src[p] == '\\' && p + 1 < m_src.size()){ p += 2; continue; }
              ++p;
            }
            if(p < m_src.size()) ++p;  // consume closing quote
            atKeyStart = false;
            continue;
          }
          atKeyStart = false;
          if(c == '#' && p > m_pos && (m_src[p-1] == ' ' || m_src[p-1] == '\t')) return false;
          if(c == ':'){
            char nxt = (p + 1 < m_src.size()) ? m_src[p+1] : '\n';
            if(nxt == ' ' || nxt == '\t' || nxt == '\n') return true;
            // `:` without following space is part of the scalar (e.g. "foo:bar")
          }
          ++p;
        }
        return false;
      }

      // Peek whether a `:` follows current m_pos after optional inline ws.
      bool peekColonAfterWs() const {
        std::size_t p = m_pos;
        while(p < m_src.size() && (m_src[p] == ' ' || m_src[p] == '\t')) ++p;
        if(p >= m_src.size()) return false;
        if(m_src[p] != ':') return false;
        char nxt = (p + 1 < m_src.size()) ? m_src[p+1] : '\n';
        return (nxt == ' ' || nxt == '\t' || nxt == '\n' || p + 1 == m_src.size());
      }

      // --------------------- block mapping --------------------------

      Node parseBlockMapping(int indent){
        Node n; n.setMapping();
        while(!eof()){
          // Key
          ScalarStyle keyStyle = ScalarStyle::Plain;
          std::string_view key;
          const char c0 = peek();
          if(c0 == '\'' || c0 == '"') key = parseQuotedScalar(keyStyle);
          else                        key = parsePlainScalar(/*inFlow=*/false);

          // Expect `:`
          skipInlineWs();
          if(eof() || peek() != ':') error("block mapping: expected ':' after key");
          advance();  // consume ':'
          // Optional space after `:`
          if(!eof() && (peek() == ' ' || peek() == '\t')){
            while(!eof() && (peek() == ' ' || peek() == '\t')) advance();
          }

          // Duplicate key check
          for(const auto &kv : n.mapping()){
            if(kv.first == key) error("duplicate mapping key");
          }

          Node value;
          if(eof() || isLineEnd(peek()) || (peek() == '#' && prevWasSpace())){
            // Value on next line (indented > indent) or null
            skipInlineRest();
            skipBomAndBlanks();
            if(eof()){
              value.setNull();
            } else {
              const int childIndent = static_cast<int>(m_col) - 1;
              if(childIndent > indent){
                value = parseNode(childIndent, /*inFlow=*/false, /*ownerIndent=*/indent);
                // After a flow or scalar child, the cursor may still be
                // mid-line; catch up to the next line-start.
                skipInlineWsAndComment();
                if(!eof() && peek() == '\n') advance();
                skipBomAndBlanks();
              } else if(childIndent == indent && peek() == '-' &&
                        (peek(1) == ' ' || peek(1) == '\t' || isLineEnd(peek(1)))){
                // YAML idiom: block sequence as mapping value at the
                // same indent as the key.  Disambiguated by the `-`
                // indicator.
                value = parseBlockSequence(indent);
                // parseBlockSequence already consumed trailing blanks.
              } else {
                value.setNull();  // empty value, sibling/parent level follows
              }
            }
          } else {
            // Inline value on same line — owner is this mapping.
            value = parseNode(static_cast<int>(m_col) - 1, /*inFlow=*/false, /*ownerIndent=*/indent);
            skipInlineWsAndComment();
            if(!eof() && peek() == '\n') advance();
            skipBomAndBlanks();
          }
          n.mapping().emplace_back(key, std::move(value));

          if(eof()) break;
          const int nextIndent = static_cast<int>(m_col) - 1;
          if(nextIndent < indent) break;
          if(nextIndent > indent) error("block mapping: unexpected indent increase");
          // same indent → next key
        }
        return n;
      }

      // --------------------- block sequence -------------------------

      Node parseBlockSequence(int indent){
        Node n; n.setSequence();
        while(!eof() && static_cast<int>(m_col) - 1 == indent && peek() == '-' &&
              (peek(1) == ' ' || peek(1) == '\t' || isLineEnd(peek(1)))){
          advance();  // consume '-'
          // Consume the mandatory space (or treat newline as empty item)
          if(!eof() && (peek() == ' ' || peek() == '\t')){
            while(!eof() && (peek() == ' ' || peek() == '\t')) advance();
          }
          Node item;
          if(eof() || isLineEnd(peek()) || (peek() == '#' && prevWasSpace())){
            // Empty item — value on next line indented more, or null.
            // `#` here is a comment that extends to EOL, item is null.
            skipInlineRest();
            skipBomAndBlanks();
            if(eof()){
              item.setNull();
            } else {
              const int childIndent = static_cast<int>(m_col) - 1;
              if(childIndent > indent){
                item = parseNode(childIndent, /*inFlow=*/false, /*ownerIndent=*/indent);
                // Catch up to next line-start if child was flow/scalar.
                skipInlineWsAndComment();
                if(!eof() && peek() == '\n') advance();
                skipBomAndBlanks();
              } else {
                item.setNull();
              }
            }
          } else {
            // Inline item — owner is this sequence.
            item = parseNode(static_cast<int>(m_col) - 1, /*inFlow=*/false, /*ownerIndent=*/indent);
            skipInlineWsAndComment();
            if(!eof() && peek() == '\n') advance();
            skipBomAndBlanks();
          }
          n.sequence().push_back(std::move(item));
          if(eof()) break;
          const int next = static_cast<int>(m_col) - 1;
          if(next < indent) break;
          if(next > indent) error("block sequence: unexpected indent increase");
        }
        return n;
      }

      // --------------------- flow collections -----------------------

      Node parseFlowSequence(){
        advance();  // '['
        Node n; n.setSequence();
        skipFlowWs();
        if(!eof() && peek() == ']'){ advance(); return n; }
        while(!eof()){
          Node item = parseNode(static_cast<int>(m_col) - 1, /*inFlow=*/true, /*ownerIndent=*/-1);
          n.sequence().push_back(std::move(item));
          skipFlowWs();
          if(eof()) error("flow sequence: unexpected EOF");
          if(peek() == ','){
            advance(); skipFlowWs();
            if(!eof() && peek() == ']'){ advance(); return n; }  // trailing comma
            continue;
          }
          if(peek() == ']'){ advance(); return n; }
          error("flow sequence: expected ',' or ']'");
        }
        error("flow sequence: unexpected EOF");
      }

      Node parseFlowMapping(){
        advance();  // '{'
        Node n; n.setMapping();
        skipFlowWs();
        if(!eof() && peek() == '}'){ advance(); return n; }
        while(!eof()){
          // Key
          ScalarStyle keyStyle = ScalarStyle::Plain;
          std::string_view key;
          char c0 = peek();
          if(c0 == '\'' || c0 == '"'){
            key = parseQuotedScalar(keyStyle);
          } else {
            key = parsePlainScalar(/*inFlow=*/true);
          }
          skipFlowWs();
          if(eof() || peek() != ':') error("flow mapping: expected ':' after key");
          advance();
          skipFlowWs();

          // Duplicate key check
          for(const auto &kv : n.mapping()){
            if(kv.first == key) error("duplicate mapping key");
          }

          Node value;
          if(!eof() && peek() != ',' && peek() != '}'){
            value = parseNode(static_cast<int>(m_col) - 1, /*inFlow=*/true, /*ownerIndent=*/-1);
          } // else null value
          n.mapping().emplace_back(key, std::move(value));
          skipFlowWs();
          if(eof()) error("flow mapping: unexpected EOF");
          if(peek() == ','){
            advance(); skipFlowWs();
            if(!eof() && peek() == '}'){ advance(); return n; }  // trailing comma
            continue;
          }
          if(peek() == '}'){ advance(); return n; }
          error("flow mapping: expected ',' or '}'");
        }
        error("flow mapping: unexpected EOF");
      }

      void skipFlowWs(){
        // Flow context ignores newlines between tokens.
        while(!eof()){
          char c = peek();
          if(c == ' ' || c == '\t'){ advance(); continue; }
          if(c == '\n'){ advance(); continue; }
          if(c == '#'){
            while(!eof() && peek() != '\n') advance();
            continue;
          }
          break;
        }
      }

      void skipInlineWs(){
        while(!eof() && (peek() == ' ' || peek() == '\t')) advance();
      }

      bool prevWasSpace() const {
        return m_pos > 0 && (m_src[m_pos - 1] == ' ' || m_src[m_pos - 1] == '\t');
      }

      // --------------------- scalars --------------------------------

      // Plain scalar: contiguous non-special chars on a single line.
      // Terminators (block): `: ` / `:$` / ` #` / newline.
      // Terminators (flow) : additionally `,`, `]`, `}`.
      std::string_view parsePlainScalar(bool inFlow){
        const std::size_t start = m_pos;
        bool prevSpace = false;
        while(!eof()){
          char c = peek();
          if(c == '\n') break;
          if(inFlow && (c == ',' || c == ']' || c == '}')) break;
          if(c == ':'){
            char nxt = peek(1);
            if(nxt == ' ' || nxt == '\t' || nxt == '\n' || m_pos + 1 == m_src.size() ||
               (inFlow && (nxt == ',' || nxt == ']' || nxt == '}'))){
              break;
            }
          }
          if(c == '#' && prevSpace) break;
          prevSpace = (c == ' ' || c == '\t');
          advance();
        }
        // Trim trailing whitespace from the scalar span.
        std::size_t end = m_pos;
        while(end > start && (m_src[end-1] == ' ' || m_src[end-1] == '\t')) --end;
        if(end == start) error("plain scalar: empty");
        return m_src.substr(start, end - start);
      }

      // Quoted scalar — single-line.  Returns a view into m_src when no
      // escapes are present; otherwise decodes into Document arena and
      // returns a view into the arena entry.
      std::string_view parseQuotedScalar(ScalarStyle &style){
        char quote = peek();
        advance();  // consume opening quote
        const std::size_t start = m_pos;
        bool hasEscape = false;

        if(quote == '\''){
          style = ScalarStyle::SingleQuoted;
          while(!eof()){
            char c = peek();
            if(c == '\n') error("single-quoted scalar: unterminated (newline)");
            if(c == '\''){
              if(peek(1) == '\''){
                hasEscape = true;  // `''` escape inside single-quoted
                advance(); advance();
                continue;
              }
              // closing quote
              std::string_view raw = m_src.substr(start, m_pos - start);
              advance();  // consume closing quote
              if(!hasEscape) return raw;
              std::string out;
              out.reserve(raw.size());
              for(std::size_t i = 0; i < raw.size(); ++i){
                if(raw[i] == '\'' && i + 1 < raw.size() && raw[i+1] == '\''){
                  out.push_back('\''); ++i;
                } else {
                  out.push_back(raw[i]);
                }
              }
              return m_doc.intern(std::move(out));
            }
            advance();
          }
          error("single-quoted scalar: unterminated (EOF)");
        }

        // double-quoted
        style = ScalarStyle::DoubleQuoted;
        while(!eof()){
          char c = peek();
          if(c == '\n') error("double-quoted scalar: unterminated (newline)");
          if(c == '"'){
            std::string_view raw = m_src.substr(start, m_pos - start);
            advance();
            if(!hasEscape) return raw;
            // decode escapes
            std::string out;
            out.reserve(raw.size());
            for(std::size_t i = 0; i < raw.size(); ++i){
              char r = raw[i];
              if(r != '\\'){ out.push_back(r); continue; }
              if(i + 1 >= raw.size()) error("double-quoted: trailing backslash");
              char e = raw[++i];
              switch(e){
                case 'n':  out.push_back('\n'); break;
                case 't':  out.push_back('\t'); break;
                case 'r':  out.push_back('\r'); break;
                case '0':  out.push_back('\0'); break;
                case 'a':  out.push_back('\a'); break;
                case 'b':  out.push_back('\b'); break;
                case 'f':  out.push_back('\f'); break;
                case 'v':  out.push_back('\v'); break;
                case '\\': out.push_back('\\'); break;
                case '"':  out.push_back('"');  break;
                case '/':  out.push_back('/');  break;
                case 'x': {
                  if(i + 2 >= raw.size()) error("double-quoted: bad \\x escape");
                  auto hex = [&](char h)->int{
                    if(h >= '0' && h <= '9') return h - '0';
                    if(h >= 'a' && h <= 'f') return h - 'a' + 10;
                    if(h >= 'A' && h <= 'F') return h - 'A' + 10;
                    error("double-quoted: bad hex digit");
                  };
                  int v = (hex(raw[i+1]) << 4) | hex(raw[i+2]);
                  out.push_back(static_cast<char>(v));
                  i += 2;
                  break;
                }
                case 'u': {
                  if(i + 4 >= raw.size()) error("double-quoted: bad \\u escape");
                  auto hex = [&](char h)->int{
                    if(h >= '0' && h <= '9') return h - '0';
                    if(h >= 'a' && h <= 'f') return h - 'a' + 10;
                    if(h >= 'A' && h <= 'F') return h - 'A' + 10;
                    error("double-quoted: bad hex digit");
                  };
                  int cp = 0;
                  for(int k = 1; k <= 4; ++k) cp = (cp << 4) | hex(raw[i+k]);
                  i += 4;
                  // minimal UTF-8 encoder
                  if(cp < 0x80){
                    out.push_back(static_cast<char>(cp));
                  } else if(cp < 0x800){
                    out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
                    out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                  } else {
                    out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
                    out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                    out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                  }
                  break;
                }
                default:
                  error("double-quoted: unknown escape");
              }
            }
            return m_doc.intern(std::move(out));
          }
          if(c == '\\'){ hasEscape = true; advance(); if(!eof()) advance(); continue; }
          advance();
        }
        error("double-quoted scalar: unterminated (EOF)");
      }

      // Block scalar (| or >) — newlines preserved (|) or folded (>).
      // Chomping:
      //   `-`  strip: trim all trailing newlines
      //   `+`  keep : preserve all trailing newlines
      //   (none) clip: preserve exactly one trailing newline
      // Numeric indent indicator is tolerated but ignored (auto-detected).
      Node parseBlockScalar(char kind, int parentIndent){
        advance();  // consume | or >
        enum class Chomp { Clip, Strip, Keep };
        Chomp chomp = Chomp::Clip;
        while(!eof() && peek() != '\n' && peek() != '\r' && peek() != '#'){
          char h = peek();
          if(h == '-'){ chomp = Chomp::Strip; advance(); }
          else if(h == '+'){ chomp = Chomp::Keep;  advance(); }
          else if(h >= '0' && h <= '9'){ advance(); }   // indent indicator — ignored
          else if(h == ' ' || h == '\t'){ advance(); }
          else break;
        }
        if(!eof() && peek() == '#'){
          while(!eof() && peek() != '\n') advance();
        }
        if(!eof() && peek() == '\n') advance();

        // Determine content indent = indent of first non-empty line.
        // Content indent must exceed parentIndent.
        std::size_t save_pos = m_pos, save_line = m_line, save_col = m_col;
        (void)save_pos; (void)save_line; (void)save_col;
        int contentIndent = -1;

        auto lineIndent = [&]()->int{
          int c = 0;
          while(!eof() && peek() == ' '){ advance(); ++c; }
          return c;
        };

        std::string body;
        while(!eof()){
          // Measure this line's indent
          std::size_t line_start_pos = m_pos;
          std::size_t line_start_col = m_col;
          int ind = 0;
          while(!eof() && peek() == ' '){ advance(); ++ind; }
          if(eof()){ break; }
          if(peek() == '\n'){
            // blank line → preserved as empty content line
            body.push_back('\n');
            advance();
            continue;
          }
          if(contentIndent < 0){
            if(ind <= parentIndent){
              // No content — block is empty, backtrack.
              m_pos = line_start_pos; m_col = line_start_col;
              break;
            }
            contentIndent = ind;
          } else if(ind < contentIndent){
            // End of block scalar — backtrack to line start (we've
            // consumed only spaces), then restore cursor to before them.
            m_pos = line_start_pos; m_col = line_start_col;
            break;
          }
          // Emit line content (everything past contentIndent's spaces)
          // plus the original leading extras.
          int extra = ind - contentIndent;
          while(extra-- > 0) body.push_back(' ');
          while(!eof() && peek() != '\n') body.push_back(advance());
          // Terminate line: literal keeps \n, folded converts \n→' ' for
          // non-empty-to-non-empty adjacency (simplified: always \n here,
          // fold-pass below merges).
          if(!eof() && peek() == '\n') advance();
          body.push_back('\n');
        }
        (void)lineIndent;

        // Apply chomping on the trailing newlines of `body`.
        if(chomp == Chomp::Strip){
          while(!body.empty() && body.back() == '\n') body.pop_back();
        } else if(chomp == Chomp::Clip){
          // Collapse trailing newlines to exactly one.
          while(body.size() >= 2 && body[body.size()-1] == '\n' && body[body.size()-2] == '\n'){
            body.pop_back();
          }
          if(body.size() == 1 && body[0] == '\n') body.clear();
        }
        // Chomp::Keep: leave all trailing newlines as-is.

        if(kind == '>'){
          // Folded: replace single internal \n with space; double \n stays
          // as one \n (paragraph break).
          std::string folded;
          folded.reserve(body.size());
          for(std::size_t i = 0; i < body.size(); ++i){
            if(body[i] == '\n'){
              // Count run
              std::size_t run_start = i;
              while(i < body.size() && body[i] == '\n') ++i;
              std::size_t run = i - run_start;
              if(run == 1){
                // Single newline between content → space (unless at end)
                if(i < body.size()) folded.push_back(' ');
              } else {
                // Two or more → emit (run-1) newlines
                for(std::size_t k = 1; k < run; ++k) folded.push_back('\n');
              }
              --i;  // outer loop increments
            } else {
              folded.push_back(body[i]);
            }
          }
          body = std::move(folded);
        }

        Node n;
        n.setScalar(m_doc.intern(std::move(body)),
                    kind == '|' ? ScalarStyle::Literal : ScalarStyle::Folded);
        return n;
      }
    };

  }  // anonymous

  void parseInto(std::string_view src, Document &doc){
    Parser p(src, doc);
    doc.m_root = p.parse();
  }

}  // namespace icl::utils::yaml::detail
