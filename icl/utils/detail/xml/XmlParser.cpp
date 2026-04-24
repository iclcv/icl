// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/detail/xml/XmlParser.h>

#include <icl/utils/detail/simd/SSETypes.h>   // ICL_HAVE_SSE2 + intrinsics

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>

namespace icl::utils::xml::detail {

  // ---------------------------------------------------------------------
  // Entity decoder — shared between parser and (Attribute/Element)::value()
  // accessors.  Public (within detail::) so the accessors can cross the
  // TU boundary.
  // ---------------------------------------------------------------------

  bool decodeEntities(std::string_view raw, std::string &out){
    out.clear();
    out.reserve(raw.size());
    for(std::size_t i = 0; i < raw.size(); ){
      char c = raw[i];
      if(c != '&'){
        out.push_back(c);
        ++i;
        continue;
      }
      // Find terminating ';'
      std::size_t semi = raw.find(';', i + 1);
      if(semi == std::string_view::npos) return false;
      std::string_view body = raw.substr(i + 1, semi - i - 1);
      if(body.empty()) return false;
      if(body == "amp")       out.push_back('&');
      else if(body == "lt")   out.push_back('<');
      else if(body == "gt")   out.push_back('>');
      else if(body == "quot") out.push_back('"');
      else if(body == "apos") out.push_back('\'');
      else if(body.size() >= 2 && body[0] == '#'){
        unsigned long cp = 0;
        if(body[1] == 'x' || body[1] == 'X'){
          if(body.size() < 3) return false;
          for(std::size_t k = 2; k < body.size(); ++k){
            char d = body[k];
            unsigned v;
            if(d >= '0' && d <= '9') v = d - '0';
            else if(d >= 'a' && d <= 'f') v = d - 'a' + 10;
            else if(d >= 'A' && d <= 'F') v = d - 'A' + 10;
            else return false;
            cp = (cp << 4) | v;
            if(cp > 0x10FFFF) return false;
          }
        } else {
          for(std::size_t k = 1; k < body.size(); ++k){
            char d = body[k];
            if(d < '0' || d > '9') return false;
            cp = cp * 10 + (d - '0');
            if(cp > 0x10FFFF) return false;
          }
        }
        // Encode as UTF-8.
        if(cp < 0x80){
          out.push_back(static_cast<char>(cp));
        } else if(cp < 0x800){
          out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
          out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if(cp < 0x10000){
          out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
          out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
          out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
          out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
          out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
          out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
          out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
      } else {
        return false;       // unknown named entity
      }
      i = semi + 1;
    }
    return true;
  }

  namespace {

    // ----------------------------------------------------------------
    // Parser — single-pass recursive descent.
    //
    // Design notes:
    //   * Element names + attribute names + raw content are zero-copy
    //     views into the source buffer.
    //   * Entity-decoded attribute values and element text are
    //     resolved lazily in the corresponding accessor.  The parser
    //     itself only keeps raw views; decoding happens in
    //     Attribute::value() / Element::textDecoded() (in Xml.cpp).
    //   * For elements with mixed content (text interleaved with
    //     children), all text chunks are concatenated into a single
    //     arena string and `text` views into it.  Interleaving order
    //     is NOT preserved.  See `Xml.h` for the rationale.
    // ----------------------------------------------------------------

    class Parser {
    public:
      Parser(std::string_view src, Document &doc) : m_src(src), m_doc(doc) {}

      void parse(){
        skipBom();
        skipProlog();
        skipMisc();
        if(eof()) error("empty document (no root element)");
        if(peek() != '<') error("expected root element");
        ElementNode *root = parseElement(nullptr);
        m_doc.setRootNode(root);
        skipMisc();
        if(!eof()) error("unexpected trailing content after root element");
      }

    private:
      std::string_view  m_src;
      Document&         m_doc;
      std::size_t       m_pos  = 0;
      std::size_t       m_line = 1;
      std::size_t       m_col  = 1;

      // --------------------------- cursor ---------------------------
      bool eof() const { return m_pos >= m_src.size(); }
      char peek(std::size_t off = 0) const {
        return (m_pos + off < m_src.size()) ? m_src[m_pos + off] : '\0';
      }
      std::string_view rest() const { return m_src.substr(m_pos); }
      char advance(){
        char c = m_src[m_pos++];
        if(c == '\n'){ ++m_line; m_col = 1; } else { ++m_col; }
        return c;
      }
      bool startsWith(std::string_view lit) const {
        return m_pos + lit.size() <= m_src.size() &&
               m_src.substr(m_pos, lit.size()) == lit;
      }
      bool consume(std::string_view lit){
        if(!startsWith(lit)) return false;
        for(std::size_t i = 0; i < lit.size(); ++i) advance();
        return true;
      }

      [[noreturn]] void error(const std::string &msg){
        throw ParseError(m_line, m_col, msg);
      }

      // --------------------------- utilities ------------------------
      void skipBom(){
        if(m_src.size() >= 3 &&
           static_cast<unsigned char>(m_src[0]) == 0xEF &&
           static_cast<unsigned char>(m_src[1]) == 0xBB &&
           static_cast<unsigned char>(m_src[2]) == 0xBF){
          m_pos = 3; m_col = 1;
        }
      }

      static bool isWs(char c){
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
      }
      static bool isNameStart(char c){
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
               c == '_' || c == ':';
      }
      static bool isNameChar(char c){
        return isNameStart(c) || (c >= '0' && c <= '9') ||
               c == '-' || c == '.';
      }

      void skipWs(){ while(!eof() && isWs(peek())) advance(); }

      // ----------------------------------------------------------
      // SIMD-accelerated content-text scan.
      //
      // Advances m_pos until it hits '<' or EOF, updating m_line /
      // m_col in bulk from the count+position of '\n' in the
      // skipped range.  Returns a view of the consumed run.
      //
      // This is the hottest loop in parseElement — it's invoked
      // once per text/whitespace run between children.  Falling
      // back to a per-byte loop would make this function ~5x
      // slower on the `parse_large` benchmark (measured).
      // ----------------------------------------------------------
      std::string_view scanTextUntilLT(){
        const char       *src       = m_src.data();
        const std::size_t n         = m_src.size();
        std::size_t       i         = m_pos;
        std::size_t       lineDelta = 0;
        // Offset (absolute in src) of the last '\n' seen during the
        // scan.  Used to recompute m_col after a multi-line bulk
        // advance.  SIZE_MAX means "no newline seen yet".
        std::size_t       lastNL    = static_cast<std::size_t>(-1);

#if defined(ICL_HAVE_SSE2)
        const __m128i vlt = _mm_set1_epi8('<');
        const __m128i vnl = _mm_set1_epi8('\n');
        while(i + 16 <= n){
          __m128i chunk = _mm_loadu_si128(
              reinterpret_cast<const __m128i *>(src + i));
          int mlt = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vlt));
          int mnl = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, vnl));
          if(mlt){
            int ltOff = __builtin_ctz(static_cast<unsigned>(mlt));
            int nlMask = mnl & ((1 << ltOff) - 1);
            if(nlMask){
              lineDelta += static_cast<std::size_t>(
                  __builtin_popcount(static_cast<unsigned>(nlMask)));
              int lastNlBit = 31 - __builtin_clz(
                  static_cast<unsigned>(nlMask));
              lastNL = i + static_cast<std::size_t>(lastNlBit);
            }
            i += static_cast<std::size_t>(ltOff);
            goto finish;
          }
          if(mnl){
            lineDelta += static_cast<std::size_t>(
                __builtin_popcount(static_cast<unsigned>(mnl)));
            int lastNlBit = 31 - __builtin_clz(
                static_cast<unsigned>(mnl));
            lastNL = i + static_cast<std::size_t>(lastNlBit);
          }
          i += 16;
        }
#endif
        // Scalar tail / non-SSE fallback.
        while(i < n && src[i] != '<'){
          if(src[i] == '\n'){ ++lineDelta; lastNL = i; }
          ++i;
        }
#if defined(ICL_HAVE_SSE2)
      finish:
#endif
        std::string_view run(src + m_pos, i - m_pos);
        // Update cursor state in bulk.
        if(lineDelta){
          m_line += lineDelta;
          // m_col = 1 + chars since last '\n'.  `i` is the position
          // *past* the '\n', so `i - lastNL` counts the '\n' itself
          // plus any chars after it; `m_col = i - lastNL` puts us
          // at column (chars-after-newline + 1), 1-based.
          m_col = i - lastNL;
        } else {
          m_col += (i - m_pos);
        }
        m_pos = i;
        return run;
      }


