/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/XMLNodeFilter.cpp                         **
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

#include <ICLUtils/XMLNodeFilter.h>
#include <ICLUtils/XMLNode.h>
#include <ICLUtils/StringUtils.h>
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
