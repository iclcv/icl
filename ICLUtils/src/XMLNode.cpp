#include <ICLUtils/XMLNode.h>
#include <ICLUtils/XMLDocument.h>
#include <sstream>
#include <list>
#include <iterator>

namespace icl{
  
  /*
      static std::string strip(const std::string &s){
      if(!s.length()) return "";
      std::string::size_type f = s.find_first_not_of(" \n\t");
      std::string::size_type l = s.find_last_not_of(" \n\t");
      return s.substr(f,l-f);
      }
  */
  static inline const std::string &strip(const std::string &s) { return s; }
  
  XMLNode::XMLNode():m_parent(0),m_document(0),m_type(XMLNode::UNDEFINED),m_attribs(new XMLAttMap){}
  
  XMLNode::XMLNode(XMLNode *parent, XMLDocument *doc):
    m_parent(parent),m_document(doc),m_type(XMLNode::UNDEFINED),m_attribs(new XMLAttMap){}


  bool XMLNode::isText() const{
    return m_type == TEXT;
    //    return m_text != "";
  }
  bool XMLNode::isComment() const{
    return m_type == COMMENT;
    //    return m_comment != "";
  }
  bool XMLNode::isNode() const{
    return m_type == NODE;
  }
  bool XMLNode::isNull() const{
    return !m_document;
  }
  bool XMLNode::isRoot() const{
    return !m_parent;
  }
  bool XMLNode::hasAttribute(const std::string &name, const std::string &value) const{
    if(!m_attribs || !isNode()) return false;
    const_attrib_iterator it = m_attribs->find(name);
    if(it != m_attribs->end()){
      if(value != "") return it->second == value;
      else return true;
    }
    return false;
  }

  const std::string & XMLNode::getTag() const throw (InvalidNodeTypeException){
    if(!isNode()) throw InvalidNodeTypeException("only NODE nodes have tags");
    return m_content;
  }
  
  const std::string &XMLNode::getComment() const throw (InvalidNodeTypeException){
    if(!isNode()) throw InvalidNodeTypeException("only COMMENT nodes have comments :-)");
    return m_content;
  }
  

  const std::string & XMLNode::getText() const throw (InvalidNodeTypeException){
    if(!isText()) throw InvalidNodeTypeException("only TEXT can ge asked for getText()");
    return m_content;
  }
  
  XMLNode::const_attrib_iterator  XMLNode::attrib_begin() const throw (InvalidNodeTypeException){
    return const_cast<XMLNode*>(this)->attrib_begin();
  }
  XMLNode::const_attrib_iterator  XMLNode::attrib_end() const throw (InvalidNodeTypeException){
    return const_cast<XMLNode*>(this)->attrib_end();
  }

  XMLNode::attrib_iterator  XMLNode::attrib_begin() throw (InvalidNodeTypeException){
    if(!isNode()) throw InvalidNodeTypeException("only NODES may have attributes");
    return m_attribs->begin();
  }
  XMLNode::attrib_iterator  XMLNode::attrib_end() throw (InvalidNodeTypeException){
    if(!isNode()) throw InvalidNodeTypeException("only NODES may have attributes");
    return m_attribs->end();
  }

  
  int XMLNode::getLevel() const{
    return m_parent ? (m_parent->getLevel() + 1) : 0;
  }
  XMLDocument *XMLNode::getDocument(){
    return m_document;
  }

  const XMLDocument *XMLNode::getDocument() const{
    return m_document;
  }
  
  XMLNode *XMLNode::getParent(){
    return m_parent;
  }

  const XMLNode  *XMLNode::getParent() const{
    return m_parent;
  }



  static std::string get_indent(int level){
    std::ostringstream str;
    for(int i=0;i<level;++i){
      str << "  ";
    }
    return str.str();
  }
  static std::string get_attrib_text(const XMLNode *node){
    if(!node->hasAttributes()) return "";
    std::ostringstream str;

    str << ' ';
    for(XMLNode::const_attrib_iterator it = node->attrib_begin();
        it != node->attrib_end();){
      str << it->first << '=' << '"' << it->second << '"';
      ++it;
      if(it != node->attrib_end()) str << ' ';
    }
    return str.str();
  }
  