      // Skip comment, PI, or whitespace — does NOT enter an element.
      void skipMisc(){
        while(!eof()){
          if(isWs(peek())){ advance(); continue; }
          if(startsWith("<!--")){ skipComment(); continue; }
          if(startsWith("<?")){   skipPI();      continue; }
          break;
        }
      }

      void skipComment(){
        if(!consume("<!--")) error("expected '<!--'");
        while(!eof()){
          if(startsWith("-->")){ consume("-->"); return; }
          // XML spec disallows `--` inside a comment.  Be lenient.
          advance();
        }
        error("unterminated comment");
      }

      void skipPI(){
        if(!consume("<?")) error("expected '<?'");
        while(!eof()){
          if(startsWith("?>")){ consume("?>"); return; }
          advance();
        }
        error("unterminated processing instruction");
      }

      // Skip XML declaration / DOCTYPE from the prolog.  Both are
      // tolerated leniently — we don't validate their content.
      void skipProlog(){
        skipMisc();
        // DOCTYPE (if any)
        if(startsWith("<!DOCTYPE")){
          skipDoctype();
          skipMisc();
        }
      }

      void skipDoctype(){
        if(!consume("<!DOCTYPE")) error("expected '<!DOCTYPE'");
        int depth = 0;
        while(!eof()){
          char c = peek();
          if(c == '['){ ++depth; advance(); continue; }
          if(c == ']'){ if(depth > 0) --depth; advance(); continue; }
          if(c == '>' && depth == 0){ advance(); return; }
          advance();
        }
        error("unterminated DOCTYPE");
      }

