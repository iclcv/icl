/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
** University of Bielefeld                                                **
** Contact: nivision@techfak.uni-bielefeld.de                             **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_XML_NODE_H
#define ICL_XML_NODE_H

#include <ICLUtils/XML.h>
#include <map>
#include <vector>
#include <iostream>
#include <ICLUtils/XMLAttribRef.h>
#include <ICLUtils/XMLNodeIdx.h>
#include <ICLUtils/XMLNodeConstIterator.h>
#include <ICLUtils/XMLNodeFilter.h>


namespace icl{

  /// recursive node type for xml document trees  \ingroup XML
  /** Nodes have a special type (see XMLNode::Type enum) that defines
      how that node is treated internally and which functions are valid
      on it (e.g. a comment node may not have children)
  */
  class XMLNode{
    public:
    /// Node type enumeration
    /** Dependent on the nodes type, operations are allowed on it or not 
        - TEXT nodes are only allowed between other nodes and may (of course) not have children
        - NODE nodes (like <foo/> or also <foo></foo> are allowed to have child nodes. Child nodes
          may have arbitrary type
        - COMMENT nodes (like <!-- this is a comment -->) can be used
          to put comments into the document (As comment nodes are treated as normal nodes,
          they can only be children of CONTAINER nodes)\n
          Please note, that it is allowed to put comment nodes outside the documents root tags. These comment 
          nodes are not part of the document tree structure, rather they are accessible by the XMLDocument's
          getHeaderComments function.
        - UNDEFINED nodes will only occur in case of heavily incorrect XML documents
        
    */
    enum Type { TEXT=1,     //<! text nodes
                NODE=2,     //<! nodes like <bla/> or <bla>...</bla> 
                COMMENT=4,  //<! nodes like <!-- foo bar -->
                UNDEFINED=8, //<! internally used type
                ALL=TEXT|NODE|COMMENT //<! shortcut for all typs at once
              };
    
          
    /// const Attribute iterator (std::map<std::string,std::string>::const_iterator
    typedef XMLAttMap::const_iterator const_attrib_iterator;

    /// Attribute iterator (std::map<std::string,std::string>::iterator
    typedef XMLAttMap::iterator attrib_iterator;

    /// const sub node iterator (std::vector<XMLNodePtr>::const_iteratora
    typedef std::vector<XMLNodePtr>::const_iterator const_node_iterator;

    /// sub node iterator (std::vector<XMLNodePtr>::const_iterator
    typedef std::vector<XMLNodePtr>::iterator node_iterator;
    
    /// recursive sub node iterator
    typedef XMLNodeIterator recursive_node_iterator;

    /// const recursive sub node iterator
    typedef XMLNodeConstIterator const_recursive_node_iterator;

    /// tight integration with XMLDocument class
    friend class XMLDocument;
    
    /// ostream operator (XMLDocument)
    friend std::ostream &operator<<(std::ostream &os, const XMLDocument &doc) throw (NullDocumentException);    

    /// ostream operator
    friend std::ostream &operator<<(std::ostream &os, const XMLNode &node);

    /// returns whether the node is a text node like ...text...
    bool isText() const;
    
    /// returns whether the node is a comment node like <!-- comment -->
    bool isComment() const;
    
    /// return whether the node is 'normal' node that is allowed to have children
    bool isNode() const;
    
    /// returns whether this node is currently null
    bool isNull() const;
    
    /// return whether this node is a documents root node
    bool isRoot() const;
    
    /// returns the nodes tag
    /** throws an exception on COMMENT and TEXT nodes*/
    const std::string &getTag() const throw (InvalidNodeTypeException);

    /// returns nodes text entry
    /** throws an exception on COMMENT and NODE nodes */
    const std::string &getText() const throw (InvalidNodeTypeException);

    /// returns nodes comment 
    /** throws an exception on NODE and TEXT nodes */
    const std::string &getComment() const throw (InvalidNodeTypeException);

    
    /// returns an iterator to he first attribute (const)
    /** throws an exception on non-NODE nodes*/
    const_attrib_iterator attrib_begin() const throw (InvalidNodeTypeException);