  void XMLNode::serialize(std::ostream &s, int level,bool lastNodeWasText, bool recursive) const{
    switch(m_type){
      case NODE:
        if(hasChildren()){
          if(recursive){
            if(!lastNodeWasText && level) s << std::endl;
            s << get_indent(level) << "<" << m_content << get_attrib_text(this)
              << ">";
            lastNodeWasText = false;
            for(std::vector<XMLNodePtr>::const_iterator it = m_subnodes.begin(); it != m_subnodes.end(); ++it){
              (*it)->serialize(s,level+1,lastNodeWasText,recursive);
              lastNodeWasText = (*it)->isText();
            }
            if(!lastNodeWasText) s << std::endl << get_indent(level);
            s << "</" << m_content << ">";
          }else{
            s << std::endl << get_indent(level) << "<" << m_content << get_attrib_text(this)
              << ">@" << m_subnodes.size() << " subnodes@" << 
            get_indent(level) << "</" << m_content << ">";
          }
        }else{
          if(!lastNodeWasText && level) s << std::endl;
          s << get_indent(level) << "<" << m_content << get_attrib_text(this)
            << "/>";
        }
        break;
      case COMMENT:
        if(!lastNodeWasText && level) s << std::endl;
        s << get_indent(level) << "<!--" << m_content << "-->";
        break;
      case TEXT:
        s << m_content;
        break;
      default:
        if(!lastNodeWasText && level) s << std::endl;
        s << get_indent(level) << "<@undefined type-ID=\"" << (int)m_type << "\"/>";
        break;
    }
  }

  XMLNode::const_node_iterator XMLNode::begin() const throw (InvalidNodeTypeException){
    if(!isNode()) throw InvalidNodeTypeException("only CONTAINER nodes have subnodes");
    return m_subnodes.begin();
  }
  XMLNode::const_node_iterator XMLNode::end() const throw (InvalidNodeTypeException){
    if(!isNode()) throw InvalidNodeTypeException("only CONTAINER nodes have subnodes");
    return m_subnodes.end();
  }
  
  XMLNode::node_iterator XMLNode::begin() throw (InvalidNodeTypeException){
    if(!isNode()) throw InvalidNodeTypeException("only CONTAINER nodes have subnodes");
    return m_subnodes.begin();
  }
  XMLNode::node_iterator XMLNode::end() throw (InvalidNodeTypeException){
    if(!isNode()) throw InvalidNodeTypeException("only CONTAINER nodes have subnodes");
    return m_subnodes.end();
  }

  XMLNode::Type XMLNode::getType() const{
    return m_type;
  }
  int XMLNode::getChildCount() const{
    return (int)m_subnodes.size();
  }
  bool XMLNode::hasChildren() const{
    return (bool)getChildCount();
  }
  bool XMLNode::hasAttributes() const{
    return m_attribs && m_attribs->size();
  }
  int XMLNode::getAttributeCount() const{
    return m_attribs ? m_attribs->size() : 0;
  }

  
  void XMLNode::addNode(XMLNodePtr child,int index) throw (InvalidNodeTypeException,DoubledReferenceException){
    switch(getType()){
      case TEXT: case COMMENT: case UNDEFINED:
        throw InvalidNodeTypeException("childs can only be added to SINGLE- or to CONTAINER nodes (parent:" + str(*this) + ")");
      case NODE:
        if(child->m_parent && child->m_parent != this){
          throw DoubledReferenceException("tryed to add a child that already has a parent node");
        }
        if(child->m_document && child->m_document != m_document){
          throw DoubledReferenceException("tryed to add a child that is already part of another document");
        }
        child->m_parent = this;
        child->m_document = m_document;
        if(index >= 0 && (int)m_subnodes.size() < index){
          m_subnodes.insert(begin()+index, child);
        }else{
          m_subnodes.push_back(child);
        }
        break;
      default:
        throw std::logic_error(str("in function:") + __FUNCTION__ +  "(XMLNode::Type value ALL is just a shortcut and"
                                 " cannot be used as a node type");
    }
  }
  void XMLNode::addText(const std::string &text, int idx) throw (InvalidNodeTypeException){
    XMLNodePtr n = new XMLNode;
    n->m_type = TEXT;
    n->m_content = strip(text);
    addNode(n,idx);
  }
  void XMLNode::addNode(const std::string &tag,int idx)  throw (InvalidNodeTypeException){
    XMLNodePtr n = new XMLNode;
    n->m_type = NODE;
    n->m_content = tag;
    addNode(n,idx);
  }
  void XMLNode::addNodeWithTextContent(const std::string &tag,const std::string &text, int index)
    throw (InvalidNodeTypeException){
    XMLNodePtr n = new XMLNode;
    n->m_type = NODE;
    n->m_content = tag;
    addNode(n,index);
    n->addText(text);
  }

