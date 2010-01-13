#ifndef XML_NODE_CONST_ITERATOR_H
#define XML_NODE_CONST_ITERATOR_H

#include <ICLUtils/XMLNodeIterator.h>

namespace icl{

  /// Shallow wrapper of a XMLNodeIterator instance that provides const access to nodes \ingroup XML
  /** @see XMLNodeIterator */
  class XMLNodeConstIterator : public std::iterator<std::forward_iterator_tag,XMLNode>{
    
    /// Wrapped uncost iterator type
    XMLNodeIterator it;
    
    /// Private 'real' constructor only available by the friend class XMLNode
    XMLNodeConstIterator(const XMLNode &origin, const XMLNodeFilter &filter):it(origin,filter){}

    public:

    /// Empty constructor creating an end-iterator
    XMLNodeConstIterator(){}
    
    /// For tight integration with XMLNode class
    friend class XMLNode;
    
    /// pre-increment operator
    XMLNodeConstIterator operator++(int){
      XMLNodeConstIterator t(*this);  ++it;  return t;
    }
   
    /// post-increment operator
    XMLNodeConstIterator &operator++(){
      ++it; return *this;
    }
    
    /// dereference operator (providing access to current node) (const only)
    const XMLNode &operator*() const{
      return it.operator*();
    }

    /// pointer operator (providing access to current node) (const only)
    const XMLNode* operator->() const{
      return it.operator->();
    }

    /// inequality comparison operator
    bool operator!=(const XMLNodeConstIterator &other) const{
      return it != other.it;
    }
    /// equality comparison operator
    bool operator==(const XMLNodeConstIterator &other) const{
      return it == other.it;
    }
  };

}
#endif
