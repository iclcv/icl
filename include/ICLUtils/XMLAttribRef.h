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

#ifndef ICL_XML_ATTRIB_REF_H
#define ICL_XML_ATTRIB_REF_H

#include <ICLUtils/XML.h>
#include <ICLUtils/StringUtils.h>

namespace icl{
  
  
  /// Utility class that provides read and deferred write access to XMLNode attributes \ingroup XML
  /** Internally the XMLNode class stores attributes in a std::map<std::string,std::string>
      typed member variable. Unlike the std::map, which automatically creates an
      element for a new key, we want to defer creation of new attributes, until these
      new values are assigned. This behaviour is implemented in the XMLAttribRef utility class.
      
      \section CC Creation and Copies
      XMLAttribRef instances cannot be instantiated by the user, but the XNLNode::operator()
      returns valid XMLAttribRef instances. Furthermore, XMLAttribRef instances cannot be
      copied (private copy constructor), as this would enable the user to violate it's
      const concept.
      
      \section Constness
      The const version of XMLAttribRef does not allow to assign new values to the referenced
      XMLNode attribute, so the const version of the XMLNode::operator() returns also
      a const version of XMLAttribRef. In addition, the const version of XMLNode::operator()
      will not allow to reference attributes that do not exist.
  */
  class XMLAttribRef{ 
    
    /// Internal referenced attribute map
    XMLAttMapPtr m_map;
    
    /// id within m_map
    std::string m_id;
    
    /// private empty constructor
    XMLAttribRef();
    
    /// private copy constructor
    XMLAttribRef(const XMLAttribRef &other):m_map(other.m_map),m_id(other.m_id){}
    
    /// private 'real' constructor    
    XMLAttribRef(XMLAttMapPtr map, const std::string &id);
    
    public:
    /// For tight integration with XMLNode class
    friend class XMLNode;
    
    /// Assign a new attribute value (template internally using icl::str(..) function)
    /** Note: icl::str is defined in ICLUtils::iclStringUtils.h */
    template<class T>
    XMLAttribRef &operator=(const T &elem){
      return set(str(elem));
    }
    
    /// Implicit cast operator, casts to std::string
    operator std::string() const throw (AttribNotFoundException);
    

    /// Attribute value extraction function (internally using icl::parse<T>(..) function)
    /** Note: icl::parse<T> is defined in ICLUtils::iclStringUtils.h */
    template<class T>
    T as() const throw (AttribNotFoundException){
      return parse<T>( static_cast<std::string>(*this) );
    }

    /// Sets the attribute value to given std::string
    /** Actually this is just a utility function which is called from operator= */
    XMLAttribRef &set(const std::string &str);
  };
  
  /// std::ostream operator \ingroup XML
  std::ostream &operator<<(std::ostream &os, const XMLAttribRef &ar) throw (AttribNotFoundException);
}

#endif
