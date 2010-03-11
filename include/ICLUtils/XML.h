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
  typedef SmartPtrBase<XMLNode,XMLNodeDelOp> XMLNodePtr;

  /// Internal attribute list/map type \ingroup XML
  typedef std::map<std::string,std::string> XMLAttMap;
  
  /// Internal attribute list/map smart pointer type \ingroup XML
  typedef SmartPtr<XMLAttMap> XMLAttMapPtr;


  
}

#endif