    /// returns the attribute end iterator (const)
    /** throws an exception on non-NODE nodes*/
    const_attrib_iterator attrib_end() const throw (InvalidNodeTypeException);


    /// returns an iterator to he first attribute
    /** throws an exception on non-NODE nodes*/
    attrib_iterator attrib_begin() throw (InvalidNodeTypeException);

    /// returns the attribute end iterator
    /** throws an exception on non-NODE nodes*/
    attrib_iterator attrib_end() throw (InvalidNodeTypeException);
    
    /// returns the sub node begin iterator (const)
    /** throws an exception on non-NODE nodes */
    const_node_iterator begin() const throw (InvalidNodeTypeException);

    /// returns the sub node end iterator (const)
    /** throws an exception on non-NODE nodes */
    const_node_iterator end() const throw (InvalidNodeTypeException);

    /// returns the sub node begin iterator
    /** throws an exception on non-NODE nodes */
    node_iterator begin() throw (InvalidNodeTypeException);

    /// returns the sub node end iterator
    /** throws an exception on non-NODE nodes */
    node_iterator end() throw (InvalidNodeTypeException);
    

    /// returns the hierarchy level of this node (root-node -> 0)
    int getLevel() const;

    /// returns this nodes document
    XMLDocument *getDocument();

    /// returns this nodes document (const);
    const XMLDocument *getDocument() const;

    /// returns this nodes parent node (NULL for root nodes)
    XMLNode *getParent();

    /// returns this nodes parent node (const) (NULL for root nodes)
    const XMLNode *getParent() const;

    /// Returns node type
    Type getType() const;
    
    /// Returns number of children of this node (0 for all but NODE nodes)
    int getChildCount() const;
    
    /// Returns whether this node has children at all (false for all but NODE nodes)
    bool hasChildren() const;
    
    /// Returns whether this node has attributs at all (false for all but NODE nodes)
    bool hasAttributes() const;
    
    /// Returns whether this node has given attribute (with optionally given value)
    /** if value is left "", only the presence of given attribute is mandatory */
    bool hasAttribute(const std::string &name, const std::string &value="") const; 
    
    /// Returns this nodes attribute count 
    int getAttributeCount() const ;
    
    /// returns whether this node has a child with given properties
    /** @param tag tagname filter (if "": no tag filter is applied)
        @param types or-ed list of allowed types (if 0: no type filter is applied)
        @param maxDepth maximun depths hierarchy depths of search (if 0, no subnode
                        would be found, so 0 is an indicator for 'any depth'
        @param attrib1 mandatory attibute (if "": attrib1 and value1 are not used) 
        @param value1 mandatory value of attrib1 (if "": abbrib1 just has to be present)
        @param attrib2 mandatory attibute (if "": attrib2 and value2 are not used) 
        @param value2 mandatory value of attrib2 (if "": abbrib2 just has to be present)
        @param attrib3 mandatory attibute (if "": attrib3 and value3 are not used) 
        @param value3 mandatory value of attrib3 (if "": abbrib3 just has to be present)
    */
    bool hasChild(const std::string &tag, int types=0xFFFF, int maxDepth=0,
                  const std::string attrib1="", const std::string &value1="",
                  const std::string attrib2="", const std::string &value2="",
                  const std::string attrib3="", const std::string &value3="") const;

    /// returns whether this node has a child wrt. given XMLNodeFilter
    bool hasChild(const XMLNodeFilter &filter=XMLNodeFilterAll()) const;