      // --------------------------- names ----------------------------
      std::string_view parseName(){
        if(eof() || !isNameStart(peek())) error("expected XML name");
        std::size_t start = m_pos;
        while(!eof() && isNameChar(peek())) advance();
        return m_src.substr(start, m_pos - start);
      }

      // --------------------------- attributes -----------------------
      // Caller has already consumed whitespace; peek should be either
      // a name start (→ parse attr) or '/' / '>' (→ done).
      void parseAttributes(ElementNode *el){
        while(!eof()){
          skipWs();
          char c = peek();
          if(c == '/' || c == '>' || c == '\0') return;
          if(!isNameStart(c)) error("expected attribute name");
          std::string_view name = parseName();
          skipWs();
          if(peek() != '=') error("expected '=' after attribute name");
          advance();                        // consume '='
          skipWs();
          char q = peek();
          if(q != '"' && q != '\'') error("expected quoted attribute value");
          advance();                        // consume open quote
          std::size_t vstart = m_pos;
          while(!eof() && peek() != q){
            if(peek() == '<') error("'<' not allowed in attribute value");
            advance();
          }
          if(eof()) error("unterminated attribute value");
          std::string_view valueRaw = m_src.substr(vstart, m_pos - vstart);
          advance();                        // consume close quote
          AttributeNode *a = m_doc.allocAttribute();
          a->name     = name;
          a->valueRaw = valueRaw;
          if(!el->firstAttribute){
            el->firstAttribute = a;
            el->lastAttribute  = a;
          } else {
            el->lastAttribute->next = a;
            el->lastAttribute       = a;
          }
        }
      }