  void XMLNode::addComment(const std::string &comment,int idx)  throw (InvalidNodeTypeException){
    XMLNodePtr n = new XMLNode;
    n->m_type = COMMENT;
    n->m_content = comment;
    addNode(n,idx);
  }

  void XMLNode::addXML(const std::string &xml,int idx)  throw (InvalidNodeTypeException,ParseException){
    XMLDocument doc(xml);
    XMLNodePtr cpy = doc.getRootNode()->deepCopy(this,m_document);
    addNode(cpy,idx);
  }

  void XMLNode::addXML(std::istream &stream,int idx)  throw (InvalidNodeTypeException,ParseException){

    XMLDocument doc(stream);
    addNode(doc.getRootNode()->deepCopy(this,m_document),idx);
  }

  
  XMLNodePtr XMLNode::removeChildNode(unsigned int index) 
    throw (ChildNotFoundException,InvalidNodeTypeException){
    switch(getType()){
      case NODE:{
        if(index <= m_subnodes.size()){
          throw ChildNotFoundException("child index was too high (given index:" + str(index) + " sub node vector size:" + str(m_subnodes.size())+")");
        }
        XMLNodePtr n  = m_subnodes[index];
        m_subnodes.erase(begin()+index);
        n->m_parent = 0;
        n->m_document = 0;
        return n;
      }
      default:
        throw InvalidNodeTypeException("childs can only be removed from container nodes");
        return 0;
    }
  }


  std::vector<XMLNodePtr> XMLNode::removeChildNode(const std::string &tagName, bool all) 
    throw (ChildNotFoundException,InvalidNodeTypeException){
    switch(getType()){
      case NODE:{
        std::list<XMLNodePtr> hold,del;
        for(node_iterator it = begin();it != end() ;++it){
          if((all || !del.size()) && (*it)->isNode() && (*it)->getTag() == tagName){
            del.push_back(*it);
            (*it)->m_parent = 0;
            (*it)->m_document = 0;
          }else{
            hold.push_back(*it);
          }
        }
        if(!del.size()) throw ChildNotFoundException("unable to remove child "+ tagName+" (reason: tag not found)");
        m_subnodes = std::vector<XMLNodePtr>(hold.begin(),hold.end());
        return std::vector<XMLNodePtr>(del.begin(),del.end());
        break;
      }
      default:
        throw InvalidNodeTypeException("childs can only be removed from container nodes");
        return std::vector<XMLNodePtr>();
    }
    return std::vector<XMLNodePtr>();
  }
  
  std::vector<XMLNodePtr> XMLNode::removeAllComments(){
    switch(getType()){
      case COMMENT:{
        return std::vector<XMLNodePtr>(1,delSelf());
      }
      case NODE:{
        std::list<XMLNodePtr> hold,del;
        for(node_iterator it = begin();it != end() ;++it){
          if((*it)->isComment()){
            del.push_back(*it);
            (*it)->m_parent = 0;
            (*it)->m_document = 0;
          }else{
            if((*it)->isNode()){
              std::vector<XMLNodePtr> v = (*it)->removeAllComments();
              for(unsigned int i=0;i<v.size();++i){
                del.push_back(v[i]);
              }
            }
            hold.push_back(*it);
          }
        }
        m_subnodes = std::vector<XMLNodePtr>(hold.begin(),hold.end());
        return std::vector<XMLNodePtr>(del.begin(),del.end());
        break;
      }
      default:
        break;
    }
    return std::vector<XMLNodePtr>();
  }
  
  XMLNode &XMLNode::operator[](unsigned int index) throw (ChildNotFoundException,InvalidNodeTypeException){
    if(!isNode()) throw InvalidNodeTypeException(__FUNCTION__ + str(": is only allowd for NODE nodes"));
    if(index < m_subnodes.size()) return *m_subnodes[index];
    else throw ChildNotFoundException(__FUNCTION__ + str(": index too large"));
    return *this;
  }
  const XMLNode &XMLNode::operator[](unsigned int index) const throw (ChildNotFoundException,InvalidNodeTypeException){
    return const_cast<XMLNode*>(this)->operator[](index);
  }
  
