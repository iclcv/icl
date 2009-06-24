#include <iclXMLNodeIterator.h>
#include <iclXMLNode.h>


namespace icl{
  
  struct XMLNodeIterator::XMLNodeIteratorImpl{
    XMLNodeIteratorImpl():idx(-1){}
    XMLNodeIteratorImpl(const std::vector<XMLNode*> &v):
      v(v),idx(0){}
    std::vector<XMLNode*> v;
    int idx;
  };
  
  XMLNodeIterator::XMLNodeIterator(){
    impl = new XMLNodeIteratorImpl;
  }
  
  XMLNodeIterator::XMLNodeIterator(const XMLNode &origin, const XMLNodeFilter &filter){
    std::vector<XMLNode*> v = const_cast<XMLNode&>(origin).getAllChildNodes(filter);
    impl = new XMLNodeIteratorImpl(v);
  }

  XMLNodeIterator::XMLNodeIterator(const XMLNodeIterator &other){
    *impl = *other.impl;
  }

  XMLNodeIterator& XMLNodeIterator::operator=(const XMLNodeIterator &other){
    *this->impl = *other.impl;
    return *this;
  }

  XMLNodeIterator::~XMLNodeIterator(){
    ICL_DELETE(impl);
  }
  
  XMLNodeIterator XMLNodeIterator::operator++(int){
    XMLNodeIterator it = *this;
    ++impl->idx;
    return it;
  }

  XMLNodeIterator &XMLNodeIterator::operator++(){
    ++impl->idx;
    return *this;
  }

  XMLNode* XMLNodeIterator::operator->(){
    return impl->v[impl->idx];
  }

  const XMLNode* XMLNodeIterator::operator->() const{
    return impl->v[impl->idx];
  }

  
  XMLNode &XMLNodeIterator::operator*(){
    return *impl->v[impl->idx];
  }

  const XMLNode &XMLNodeIterator::operator*() const{
    return *impl->v[impl->idx];
  }
  
  bool XMLNodeIterator::operator!=(const XMLNodeIterator &other) const{
    return !operator==(other);
  }
  
  bool XMLNodeIterator::operator==(const XMLNodeIterator &other) const{
    if(impl->idx == -1 && other.impl->idx == -1) return true;
    if(impl->idx == -1) return other.impl->idx == (int)other.impl->v.size();
    if(other.impl->idx == -1) return impl->idx == (int)impl->v.size();
    return impl->idx == other.impl->idx;
  }
}