      // --------------------------- elements -------------------------
      // Caller is positioned at '<' of the start tag.
      ElementNode* parseElement(ElementNode *parent){
        if(!consume("<")) error("expected '<'");
        std::string_view name = parseName();
        ElementNode *el = m_doc.allocElement();
        el->name   = name;
        el->parent = parent;

        parseAttributes(el);
        skipWs();

        if(consume("/>")){
          return el;                        // self-closing
        }
        if(!consume(">")) error("expected '>' or '/>'");

        // Element content.  We collect text and child elements
        // separately; mixed content loses interleaving order.
        std::string textBuf;                // arena-bound later
        bool textFromSingleView = true;     // optimisation: if exactly
                                            // one contiguous text span
                                            // is seen with no children
                                            // interleaved, skip the
                                            // copy.
        std::string_view firstTextView;

        while(!eof()){
          if(startsWith("<![CDATA[")){
            consume("<![CDATA[");
            std::size_t cs = m_pos;
            while(!eof() && !startsWith("]]>")) advance();
            if(eof()) error("unterminated CDATA section");
            std::string_view chunk = m_src.substr(cs, m_pos - cs);
            consume("]]>");
            appendText(textBuf, firstTextView, textFromSingleView, chunk,
                       /*hasEntities=*/false, /*preserveRaw=*/true);
            continue;
          }
          if(startsWith("<!--")){
            skipComment();
            continue;
          }
          if(startsWith("<?")){
            skipPI();
            continue;
          }
          if(startsWith("</")){
            // End tag.
            consume("</");
            std::string_view endName = parseName();
            skipWs();
            if(!consume(">")) error("expected '>' after end tag name");
            if(endName != name){
              error("mismatched end tag (expected '</" + std::string(name) +
                    ">', got '</" + std::string(endName) + ">')");
            }
            // Finalise text: if we collected into textBuf, intern it.
            if(textFromSingleView){
              el->text = firstTextView;
            } else if(!textBuf.empty()){
              el->text = m_doc.intern(std::move(textBuf));
            }
            return el;
          }
          if(peek() == '<'){
            // (Any leading text run was already consumed below via
            // scanTextUntilLT; reaching '<' here means we're at a
            // child-element boundary.)
            // Nested element.
            ElementNode *child = parseElement(el);
            if(!el->firstChild){
              el->firstChild = child;
              el->lastChild  = child;
            } else {
              el->lastChild->nextSibling = child;
              el->lastChild              = child;
            }
            // If mixed content ensues, any further text chunks must
            // go into textBuf; lose the single-view optimisation.
            if(textFromSingleView && !firstTextView.empty()){
              textBuf.assign(firstTextView.data(), firstTextView.size());
              textFromSingleView = false;
            } else if(textFromSingleView){
              textFromSingleView = true;  // unchanged
            }
            continue;
          }
          // Character data — SIMD-accelerated scan to the next '<'.
          std::string_view chunk = scanTextUntilLT();
          // We don't try to detect entities here — `text` stores the
          // raw view verbatim and `textDecoded()` resolves on demand.
          appendText(textBuf, firstTextView, textFromSingleView, chunk,
                     /*hasEntities=*/chunk.find('&') != std::string_view::npos,
                     /*preserveRaw=*/false);
        }
        error("unterminated element");
      }

      // Helper: accumulate a text chunk into (firstTextView, textBuf)
      // honouring the "single contiguous view" optimisation.  Once
      // that optimisation is dropped, all further chunks flow into
      // `textBuf`.
      //
      // `hasEntities` / `preserveRaw` are currently unused (kept for
      // future use — e.g. if we want to distinguish CDATA from PCDATA
      // in the stored text).  Leaving the signature extensible.
      void appendText(std::string &textBuf,
                      std::string_view &firstTextView,
                      bool &textFromSingleView,
                      std::string_view chunk,
                      bool /*hasEntities*/,
                      bool /*preserveRaw*/){
        if(chunk.empty()) return;
        if(textFromSingleView && firstTextView.empty()){
          firstTextView = chunk;
          return;
        }
        if(textFromSingleView){
          // Second contiguous chunk — still contiguous if chunks are
          // adjacent in source (common: consecutive CDATA then PCDATA).
          if(chunk.data() == firstTextView.data() + firstTextView.size()){
            firstTextView = std::string_view(firstTextView.data(),
                                             firstTextView.size() + chunk.size());
            return;
          }
          // Non-adjacent — demote to arena buffer.
          textBuf.assign(firstTextView.data(), firstTextView.size());
          textBuf.append(chunk.data(), chunk.size());
          textFromSingleView = false;
          return;
        }
        textBuf.append(chunk.data(), chunk.size());
      }
    };

  }  // anonymous namespace

  void parseInto(std::string_view src, Document &doc){
    Parser p(src, doc);
    p.parse();
  }

}  // namespace icl::utils::xml::detail