  XMLNode &XMLNode::operator[](const std::string &tag) throw (ChildNotFoundException,InvalidNodeTypeException){
    if(!isNode()) throw InvalidNodeTypeException(__FUNCTION__ + str(": is only allowed for CONTAINER nodes"));
    for(node_iterator it = begin(); it != end(); ++it){
      if((*it)->isNode() && (*it)->getTag() == tag) return **it;
    }
    throw ChildNotFoundException(__FUNCTION__ + str(": unable to find node with tag name ") + tag);
    return *this;
  }
  const XMLNode &XMLNode::operator[](const std::string &tag) const
    throw (ChildNotFoundException,InvalidNodeTypeException){
    return const_cast<XMLNode*>(this)->operator[](tag);
  }

  XMLNode &XMLNode::operator[](const XMLNodeIdx &idx) throw (ChildNotFoundException,InvalidNodeTypeException){
    if(!isNode()) throw InvalidNodeTypeException(__FUNCTION__ + str(": is only allowed for NODE nodes"));
    if(idx.m_special == XMLNodeIdx::FIRST){
      return **begin();
    }else if(idx.m_special == XMLNodeIdx::LAST){
      return **(end()-1);
    }
    int idxNr = idx.m_idx;
    for(node_iterator it = begin(); it != end(); ++it){
      if((*it)->isNode() && (*it)->getTag() == idx.m_tag && !idxNr--){
        return **it;
      }
    }
    throw ChildNotFoundException(__FUNCTION__ + str(": unable to find node with tag name ") + idx.m_tag + " and index " +  str(idx.m_idx) );
    return *this;
  }
  
  const XMLNode &XMLNode::operator[](const XMLNodeIdx &idx) const throw (ChildNotFoundException,InvalidNodeTypeException){
    return const_cast<XMLNode*>(this)->operator[](idx);
  }

  
  
  XMLNode &XMLNode::getFirstChildNode(int types) throw (ChildNotFoundException,InvalidNodeTypeException){
    if(types == ALL){
      return this->operator[](0u);
    }else{
      if(m_type != NODE) throw InvalidNodeTypeException("only NODE nodes may have child nodes");
      for(node_iterator it=begin(); it!= end(); ++it){
        if( (*it)->getType() & types ) return **it;
      }
      throw ChildNotFoundException(__FUNCTION__);
      return this->operator[](0u); // just to avoid compiler warning
    }
  }
  const XMLNode &XMLNode::getFirstChildNode(int types) const throw (ChildNotFoundException,InvalidNodeTypeException){
    return const_cast<XMLNode*>(this)->getFirstChildNode(types);
  }

  XMLNode &XMLNode::getLastChildNode(int types) throw (ChildNotFoundException,InvalidNodeTypeException){
    if(types == ALL){
      return this->operator[]((unsigned int)getChildCount()-1);
    }else{
      if(m_type != NODE) throw InvalidNodeTypeException("only NODE nodes may have child nodes");
      for(node_iterator it=--end(); it>= begin(); ++it){
        if( (*it)->getType() & types ) return **it;
      }
      throw ChildNotFoundException(__FUNCTION__);
      return this->operator[](0u); // just to avoid compiler warning

    }
  }
  const XMLNode &XMLNode::getLastChildNode(int types) const throw (ChildNotFoundException,InvalidNodeTypeException){
    return const_cast<XMLNode*>(this)->getLastChildNode(types);
  }

  
  const XMLAttribRef XMLNode::operator()(const std::string &attributeName) const throw (AttribNotFoundException,InvalidNodeTypeException){
    if(!isNode()) throw InvalidNodeTypeException("only NODE nodes may have attibutes (attibute: " + attributeName + ")");
    return XMLAttribRef(m_attribs,attributeName);
  }

  XMLAttribRef XMLNode::operator()(const std::string &attributeName) throw (InvalidNodeTypeException){
    if(!isNode()) throw InvalidNodeTypeException("only NODE nodes may have attibutes (attibute: " + attributeName + ")");
    return XMLAttribRef(m_attribs,attributeName);
  }
  
  XMLNodePtr XMLNode::deepCopy(XMLNode *newParent, XMLDocument *newDoc) const{
    XMLNode *cpy = new XMLNode;
    cpy->m_type = m_type;
    cpy->m_parent = newParent;
    cpy->m_document = newDoc;
    if(isNode()){
      for(const_node_iterator it=begin();it!=end();++it){
        cpy->m_subnodes.push_back((*it)->deepCopy(cpy,newDoc));
      }
    }
    if(hasAttributes()){
      cpy->m_attribs = XMLAttMapPtr(new XMLAttMap(attrib_begin(), attrib_end()));
    }
    cpy->m_content = m_content;
    return cpy;
  }
  
