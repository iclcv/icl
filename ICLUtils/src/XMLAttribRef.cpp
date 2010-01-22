#include <ICLUtils/XMLAttribRef.h>
#include <ICLUtils/XMLNode.h>

namespace icl{
  
  XMLAttribRef::XMLAttribRef(){}
  
  XMLAttribRef::XMLAttribRef(XMLAttMapPtr map, const std::string &id):
    m_map(map),m_id(id){}

  XMLAttribRef::operator std::string() const throw (AttribNotFoundException){
    XMLAttMap::const_iterator it = m_map->find(m_id);
    if(it == m_map->end()) throw AttribNotFoundException("Attribute " + str(m_id) + " not found");
    else return it->second;
  }
  
  XMLAttribRef &XMLAttribRef::set(const std::string &str){
    m_map->operator[](m_id) = ""+str+"";
    return *this;
  }

  std::ostream &operator<<(std::ostream &os, const XMLAttribRef &ar) throw (AttribNotFoundException){
    return os << static_cast<std::string>(ar);
  }

}
