// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/detail/xml/XmlEmitter.h>

#include <string>
#include <string_view>

namespace icl::utils::xml::detail {

  namespace {

    // Escape `in` for attribute-value or text context.  Attribute
    // values escape additionally `"` (since we emit double-quoted
    // attributes); text content does not need to.
    void appendEscaped(std::string &out, std::string_view in, bool isAttr){
      for(char c : in){
        switch(c){
          case '&': out.append("&amp;"); break;
          case '<': out.append("&lt;"); break;
          case '>': out.append("&gt;"); break;
          case '"': if(isAttr) out.append("&quot;"); else out.push_back(c); break;
          default:  out.push_back(c); break;
        }
      }
    }

    void appendIndent(std::string &out, int depth, int step){
      if(step <= 0) return;
      out.append(static_cast<std::size_t>(depth * step), ' ');
    }

    // True when `text` is non-empty and contains at least one
    // non-whitespace byte — determines whether we prefer inline
    // emission vs. indented children-only layout.
    bool isNonWsText(std::string_view text){
      for(char c : text){
        if(c != ' ' && c != '\t' && c != '\n' && c != '\r') return true;
      }
      return false;
    }

    void emitElement(std::string &out,
                     const ElementNode *el,
                     int depth,
                     const EmitOptions &opts){
      appendIndent(out, depth, opts.indent);
      out.push_back('<');
      out.append(el->name.data(), el->name.size());
      for(const auto *a = el->firstAttribute; a; a = a->next){
        out.push_back(' ');
        out.append(a->name.data(), a->name.size());
        out.append("=\"");
        appendEscaped(out, a->valueRaw, /*isAttr=*/true);
        out.push_back('"');
      }
      const bool hasChildren = el->firstChild != nullptr;
      const bool hasText     = isNonWsText(el->text);
      if(!hasChildren && !hasText){
        if(opts.selfCloseEmpty){
          out.append("/>");
        } else {
          out.append("></");
          out.append(el->name.data(), el->name.size());
          out.push_back('>');
        }
        if(opts.indent > 0) out.push_back('\n');
        return;
      }
      out.push_back('>');
      // Leaf element with only text: one-line form.
      if(!hasChildren && hasText){
        appendEscaped(out, el->text, /*isAttr=*/false);
        out.append("</");
        out.append(el->name.data(), el->name.size());
        out.push_back('>');
        if(opts.indent > 0) out.push_back('\n');
        return;
      }
      // Container with children (and possibly text — text comes first,
      // since we dropped interleaving at parse time).
      if(opts.indent > 0) out.push_back('\n');
      if(hasText){
        appendIndent(out, depth + 1, opts.indent);
        appendEscaped(out, el->text, /*isAttr=*/false);
        if(opts.indent > 0) out.push_back('\n');
      }
      for(const ElementNode *c = el->firstChild; c; c = c->nextSibling){
        emitElement(out, c, depth + 1, opts);
      }
      appendIndent(out, depth, opts.indent);
      out.append("</");
      out.append(el->name.data(), el->name.size());
      out.push_back('>');
      if(opts.indent > 0) out.push_back('\n');
    }

  }  // anonymous namespace

  std::string emit(const ElementNode *root, const EmitOptions &opts){
    std::string out;
    if(opts.prologue){
      out.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
      if(opts.indent > 0) out.push_back('\n');
    }
    if(root) emitElement(out, root, 0, opts);
    return out;
  }

}  // namespace icl::utils::xml::detail
