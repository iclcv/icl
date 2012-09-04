/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/TextTable.cpp                              **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLUtils/TextTable.h>
#include <ICLUtils/Macros.h>
namespace icl{
  namespace utils{
  
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
    
    static inline std::string save_substr_justified(const std::string &s, int a, int l, int ll){
      int sl = (int)s.length();
      if(a>=sl) return std::string(ll,' ');
      if(a+l > sl) l=sl-a;
      std::string tmp = s.substr(a,l);
      sl = tmp.length();
      if(sl == ll) return tmp;
      int difL = (ll-sl)/2;
      tmp = std::string(difL,' ') + tmp + std::string(ll-(difL+sl),' ');
      //    SHOW( (ll-tmp.length()));
      return tmp;
    }
    
    std::string TextTable::toString() const{
      std::vector<int> rowHeights(getSize().height,0);
      std::vector<int> columnWidths(getSize().width,0);
      
      for(int x=0;x<getSize().width;++x){
        for(int y=0;y<getSize().height;++y){
          const std::string &s = m_texts[x + getSize().width*y ];
          int l = (int)s.length();
          int cellWidth = iclMin(l,m_maxCellWidth);
          int cellHeight = iclMax(1,(int)ceil(float(l) / m_maxCellWidth));
          //        std::cout << "s: -"<< s << "- l:" << l << " cellH:" << cellHeight <<  " y:"<<  y << std::endl;
  
          if(cellWidth > columnWidths[x]) columnWidths[x] = cellWidth;
          if(cellHeight > rowHeights[y]) rowHeights[y] = cellHeight;
        }
      }
  #if 0
      std::cout << "rowHeights:";
      for(unsigned int i=0;i<rowHeights.size();++i) std::cout << rowHeights[i] << " ";
      std::cout << std::endl;
  
      std::cout << "columnWidths:";
      for(unsigned int i=0;i<columnWidths.size();++i) std::cout << columnWidths[i] << " ";
      std::cout << std::endl;
  #endif
  
      /*
       a  |  b   |    c    |
      ----+------+---------+
        e |  f   |     g   |
       e2 |      |         |
      ----+------+---------+
      */
      
      std::ostringstream stream;
      stream << '+';
      for(int x=0;x<getSize().width;++x){
        for(int i=0;i<columnWidths[x]+2;++i) stream << '-';
        stream << '+';
      }
      stream << std::endl;
  
      for(int y=0;y<getSize().height;++y){
        for(int h=0;h<rowHeights[y];++h){
          stream << '|' << ' ';
          for(int x=0;x<getSize().width;++x){
            const std::string &s = m_texts[x + getSize().width*y ];
            stream << save_substr_justified(s,h*m_maxCellWidth,m_maxCellWidth,columnWidths[x]);
            stream << ' ' << '|' << ' ';          
          }
          stream << std::endl;
        }
        stream << '+';
        for(int x=0;x<getSize().width;++x){
          for(int i=0;i<columnWidths[x]+2;++i) stream << '-';
          stream << '+';
        }
        stream << std::endl;
      }
      
      return stream.str();
    }
      
    
    void TextTable::clear(){
      std::fill(m_texts.begin(),m_texts.end(),"");
    }
  
  
  } // namespace utils
}
