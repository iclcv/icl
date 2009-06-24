#include <iclXMLNodeFilter.h>
#include <iclXMLNode.h>

namespace icl{

  bool XMLNodeFilterByType::operator()(const XMLNode &node) const{
    return node.getType() & nodeTypes;
  }

  bool XMLNodeFilterByTag::operator()(const XMLNode &node) const{
    return node.isTagNode() ? (node.getTag()==tag) : false;
  }

  bool XMLNodeFilterByPathSubstring::operator()(const XMLNode &node) const{
    return node.getPath(delim).find(pathPat) != std::string::npos;
  }
  
  bool XMLNodeFilterByAttrib::operator()(const XMLNode &node) const{
    return node.hasAttribute(attrib,value);
  }



  XMLNodeFilterCombination::XMLNodeFilterCombination(XMLNodeFilterCombination::Type t, 
                                                     XMLNodeFilter *a, 
                                                     XMLNodeFilter *b):t(t),a(a),b(b){}

  XMLNodeFilterCombination::XMLNodeFilterCombination(const XMLNodeFilterCombination &other){
    t = other.t;
    a = other.a ? other.a->deepCopy() : 0;
    b = other.b ? other.b->deepCopy() : 0;
  }
  
  XMLNodeFilterCombination &XMLNodeFilterCombination::operator=(const XMLNodeFilterCombination &other){
    ICL_DELETE(a);
    ICL_DELETE(b);
    t = other.t;
    a = other.a ? other.a->deepCopy() : 0;
    b = other.b ? other.b->deepCopy() : 0;
    return *this;
  }

  XMLNodeFilterCombination::~XMLNodeFilterCombination(){
    ICL_DELETE(a);
    ICL_DELETE(b);
  }
    
  bool XMLNodeFilterCombination::operator()(const XMLNode &node) const{
    switch(t){
      case AND_OP: return (a?a->operator()(node):true) &&  (b?b->operator()(node):true);
      case OR_OP: return (a?a->operator()(node):true) ||  (b?b->operator()(node):true);
      case XOR_OP: return (a?a->operator()(node):true) ^  (b?b->operator()(node):true);
    }
    return true;
  }
  
  XMLNodeFilter *XMLNodeFilterCombination::deepCopy() const{
    return new XMLNodeFilterCombination(t,a?a->deepCopy():0, b?b->deepCopy():0);
  }
  

  
}