    /// adds a new child node
    /** If given child node has already a document the DoubledReferenceException is thrown. So if 
        one wants e.g. to add a node from another document by this:
        \code
        XMLDocument d1(...);
        XMLDocuemtn d2(...);
        
        d1.getRootNode().addNode(&d2["config"]);
        \endcode
        The given node would be reference by to documents simultaneously, what is not allowed.

        So one has to be remove the node from the other document or to call addCopy instead
        \code
        XMLDocument d1(...);
        XMLDocuemtn d2(...);
        
        d1.getRootNode().addNode(d2["config"].removeChildNode(0));

        d1.getRootNode().addCopy(d2["config"][0]);
        \endcode
        
        If this node is not of container type, an InvalidNodeTypeException is thrown;
        
        @param child child node to add
        @param index destination index (just a hint which is not regarded, if it cannot be fulfilled)
    */
    void addNode(XMLNodePtr child, int index=-1) throw (InvalidNodeTypeException,DoubledReferenceException);
    
    /// Adds a copy of given node at given index
    /** @param child child node to add a copy of
        @param index destination index (just a hint which is not regarded, if it cannot be fulfilled)
        @see addNode
   */
    void addCopy(const XMLNode &child, int idx=-1) throw (InvalidNodeTypeException, ParseException);

    /// Adds a new text node to this one at given index
    /** @param text text of the new node
        @param index destination index (just a hint which is not regarded, if it cannot be fulfilled)
        @see addNode
   */
    void addText(const std::string &text, int index=-1) throw (InvalidNodeTypeException);

    /// Adds a new single node to this one at given index
    /** @param tag tag of the new text node
        @param index destination index (just a hint which is not regarded, if it cannot be fulfilled)
        @see addNode
    */
    void addNode(const std::string &tag, int index=-1)  throw (InvalidNodeTypeException);

    /// Convenience function to add a NODE node with a single child of type TEXT
    /** @param tag tag of the new NODE node
        @param text text body of that node
        @param index destination index (just a hint which is not regarded, if it cannot be fulfilled)
        @see addNode
    */
    void addNodeWithTextContent(const std::string &tag,const std::string &text, int index=-1)  throw (InvalidNodeTypeException);


    /// Adds a new comment node to this one at given index
    /** @param comment comment to add
        @param index destination index (just a hint which is not regarded, if it cannot be fulfilled)
        @see addNode
    */
    void addComment(const std::string &comment, int index=-1)  throw (InvalidNodeTypeException);

    /// Adds a new node (or even node hierarchy from xml representation)
    /** @param xml xml representation of new node
        @param index destination index (just a hint which is not regarded, if it cannot be fulfilled)
        @see addNode
   */
    void addXML(const std::string &xml, int idx=-1) throw (InvalidNodeTypeException, ParseException);

    /// Adds a new node (or even node hierarchy from xml stream)
    /** @param is stream that provides representation of new node
        @param index destination index (just a hint which is not regarded, if it cannot be fulfilled)
        @see addNode
   */
    void addXML(std::istream &is, int idx=-1) throw (InvalidNodeTypeException, ParseException);

    /// converts this node into an xml-string
    std::string toString(bool recursive=true) const;


    
    /// removes child node at given index
    /** The removed child node is set up to be importable by other nodes and it is returned. If given index is not
        valid, a ChildNotFoundException is thrown. if this node is not of container type, an InvalidNodeTypeException
        is thrown
        @param index node index to delete
    */
    XMLNodePtr removeChildNode(unsigned int index) throw (ChildNotFoundException,InvalidNodeTypeException);


    /// removes child node with given tag name id 
    /** The removed child nodes are set up to be importable by other nodes and are returned. If given tag cannot
        be found at least once, a ChildNotFoundException is thrown. if this node is not of container type, an InvalidNodeTypeException
        is thrown
        @param tagName node tag name to remove 
        @param all if set to true all nodes with given tagName are removed rather then the first one only
    */
    std::vector<XMLNodePtr> removeChildNode(const std::string &tagName, bool all=false) throw (ChildNotFoundException,InvalidNodeTypeException);
    
    /// removes all comments from this node and its children
    std::vector<XMLNodePtr> removeAllComments();

    /// returns all (recursive) child nodes
    /** recursively collects all child nodes in following order:
        <pre>
        <node1>
           <node2>
             <node3/>
             <node4/>
           </node2>
           <node5/>
        </node1>
        </pre>

        Note: Non-recursive access to all direct child nodes can be obtained by function begin() and end() or
              also using operator[] in combination with getChildCount()
        @param xpathFilter currently not implemented filter expression (possibly we will provide a
               small subset from XPATH functionality instead
    */
    std::vector<XMLNode*> getAllChildNodes(const XMLNodeFilter &filter=XMLNodeFilterAll());

