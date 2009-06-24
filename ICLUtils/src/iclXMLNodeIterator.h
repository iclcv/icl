#ifndef ICL_XML_NODE_ITERATOR_H
#define ICL_XML_NODE_ITERATOR_H

#include <iclXML.h>
#include <iclXMLNodeFilter.h>
#include <iterator>

namespace icl{
  
  /// Utility structure that serves as recursive iterator type \ingroup XML
  /** Unlike the XMLNode::node_iterator type, the XMLNode::recursive_node_iterator, which 
      is a typedef to XMLNodeIterator can be used to iterate recursively though
      all subnodes of a node, that optionally have to match given conditions represented
      by an instance of XMLNodeFilter.
      
      \section Constness
      The XMLNodeIterator is also available in it's const version: XMLNodeConstIterator.
      The Iterator implements all necessary functions for an std::forward_iterator type, that is 
      it can be incremented using pre- and post-scrip ++ operator and it can be compared with
      other iterators using == and != operators. Furthermore, the -> operator provides access
      to the current XMLNode* and the * operator can be used to get a reference to
      that node.
      
      \section Creation
      The XMLNodeIterator class uses a private constructor, so it can only be
      copied or initialized without referencing a node. Use XMLNode::recursive_begin(...) 
      and XMLNode::recursive_end to get valid instances of XMLNodeIterator.

      \section Complexity
      <b>Please Note</b> That creation of a recursive iterator must descend the whole
      document tree originating from the source node. This may take some time especially for
      huge XML documents.
  */
  class XMLNodeIterator :  public std::iterator<std::forward_iterator_tag,XMLNode>{

    /// Internal hidden implementation type
    class XMLNodeIteratorImpl;

    /// Internal hidden implementation data
    XMLNodeIteratorImpl *impl;
    
    /// Main constructor only available by the friend class XMLNode
    XMLNodeIterator(const XMLNode &origin, const XMLNodeFilter &filter);
    public:
    
    /// Tight integration with XMLNode class
    friend class XMLNode;

    /// Tight integration with XMLNodeConstIterator wrapper for const node access
    friend class XMLNodeConstIterator;

    /// Empty constructor (creates an end-iterator)
    XMLNodeIterator();
    
    /// Copy constructor
    XMLNodeIterator(const XMLNodeIterator &other);

    /// Destructor
    ~XMLNodeIterator();

    /// Assignment operator
    XMLNodeIterator &operator=(const XMLNodeIterator &other);
    
    /// post-increment operator
    XMLNodeIterator operator++(int);

    /// pre-increment operator
    XMLNodeIterator &operator++();

    /// dereference operator (providing access to current node)
    XMLNode &operator*();
    
    /// dereference operator (providing access to current node) (const)
    const XMLNode &operator*() const;

    /// pointer operator (providing access to current node)
    XMLNode* operator->();

    /// pointer operator (providing access to current node) (const)
    const XMLNode* operator->() const;

    /// inequality comparison operator
    bool operator!=(const XMLNodeIterator &other) const;

    /// equality comparison operator
    bool operator==(const XMLNodeIterator &other) const;
  };
  
 
  
}


#endif
