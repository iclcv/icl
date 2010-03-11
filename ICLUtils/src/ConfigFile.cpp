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

#include <ICLUtils/ConfigFile.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/XMLDocument.h>

#include <fstream>
#include <list>
#include <vector>

#include <ICLUtils/Size.h>
#include <ICLUtils/Size32f.h>

#include <ICLUtils/Point.h>
#include <ICLUtils/Point32f.h>

#include <ICLUtils/Rect.h>
#include <ICLUtils/Rect32f.h>

#include <ICLUtils/Range.h>
#include <ICLUtils/SteppingRange.h>

#include <ICLUtils/FixedMatrix.h>
#include <ICLUtils/FixedVector.h>

using namespace std;

namespace icl{

  std::map<std::string,std::string> ConfigFile::s_typeMap;
  std::map<std::string,std::string> ConfigFile::s_typeMapReverse;
  
  void XMLDocumentDelOp::delete_func(XMLDocument *h){
    ICL_DELETE(h);
  }

  namespace{
    using std::string;
    
    struct StaticConfigFileTypeRegistering{
      
      StaticConfigFileTypeRegistering(){
        REGISTER_CONFIG_FILE_TYPE(char);
        REGISTER_CONFIG_FILE_TYPE(unsigned char);
        REGISTER_CONFIG_FILE_TYPE(short);
        REGISTER_CONFIG_FILE_TYPE(unsigned short);
        REGISTER_CONFIG_FILE_TYPE(int);
        REGISTER_CONFIG_FILE_TYPE(unsigned int);
        REGISTER_CONFIG_FILE_TYPE(float);
        REGISTER_CONFIG_FILE_TYPE(double);
        REGISTER_CONFIG_FILE_TYPE(string);
        REGISTER_CONFIG_FILE_TYPE(long int);
        REGISTER_CONFIG_FILE_TYPE(bool);

        REGISTER_CONFIG_FILE_TYPE(Size);
        REGISTER_CONFIG_FILE_TYPE(Point);
        REGISTER_CONFIG_FILE_TYPE(Rect);
        REGISTER_CONFIG_FILE_TYPE(Size32f);
        REGISTER_CONFIG_FILE_TYPE(Point32f);
        REGISTER_CONFIG_FILE_TYPE(Rect32f);
        REGISTER_CONFIG_FILE_TYPE(Range32s);
        REGISTER_CONFIG_FILE_TYPE(Range32f);
        REGISTER_CONFIG_FILE_TYPE(SteppingRange32s);
        REGISTER_CONFIG_FILE_TYPE(SteppingRange32f);
        //            REGISTER_CONFIG_FILE_TYPE(Color);

        typedef  FixedMatrix<float,3,3> Mat3x3;
        typedef  FixedMatrix<float,3,4> Mat3x4;
        typedef  FixedMatrix<float,4,3> Mat4x3;
        typedef  FixedMatrix<float,4,4> Mat4x4;

        REGISTER_CONFIG_FILE_TYPE(Mat3x3);
        REGISTER_CONFIG_FILE_TYPE(Mat3x4);
        REGISTER_CONFIG_FILE_TYPE(Mat4x3);
        REGISTER_CONFIG_FILE_TYPE(Mat4x4);

        typedef  FixedColVector<float,3> ColVec3;
        typedef  FixedColVector<float,4> ColVec4;

        typedef  FixedRowVector<float,3> RowVec3;
        typedef  FixedRowVector<float,4> RowVec4;

        REGISTER_CONFIG_FILE_TYPE(ColVec3);
        REGISTER_CONFIG_FILE_TYPE(ColVec4);
        REGISTER_CONFIG_FILE_TYPE(RowVec3);
        REGISTER_CONFIG_FILE_TYPE(RowVec4);
      }
    } StaticConfigFileTypeRegisteringIntance;
  }
    
  
  void ConfigFile::add_to_doc(XMLDocument &doc, 
                              const std::string &name, 
                              const std::string &type, 
                              const std::string &value){
    
    std::vector<std::string> t = tok(name,".");
    ICLASSERT_RETURN(t.size()>1);
    ICLASSERT_RETURN(t[0]=="config");
    XMLNode* n = doc.getRootNode().get();
    for(unsigned int i=1;i<t.size()-1;++i){
      std::vector<XMLNode*> sn = n->getAllChildNodes(XMLNodeFilterByType(XMLNode::NODE) & 
                                                     XMLNodeFilterByTag("section") &
                                                     XMLNodeFilterByAttrib("id",t[i]));
      switch(sn.size()){
        case 0:
          n->addNode("section");
          n = &n->getLastChildNode();
          (*n)("id")=t[i];
          break;
        case 1:
          n = sn[0];
          break;
        default:
          ERROR_LOG("Warning more then one sub-path found for key \"" << name << "\" [using first!]");
          n = sn[0];
          break;
      }
    }

    std::vector<XMLNode*> sn = n->getAllChildNodes(XMLNodeFilterByType(XMLNode::NODE) &
                                                   XMLNodeFilterByTag("data") &
                                                   XMLNodeFilterByAttrib("id",t.back()));
    XMLNode *data = 0;
    switch(sn.size()){
      case 0:
        n->addNodeWithTextContent("data",value);
        n->getLastChildNode()("type") = type;
        n->getLastChildNode()("id") = t.back();
        break;
      case 1:
        (*sn[0])("type") = type;
        data = sn[0];
        break;
      default:
        ERROR_LOG("Warning more than one sub-path found for key \"" << name << "\" (data tag doubled) [using first!]");
        (*sn[0])("type") = type;
        data = sn[0];
        break;
    }
    if(data){
      if(!data->hasChildren()){
        data->addText(value);
        return;
      }
      std::vector<XMLNode*> sn = data->getAllChildNodes(XMLNodeFilterByType(XMLNode::TEXT));
      switch(sn.size()){
        case 0:
          data->addText(value,0);
          break;
        case 1:
          (*sn[0]) = value;
          break;
        default:
          ERROR_LOG("Warning more than one (" << sn.size() << ") text sub-node found in node " + data->toString(false) + "[using first!]");
          (*sn[0]) = value;
          break;
      }
    }
    
  }
  
