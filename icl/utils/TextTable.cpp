// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/TextTable.h>
#include <icl/utils/Macros.h>
#include <vector>
#include <sstream>
#include <string>
namespace icl::utils {
  void TextTable::ensureSize(int width, int height){
    width = iclMax(getSize().width,width);     //2
    height = iclMax(getSize().height,height);  //1
    Size oldSize = getSize();                  //1x1

    //    SHOW(oldSize);

    bool wider = width > oldSize.width;        // true
    bool higher = height > oldSize.height;     // false
    if(!wider && !higher) return;

    TextTable save = *this;
    m_texts.resize(width*height);              // 2

    std::fill(m_texts.begin(),m_texts.end(),"");

    m_size = Size(width,height);               // 2x1


    for(int x=0;x<oldSize.width;++x){
      for(int y=0;y<oldSize.height;++y){
        m_texts[x+width*y] = save.m_texts[x+oldSize.width*y];
        //        (*this)(x,y) = save(x,y);
      }
    }
  }

  /// Wrap a cell into lines no wider than `width`, breaking preferentially
  /// at separator characters (space, '|', ',', '/') and keeping the
  /// separator attached to the preceding chunk.  Tokens longer than the
  /// width are hard-cut as a fallback.
  static std::vector<std::string> wrap_cell(const std::string &s, int width){
    if(width < 1) width = 1;
    auto isBreak = [](char c){ return c==' '||c=='|'||c==','||c=='/'; };
    std::vector<std::string> lines;
    std::string cur;
    size_t i = 0, n = s.size();
    while(i < n){
      // next token = run of non-break chars + the following run of breaks
      size_t j = i;
      while(j < n && !isBreak(s[j])) ++j;
      while(j < n &&  isBreak(s[j])) ++j;
      std::string token = s.substr(i, j - i);
      i = j;
      // hard-cut a token that can never fit
      while(static_cast<int>(token.size()) > width){
        if(!cur.empty()){ lines.push_back(cur); cur.clear(); }
        lines.push_back(token.substr(0, width));
        token = token.substr(width);
      }
      if(!cur.empty() && static_cast<int>(cur.size() + token.size()) > width){
        lines.push_back(cur);
        cur.clear();
      }
      cur += token;
    }
    if(!cur.empty() || lines.empty()) lines.push_back(cur);
    // drop trailing spaces so centering stays symmetric (keep '|' etc.)
    for(std::string &l : lines){
      size_t e = l.find_last_not_of(' ');
      l = (e == std::string::npos) ? std::string() : l.substr(0, e + 1);
    }
    return lines;
  }

  static inline std::string justify_left(const std::string &s, int ll){
    int sl = static_cast<int>(s.size());
    if(sl >= ll) return s;
    return s + std::string(ll - sl,' ');
  }

  std::string TextTable::toString() const{
    const int W = getSize().width, H = getSize().height;
    std::vector<std::vector<std::string>> wrapped(W * H);
    std::vector<int> rowHeights(H, 1);
    std::vector<int> columnWidths(W, 0);

    for(int x=0;x<W;++x){
      for(int y=0;y<H;++y){
        std::vector<std::string> lines = wrap_cell(m_texts[x + W*y], m_maxCellWidth);
        int w = 0;
        for(const std::string &l : lines) w = iclMax(w, static_cast<int>(l.size()));
        if(w > columnWidths[x]) columnWidths[x] = w;
        if(static_cast<int>(lines.size()) > rowHeights[y]) rowHeights[y] = static_cast<int>(lines.size());
        wrapped[x + W*y] = std::move(lines);
      }
    }

    auto hline = [&](std::ostringstream &stream){
      stream << '+';
      for(int x=0;x<W;++x){
        for(int i=0;i<columnWidths[x]+2;++i) stream << '-';
        stream << '+';
      }
      stream << std::endl;
    };

    // Horizontal rules only frame the table and separate the header row
    // (row 0) from the body — no rule between every data row (less clutter).
    std::ostringstream stream;
    hline(stream);                       // top border
    for(int y=0;y<H;++y){
      for(int h=0;h<rowHeights[y];++h){
        stream << '|' << ' ';
        for(int x=0;x<W;++x){
          const std::vector<std::string> &lines = wrapped[x + W*y];
          const std::string &line = (h < static_cast<int>(lines.size())) ? lines[h] : std::string();
          stream << justify_left(line, columnWidths[x]);
          stream << ' ' << '|' << ' ';
        }
        stream << std::endl;
      }
      if(y == 0 && H > 1) hline(stream); // separator under the header only
    }
    hline(stream);                       // bottom border

    return stream.str();
  }


  void TextTable::clear(){
    std::fill(m_texts.begin(),m_texts.end(),"");
  }


  } // namespace icl::utils