    /// returns list of  (recursive) child nodes matchin expression (which is not yet supported) (const version)
    /** @see un-const version */
    const std::vector<XMLNode*> getAllChildNodes(const XMLNodeFilter &filter=XMLNodeFilterAll()) const;
    
    /// returns an iterator to all nodes matching given xpath-filter (xpath not yet implemented)
    /** For details on current xpath implementation: @see getAllChildNodes(const std::string&)*/
    recursive_node_iterator recursive_begin(const XMLNodeFilter &filter=XMLNodeFilterAll());

    /// returns an end iterator to all nodes matching given xpath-filter (xpath not yet implemented)
    /** For details on current xpath implementation: @see getAllChildNodes(const std::string&)*/
    recursive_node_iterator recursive_end();

    /// returns a const iterator to all nodes matching given xpath-filter (xpath not yet implemented)
    /** For details on current xpath implementation: @see getAllChildNodes(const std::string&)*/
    const_recursive_node_iterator recursive_begin(const XMLNodeFilter &filter=XMLNodeFilterAll()) const;

    /// returns a const end iterator to all nodes matching given xpath-filter (xpath not yet implemented)
    /** For details on current xpath implementation: @see getAllChildNodes(const std::string&)*/
    const_recursive_node_iterator recursive_end() const;

    /// returns sub-node at given index
    /** If given index is not valid, a ChildNotFoundException is thrown. If otherwise the node is not of container type, an
        InvalidNodeTypeException is thrown. 
        @param index sub node index
        */
    XMLNode &operator[](unsigned int index) throw (ChildNotFoundException,InvalidNodeTypeException);
    
    /// returns sub-node at given index (const)
    /** If given index is not valid, a ChildNotFoundException is thrown. If otherwise the node is not of container type, an
        InvalidNodeTypeException is thrown. 
        @param index sub node index
    */
    const XMLNode &operator[](unsigned int index) const throw (ChildNotFoundException,InvalidNodeTypeException);

    /// returns first sub-node with given tag name
    /** If given tag name cannot be found, a ChildNotFoundException is thrown. If otherwise the node is not of container type, an
        InvalidNodeTypeException is thrown. 
        @param tag tag name to search for in child list
    */
    XMLNode &operator[](const std::string &tag) throw (ChildNotFoundException,InvalidNodeTypeException);

    /// returns first sub-node with given tag name (const)
    /** If given tag name cannot be found, a ChildNotFoundException is thrown. If otherwise the node is not of container type, an
        InvalidNodeTypeException is thrown. 
        @param tag tag name to search for in child list
    */
    const XMLNode &operator[](const std::string &tag) const throw (ChildNotFoundException,InvalidNodeTypeException);

    
    /// returns index'th child node with given tag name 
    /** The XMLNodeIdx utility structure can easily be used with the global function xmlidx e.g.
        \code
        doc["root"][xmlidx("section",5)].addCommentNode("this is section 5");
        \endcode
        The first and the last child of a node can be accessed using a special macro, that creates
        an appropriate XMLNodeIdx structure for that: LAST_CHILD and FIRST_CHILD
        
        If the node is not of container type, an InvalidNodeTypeException is thrown. 
        @param idx if invalid a ChildNotFoundException is thrown
    */
    XMLNode &operator[](const XMLNodeIdx &idx) throw (ChildNotFoundException,InvalidNodeTypeException);

    /// returns index'th child node with given tag name (const)
    /** The XMLNodeIdx utility structure can easily be used with the global function xmlidx e.g.
        \code
        doc["root"][xmlidx("section",5)].addCommentNode("this is section 5");
        \endcode
        The first and the last child of a node can be accessed using a special macro, that creates
        an appropriate XMLNodeIdx structure for that: LAST_CHILD and FIRST_CHILD

        If the node is not of container type, an InvalidNodeTypeException is thrown. 
        @param idx if invalid a ChildNotFoundException is thrown
        */
    const XMLNode &operator[](const XMLNodeIdx &idx) const throw (ChildNotFoundException,InvalidNodeTypeException);
    
