#ifndef ICL_XML_H
#define ICL_XML_H

#include <ICLUtils/SmartPtr.h>
#include <ICLUtils/Exception.h>
#include <string>
#include <map>

namespace icl{

  /// Basic xml exception type used for this xml package \ingroup XML
  struct XMLException : public ParseException{
    XMLException(const std::string &err) throw():
    ParseException("XML (" + err +")"){}
    ~XMLException() throw(){}
  };

  /// Thrown if a referenced child was not found \ingroup XML
  struct ChildNotFoundException : public XMLException{
    ChildNotFoundException(const std::string &text) throw() :XMLException(text){}
      ~ChildNotFoundException() throw() {};
  }; 

  /// Thrown if a referenced attribute was not found \ingroup XML
  struct AttribNotFoundException : public XMLException{
    AttribNotFoundException(const std::string &text) throw() :XMLException(text){}
    ~AttribNotFoundException() throw() {};
  };

  /// Throw if an operation is not allowd on a certain node \ingroup XML
  /** e.g. set text to a container node or add childs to a comment node */
  struct InvalidNodeTypeException : public XMLException{
    InvalidNodeTypeException(const std::string &text) throw() :XMLException(text){}
    ~InvalidNodeTypeException() throw() {};
  };

  /// Thrown if a node is added to a node while being child of another node \ingroup XML
  struct DoubledReferenceException : public XMLException{
    DoubledReferenceException(const std::string &text) throw() :XMLException(text){}
    ~DoubledReferenceException() throw() {};
  };
  
  /// Thrown if operations are called on a null document \ingroup XML
  struct NullDocumentException : public XMLException{
    NullDocumentException(const std::string &text) throw() :XMLException(text){}
    ~NullDocumentException() throw() {};
  };


  /** \cond */
  class XMLDocument;
  class XMLNode;
  struct XMLNodeDelOp : public DelOpBase{ static void delete_func(XMLNode *n); };
  /** \endcond */

  /// Internal Smart XMLNode pointer type \ingroup XML
  typedef SmartPtr<XMLNode,XMLNodeDelOp> XMLNodePtr;

  /// Internal attribute list/map type \ingroup XML
  typedef std::map<std::string,std::string> XMLAttMap;
  
  /// Internal attribute list/map smart pointer type \ingroup XML
  typedef SmartPtr<XMLAttMap,PointerDelOp> XMLAttMapPtr;


  
}

#endif
