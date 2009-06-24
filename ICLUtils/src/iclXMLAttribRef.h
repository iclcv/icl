#ifndef ICL_XML_ATTRIB_REF_H
#define ICL_XML_ATTRIB_REF_H

#include <iclXML.h>
#include <iclStringUtils.h>

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
