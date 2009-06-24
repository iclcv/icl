#ifndef ICL_XML_NODE_IDX_H
#define ICL_XML_NODE_IDX_H

#include <string>

namespace icl{
  
  /// Simple utility structure, that can be used to reference certain subnodes \ingroup XML
  /** The XMLNodeIdx structure can be passed to the XMLNode's []-operator to 
      reference the k'th subnode with given tag-name. However this is not even
      close to the capabilities of XPath, this might be quiet useful sometimes.
      
      A special global utility function icl::xmlidx create XMLNodeIdx instances:
      @see xmlidx
      
      For a convenient access to first and last subnodes, a special type is
      can be handled internally as well. Simple use the macros 
      LAST_CHILD and FIRST_CHILD as indices for the []-operator of XMLNode's
  */
  struct XMLNodeIdx{
    
    /// internal special type
    enum Special {FIRST, LAST, NOTHING};
    
    /// special constructor 
    XMLNodeIdx(Special s):m_special(s){}
    
    /// constructor with given tag name and index
    XMLNodeIdx(const std::string &tag, unsigned int idx):m_idx(idx),m_tag(tag),m_special(NOTHING){}

    /// node index
    int m_idx;

    /// node tag
    std::string m_tag;
    
    /// node special flag
    Special m_special;
    
  };
  
  /// simply create an XMLNodeIndex with this global function \ingroup XML
  inline XMLNodeIdx xmlidx(const std::string &tag, unsigned int idx){
    return XMLNodeIdx(tag,idx);
  }

  /// \ingroup XML
#define LAST_CHILD XMLNodeIdx(XMLNodeIdx::LAST)

  /// \ingroup XML
#define FIRST_CHILD XMLNodeIdx(XMLNodeIdx::FIRST)

}

#endif
