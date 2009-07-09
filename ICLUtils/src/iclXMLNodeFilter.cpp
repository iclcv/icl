#include <iclXMLNodeFilter.h>
#include <iclXMLNode.h>
#include <iclStringUtils.h>
namespace icl{

  bool XMLNodeFilterByType::operator()(const XMLNode &node) const{
    return node.getType() & nodeTypes;
  }

  bool XMLNodeFilterByTag::operator()(const XMLNode &node) const{
    return node.isNode() ? (node.getTag()==tag) : false;
  }

  XMLNodeFilterByPathRegex::XMLNodeFilterByPathRegex(const std::string &regex, const std::string &delim) 
    throw (InvalidRegularExpressionException) :
    regex(regex),delim(delim){
    match(" ",regex); // to test if this regex works
  }
  
  
  bool XMLNodeFilterByPathSubstring::operator()(const XMLNode &node) const{
    return node.getPath(delim).find(pathPat) != std::string::npos;
  }

  bool XMLNodeFilterByPathRegex::operator()(const XMLNode &node) const{
    return match(node.getPath(delim),regex);
  }
  
  bool XMLNodeFilterByAttrib::operator()(const XMLNode &node) const{
    return node.hasAttribute(attrib,value);
  }
  bool XMLNodeFilterByLevel::operator()(const XMLNode &node) const{
    return node.getLevel()<=maxLevel;
  }

  bool XMLNodeFilterByFirstChildNodeType::operator()(const XMLNode &node) const{
    if(!node.hasChildren()) return false;
    return node.getFirstChildNode().getType() & types;
  }

  bool XMLNodeFilterByHasAnyChildOfType::operator()(const XMLNode &node) const{
    if(!node.hasChildren()) return false;
    for(XMLNode::const_node_iterator it = node.begin(); it != node.end(); ++it){
      if( (*it)->getType() & types ) return true;
    }
    return false;
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