  static std::string get_id_path(const XMLNode *n){
    std::list<std::string> l;
    try{
      while(n->getParent()){
        l.push_front((*n)("id"));
        n = n->getParent();
      }
    }catch(XMLException &ex){
      ERROR_LOG("unable to estimate ID path: (found subnode without id attribute)" << ex.what());
    }
    std::ostringstream str;
    str << "config.";
    std::copy(l.begin(),--l.end(),std::ostream_iterator<std::string>(str,"."));
    str << l.back();
    return str.str();
  }

  void ConfigFile::load_internal(){
    m_doc->removeAllComments();
    m_entries.clear();
    const std::vector<XMLNode*> ns = m_doc->getRootNode()->getAllChildNodes( XMLNodeFilterByTag("data") &
                                                                             XMLNodeFilterByType(XMLNode::NODE) &
                                                                             XMLNodeFilterByAttrib("id") &
                                                                             XMLNodeFilterByAttrib("type") &
                                                                             XMLNodeFilterByHasAnyChildOfType(XMLNode::TEXT));
    std::string pfx = m_sDefaultPrefix;
    if(pfx.length() && pfx[pfx.length()-1] != '.') pfx+='.';
    
    for(unsigned int i=0;i<ns.size();++i){
      const XMLNode &n = *ns[i];

      const std::string key = pfx+get_id_path(ns[i]);

      /// add data to config file
      if(contains(key)) throw InvalidFileFormatException("Key: '" + key + "' was found at least twice!");
      Entry &e = m_entries[key];
      e.parent = this;
      e.id = key;
      e.value = n.getFirstChildNode(XMLNode::TEXT).getText();
      std::map<std::string,std::string>::const_iterator it = s_typeMapReverse.find(n("type"));
      if(it == s_typeMapReverse.end()) throw UnregisteredTypeException(n("type"));
      e.rttiType = it->second;
      
      
      if(n.hasAttribute("range")){
        std::string mm = n("range");
        if(mm.size()<5 || mm[0]!='[' || mm[mm.length()-1] != ']') throw InvalidFileFormatException("unable to parse range attribute: syntax [min,max]");
        std::vector<double> mmv = parseVecStr<double>(mm.substr(1,mm.size()-2),",");
        if(mmv.size() != 2) throw InvalidFileFormatException("unable to parse range attribute: syntax [min,max]");
        setRestriction(key,KeyRestriction(mmv[0],mmv[1])); 
      }else if(n.hasAttribute("values")){
        std::string vl = n("values");
        setRestriction(key,vl.substr(1,vl.size()-2));
      }
    }
  }
  
  void ConfigFile::load(const std::string &filename) throw(FileNotFoundException,InvalidFileFormatException,UnregisteredTypeException){
    // {{{ open

    ICLASSERT_RETURN(filename != "");
    *m_doc = XMLDocument::load(filename);
    
    load_internal();
  }

  // }}}
  
  
  ConfigFile::ConfigFile():
    // {{{ open
    
  
    m_doc(new XMLDocument("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
                          "<config>\n"
                          "   <data id=\"title\" type=\"string\">no title defined</data>\n"
                          "</config>\n")){
  }
  
  // }}}

  void ConfigFile::setRestriction(const std::string &id, const ConfigFile::KeyRestriction &r) throw (EntryNotFoundException){
    // {{{ open
    // we need to add this restriction to the file ??
    get_entry_internal(m_sDefaultPrefix+id).restr = SmartPtr<KeyRestriction>(new KeyRestriction(r));
  }