    /// returns first child node
    XMLNode &getFirstChildNode(int types=ALL) throw (ChildNotFoundException,InvalidNodeTypeException);

    /// returns first child node (const)
    const XMLNode &getFirstChildNode(int types=ALL) const throw (ChildNotFoundException,InvalidNodeTypeException);



    
    /// returns last child node
    XMLNode &getLastChildNode(int types=ALL) throw (ChildNotFoundException,InvalidNodeTypeException);

    /// returns last child node (const)
    const XMLNode &getLastChildNode(int type=ALL) const throw (ChildNotFoundException,InvalidNodeTypeException);

    /// returns the tree path from document root to this node with given delimiter
    /** e.g. using this document
        <pre>
        <root>
          <section1>
             <data1/>
             <data2/>
          </section1>
          <section2>
             <data3/>
             <data4/>
          </section2>
        </root>
        </pre>
        
        doc["root"][section1][data1].getPath() returns root.section1.data1
        doc["root"][section2].getPath() returns root.section2
        
        This is trivial when the document is approached using the []-operators,
        but it is quiet useful, when the document is explored using node iterators
    */
    std::string getPath(const std::string &delim=".") const;
    
    /// returns a reference to given attribute with given name (const)
    /** Throws an InvalidNodeTypeException on comment nodes and a AttribNotFoundException if given attribute does not exist.
        <b>Note:</b> In this const version attributes cannot be setup with other values
        @param attributeName name of the attribute to search for
    */
    const XMLAttribRef operator()(const std::string &attributeName) const throw (AttribNotFoundException,InvalidNodeTypeException);


    /// Returns a writable reference to given attribute with given name
    /** Throws an InvalidNodeTypeException on comment nodes. If reference attribute does not exist, it can be crated by simply
        assigning any value to the returned XMLAttribRef value. So we have read/write-access here
        @param attributeName name of the attribute to reference
    */
    XMLAttribRef operator()(const std::string &attributeName) throw (InvalidNodeTypeException);

    
    /// Sets the text of this node to a given string
    /** This does (of course) only work on TEXT
        nodes, otherwise an InvalidNodeTypeException is thrown */
    void setText(const std::string &text) throw (InvalidNodeTypeException);


    /// Assigns a string representation of the given T instance to this node as text entry
    /** This is equivalent to setText(icl::str(t)) 
        @see setText and icl::str for more details*/
    template<class T>
    XMLNode &operator=(const T &t) throw (InvalidNodeTypeException){
      setText(str(t));
      return *this;
    }
    
    /// deletes this note from it's parent node and from it's parents document
    XMLNodePtr delSelf();

    private:
    
    /// Private creation constructor (XMLNodes can only be created by documents and by member functions)
    XMLNode();

    /// Private creation constructor (XMLNodes can only be created by documents and by member functions)
    XMLNode(XMLNode *parent, XMLDocument *doc);

    /// Parent node (NULL for root nodes)
    XMLNode *m_parent;
    
    /// Parent document
    XMLDocument *m_document;
    
    /// Node type
    Type m_type;

    /// list of sub nodes
    std::vector<XMLNodePtr> m_subnodes;
    
    /// attribute map
    XMLAttMapPtr m_attribs;
    
    /// text value (only for text nodes)
    std::string m_content;

    /// recursive serialization function (used by toString() and ostream-operator)
    void serialize(std::ostream &s, int level, bool lastNodeWasText, bool recursive) const;

    /// deep copy function (only accessible internally)
    XMLNodePtr deepCopy(XMLNode *newParent, XMLDocument *newDoc) const;

  };

  /// std::ostream operator \ingroup XML
  std::ostream &operator<<(std::ostream &os, const XMLNode &node);

  
}

#endif
