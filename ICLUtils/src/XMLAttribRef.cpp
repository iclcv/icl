/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLUtils/src/XMLAttribRef.cpp                          **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

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