  // }}}
  
  const ConfigFile::KeyRestriction *ConfigFile::getRestriction(const std::string &id) const throw (EntryNotFoundException){
    // {{{ open
    return get_entry_internal(m_sDefaultPrefix+id).restr.get();
  }

  // }}}


  ConfigFile::ConfigFile(const std::string &filename)throw(FileNotFoundException,InvalidFileFormatException,UnregisteredTypeException):
    // {{{ open
    m_doc(new XMLDocument){
    
    load(filename);    
  }

  // }}}
  
  ConfigFile::ConfigFile(XMLDocument *handle) throw (UnregisteredTypeException):
    m_doc(handle){
    load_internal();
  }
  
  void ConfigFile::loadConfig(const std::string &filename){
    // {{{ open

    s_oConfig.load(filename);
  }

  // }}}
  
  void ConfigFile::loadConfig(const ConfigFile &configFile){ 
    // {{{ open

    s_oConfig = configFile;
  }

  // }}}

  
 
  void ConfigFile::save(const std::string &filename) const{
    // {{{ open
    try{
      XMLDocument::save(*m_doc,filename);
    }catch(const ICLException &e){
      ERROR_LOG(e.what());
    }
  }
  
  // }}}

  void ConfigFile::clear(){
    *this = ConfigFile();
  }

  bool ConfigFile::check_type_internal(const std::string &id, const std::string &rttiTypeID) const throw (EntryNotFoundException,UnregisteredTypeException){
    const Entry &e = get_entry_internal(m_sDefaultPrefix+id);
    return e.rttiType == rttiTypeID;
    /*    
        DEBUG_LOG("rtti type is " << rttiTypeID << "#registered types:" << s_typeMap.size());
        std::map<std::string,std::string>::const_iterator it=s_typeMap.find(rttiTypeID);
        if(it == s_typeMap.end()) throw UnregisteredTypeException(rttiTypeID);
        return (it->second == e.rttiType);
    */
  }

  ConfigFile::Entry &ConfigFile::get_entry_internal(const std::string &id) throw (EntryNotFoundException){
    std::map<std::string,Entry>::iterator it = m_entries.find(id);
    if(it == m_entries.end()) throw EntryNotFoundException(id);
    else return it->second;
  }
  const ConfigFile::Entry &ConfigFile::get_entry_internal(const std::string &id) const throw (EntryNotFoundException){
    return const_cast<ConfigFile*>(this)->get_entry_internal(id);
  }
  

  
  void ConfigFile::set_internal(const std::string &idIn, const std::string &val, const std::string &type) 
    throw (UnregisteredTypeException){
    std::map<std::string,std::string>::iterator it = s_typeMap.find(type);
    if(it == s_typeMap.end()) throw UnregisteredTypeException(type);
    std::string id=m_sDefaultPrefix + idIn;  
    Entry &e = m_entries[id];
    e.parent = this;
    e.id = id;
    e.rttiType = type;
    e.value = val;
    add_to_doc(*m_doc,id,it->second,val);
  }    

  void ConfigFile::listContents() const{
    std::cout << "config file entries:" << std::endl;
    for(std::map<std::string,Entry>::const_iterator it = m_entries.begin();
        it != m_entries.end(); ++it){
      std::cout << (it->second.id) << "\t" << (it->second.value) << "\t" << (it->second.rttiType) << "\t";
      if(it->second.restr) std::cout << "(with restriction)" << std::endl;
      else std::cout << "(no restriction)" << std::endl;
    }
  }
  
  bool ConfigFile::contains(const std::string &id) const{
    if(m_sDefaultPrefix.length()){
      return (m_entries.find(m_sDefaultPrefix+id) != m_entries.end());
    }else{
      return (m_entries.find(id) != m_entries.end());
    }
  }

  ConfigFile::Data::Data(const std::string &id, ConfigFile &cf):
    id(id),cf(cf){}
  

  void ConfigFile::setPrefix(const std::string &defaultPrefix) const{ 
    m_sDefaultPrefix = defaultPrefix; 
  }
  
  const std::string &ConfigFile::getPrefix() const { 
    return m_sDefaultPrefix; 
  }

  ConfigFile::Data ConfigFile::operator [](const std::string &id){ 
    return Data(id,*this); 
  }
  
  const ConfigFile::Data ConfigFile::operator[](const std::string &id) const throw (EntryNotFoundException){
    if(!contains(id)){
      throw EntryNotFoundException(m_sDefaultPrefix+id);
    }
    return Data(id,const_cast<ConfigFile&>(*this));
  }
    


  std::ostream &operator<<(ostream &s, const ConfigFile &cf){
    return s << *cf.m_doc;
  }

  /// Singelton object
  ConfigFile ConfigFile::s_oConfig;

  
}