  XMLNodePtr XMLNode::delSelf(){
    if(!m_parent){
      XMLNodePtr _this = m_document->m_root;
      m_document->m_root = XMLNodePtr();
      return _this;
    }else{
      for(node_iterator it = m_parent->begin(); it != m_parent->end(); ++it){
        if(it->get() == this){
          XMLNodePtr _this= *it;
          m_parent->m_subnodes.erase(it);
          return _this;
        }
      }
      throw std::logic_error("node was not able to remove itself ? why ?");
    }
  }

  void XMLNode::setText(const std::string &text) throw (InvalidNodeTypeException){
    if(!isText()) throw InvalidNodeTypeException("Text can only be assigned to TEXT");
    m_content = text;
  }

  std::string XMLNode::getPath(const std::string &delim) const{
    const XMLNode *curr = this;
    std::list<std::string> l;
    while(curr){
      if(curr->isNode()){
        l.push_front(curr->getTag());
      }else{
        std::string cmt = curr->getComment();
        l.push_front(str("@comment{")+cmt.substr(0,iclMax(7,(int)cmt.size()-1))+"...}@");
      }
      curr = curr->m_parent;
    }

    std::ostringstream str;
    std::copy(l.begin(),l.end(),std::ostream_iterator<std::string>(str,delim.c_str()));
    return str.str();
  }

  std::string XMLNode::toString(bool recursive) const{
    std::ostringstream str;
    serialize(str,0,false,recursive);
    return str.str();
  }

  std::ostream &operator<<(std::ostream &os, const XMLNode &node){
    node.serialize(os,0,false,true);
    return os;
  }
  

  std::vector<XMLNode*> XMLNode::getAllChildNodes(const XMLNodeFilter &filter){
    std::vector<XMLNode*> v;
    switch(getType()){
      case NODE:
        if(filter(*this)) {
          v.push_back(this);
        }
        for(node_iterator it = begin(); it != end(); ++it){
          std::vector<XMLNode*> s = (*it)->getAllChildNodes();
          for(std::vector<XMLNode*>::iterator it2 = s.begin(); it2!= s.end(); ++it2){
            if(filter(**it2)){
              v.push_back(*it2);
            }
          }
        }
        break;
      default:
        if(filter(*this)){
          v.push_back(this);
        }
    }
    return v;
  }
  
  const std::vector<XMLNode*> XMLNode::getAllChildNodes(const XMLNodeFilter &filter) const{
    return const_cast<XMLNode*>(this)->getAllChildNodes(filter);
  }


  XMLNodeIterator XMLNode::recursive_begin(const XMLNodeFilter &filter){
    return XMLNodeIterator(*this,filter);
  }
  XMLNodeIterator XMLNode::recursive_end(){
    return XMLNodeIterator();
  }

  XMLNodeConstIterator XMLNode::recursive_begin(const XMLNodeFilter &filter) const{
    return XMLNodeConstIterator(*this,filter);
  }
  XMLNodeConstIterator XMLNode::recursive_end() const{
    return XMLNodeConstIterator();
  }

  bool XMLNode::hasChild(const std::string &tag, int types, int maxDepth,
                         const std::string attrib1, const std::string &value1,
                         const std::string attrib2, const std::string &value2,
                         const std::string attrib3, const std::string &value3) const{
    XMLNodeFilterCombination c = ((tag=="") ? (const XMLNodeFilter&)XMLNodeFilterByTag(tag) : 
                                  (const XMLNodeFilter&)XMLNodeFilterAll()) & XMLNodeFilterByType(types);
    if(maxDepth > 0){
      c = c | XMLNodeFilterByLevel(maxDepth);
    }
    if(attrib1 != ""){
      c = c | XMLNodeFilterByAttrib(attrib1,value1);
    }
    if(attrib2 != ""){
      c = c | XMLNodeFilterByAttrib(attrib2,value2);
    }
    if(attrib3 != ""){
      c = c | XMLNodeFilterByAttrib(attrib3,value3);
    }
    return hasChild(c);
  }

  bool XMLNode::hasChild(const XMLNodeFilter &filter) const{
    return getAllChildNodes(filter).size();
  }

}
