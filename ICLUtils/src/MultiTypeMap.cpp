/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLUtils/src/MultiTypeMap.cpp                          **
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

#include <ICLUtils/MultiTypeMap.h>
#include <cstdio>


namespace icl{

  MultiTypeMap::MultiTypeMap(){
    m_oDataMapPtr = SmartDataMapPtr(new DataMap);
    m_oMutexPtr = SmartMutexPtr(new Mutex);
  }
  
  /// Destructor (deletes all remaining data)
  MultiTypeMap::~MultiTypeMap(){
    if(m_oDataMapPtr.use_count() == 1){
      for(DataMap::iterator it = m_oDataMapPtr->begin(); it != m_oDataMapPtr->end(); ++it){
        DataArray &da = it->second;
        da.release_func(&da);
      }
      m_oDataMapPtr->clear();
    }
  }


  std::vector<MultiTypeMap::Entry> MultiTypeMap::getEntryList() const{
    std::vector<Entry> es;
    for(DataMap::const_iterator it = m_oDataMapPtr->begin(); 
        it != m_oDataMapPtr->end(); ++it){
      es.push_back(Entry(it->first,it->second.type,it->second.len));
    }
    return es;
  }

  void MultiTypeMap::listContents() const{
    printf("MultiTypeMap content: \n");
    int i=0;
    for(DataMap::const_iterator it=m_oDataMapPtr->begin(); it != m_oDataMapPtr->end(); ++it){
      printf("%d: name:\"%s\" type:\"%s\"  arraylen:\"%d\" \n",i++,it->first.c_str(),it->second.type.c_str(),it->second.len);
    }
    printf("----------------------\n");
  }

  void MultiTypeMap::clear(){
    *this = MultiTypeMap();
  }


  const std::string &MultiTypeMap::getType(const std::string &id) const{
    if(!contains(id)){
      ERROR_LOG("id "<<  id  << " not found \n");
      static const std::string _NULL = "null";
      return _NULL;
    }
    return (*m_oDataMapPtr)[id].type;
  }

  bool MultiTypeMap::isArray(const std::string &id) const{
    if(!contains(id)){
      ERROR_LOG("id "<<  id  << " not found \n");
      return false;
    }
    return (*m_oDataMapPtr)[id].len != 0; 
  }


  bool MultiTypeMap::contains(const std::string &id) const{
    return m_oDataMapPtr->find(id) != m_oDataMapPtr->end();
  }

  bool MultiTypeMap::check_type_internal(const std::string &id, const std::string &typestr) const{
    if(!contains(id)){
      ERROR_LOG("id "<<  id  << " not found \n");
      return false;
    }
    return (*m_oDataMapPtr)[id].type == typestr;
  }

}
