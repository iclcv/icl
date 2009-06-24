#ifndef ICL_XML_DOCUMENT_H
#define ICL_XML_DOCUMENT_H

#include <iclXMLNode.h>

namespace icl{

  /// XMLDocument type \ingroup XML
  /** Unlike other (more sophisticated) XML API's ICL's XML API does only support a small subset of XML
      functionality. Furthermore it uses a simpler node concept, to underline it's simpler internal
      implementation. \n
      
      \section WIND ICL's XMLDocument approach
      XMLDocument essentially consist of a single node, which is the root of a node tree. Nodes are of type
      XMLNode which provides most of the data access functionalities for XMLDocument structures. The document 
      tree is built by Nodes of different types:
      - leaf types TEXT, and COMMENT (<a>text</a>, ..)
      - a branch type CONTAINER (<a><bxy/><c/>...</a>
      - and a still adjustable type SINGLE (<a/>). Single nodes can become TEXT or CONTAINER nodes later on.
      
      Furthermore, a document holds a set of so called global comments. These are comment tags, tag are found
      and parsed before the first 'real' node was encountered. An optional XML-Version description can be accessed
      from the XMLDocument structure as well.
    
      \section RES Restricted XML
      To stem implementation expense, we restricted parsable XML structures as follows (maybe not complete yet):
      - no namespace support at all
      - no XML-Document scheme support at all
      - Simpler structure:
        -# Nodes must not have child nodes AND text content
        -# Text is always a node type like <a>the-text</a>, in this case the node is
           'a' with text 'the-text', 'the-text' is not independent node
        -# Comments within a text section are not allowed 
      
      \section WID What is does:
      We found out, that common XML use for applications consists most of the time in simply writing data
      to disk (e.g. configuration files) or passing data from one application to another one (files, network-streams,
      pipes). ICL's xml approach is just simple utility to help the user doing this without encountering problems
      with linking against complex xml libraries like Xerxes

      \section EX Examples
  */
  class XMLDocument{
    
    /// root node pointer if this is null, the document is null
    XMLNodePtr m_root;
    
    /// comments found before the first 'real' node could be parsed
    std::vector<std::string> m_headerComments;
    
    /// extracted xml description (<?xml ...?>)
    std::string m_xmlVersion;
    
    public:
    
    /// create an empty document 
    /** empty documents cannot be filled later on, rather they work
        as placeholder for other documents. To enable an null document,
        another non-null document can simply be assigned to it */
    XMLDocument();
    
    /// create a document from given stream (e.g. std::ofstream)
    XMLDocument(std::istream &is) throw (ParseException);
    
    /// create a document from given xml-text representation
    XMLDocument(const std::string &text) throw (ParseException);

    /// deep copy of a document
    XMLDocument(const XMLDocument &other);
    
    /// deep copying assignment operator
    XMLDocument &operator=(const XMLDocument &other);

    /// returns the documents root node (const)
    const XMLNodePtr getRootNode() const;

    /// returns the documents root node
    XMLNodePtr getRootNode();
    
    /// provides direct access to the documents root node
    XMLNode* operator->() throw (NullDocumentException);;

    /// provides direct access to the documents root node (const)
    const XMLNode* operator->() const throw (NullDocumentException);;
    
    /// friend ostream operator
    friend std::ostream &operator<<(std::ostream &os, const XMLDocument &doc) throw (NullDocumentException);
    
    /// for tight integration with XMLNode class
    friend class XMLNode;
    
    /// compatibility function to XMLNodes (returns documents root node)
    /** Actually this functions is less convenient then getRootNode however is does not provide
        more functionality. Once having XMLNodes node hierarchies can be accessed by using the []-
        operator. The [] operator returns exactly the documents root node if given tag matches to
        the root nodes tag name. Otherwise, a ChildNotFoundException is thrown.
    */
    XMLNode &operator[](const std::string &tag) throw (NullDocumentException, ChildNotFoundException);

    /// compatibility function to XMLNodes (returns documents root node) (const version)
    const XMLNode &operator[](const std::string &tag) const throw (NullDocumentException, ChildNotFoundException);

    /// returns whether this document has a root node that is not null
    bool isNull() const;
    
    /// returns the documents header comments
    /** Header comments are those comment tags, that are found in the source document xml-stream or string
        before the first 'real' node was found */
    inline const std::vector<std::string> &getHeaderComments() const { return m_headerComments; }
    
    /// returns the documents xml description information
    inline const std::string &getXMLDescriptor() const { return m_xmlVersion; }

    /// recursively removes all comments from the document 
    /** To obtain a list of all events, call getRootNode().removeAllComments() */
    void removeAllComments();
    
    /// loads document from file
    static XMLDocument load(const std::string &fileName) throw (ParseException,FileNotFoundException);
    
    /// writes a document into a file
    static void save(const XMLDocument &doc, const std::string &fileName) throw (ICLException,NullDocumentException);

    
    
    private:
    /// internal initialization function
    void init(std::istream &is);
    
    /// internal parsing function
    static void parse(XMLNode *root, std::istream &src,
                      std::vector<std::string> &m_headerComments,
                      std::string &xmlVersionDescription);
    
    /// internal parsing function
    static void parse_it(XMLNode *instance, std::istream &src, void *state);
    
  };

  /// ostream operator unfolds document tree to text form (always with new-lines) \ingroup XML
  std::ostream &operator<<(std::ostream &os, const XMLDocument &doc) throw (NullDocumentException);
  
  
}

#endif

