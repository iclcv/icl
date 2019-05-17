/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/ConfigFile.cpp                   **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLUtils/ConfigFile.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/XML.h>

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

#include <ICLUtils/Any.h>

using namespace std;

namespace icl{
  namespace utils{

    ConfigFile::Maps *ConfigFile::getMapsInstance(){
      static SmartPtr<Maps> typeMap = new Maps;
      return typeMap.get();
    }
    //    std::map<std::string,std::string> ConfigFile::s_typeMap;
    //std::map<std::string,std::string> ConfigFile::s_typeMapReverse;

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
          REGISTER_CONFIG_FILE_TYPE(Any);
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

          // todo: this needs to be done in static initialization of the ICLMath library
          // we need to use a real singelton typemap then
          /*
          //typedef  FixedMatrix<float,3,3> Mat3x3;
          //typedef  FixedMatrix<float,3,4> Mat3x4;
          //typedef  FixedMatrix<float,4,3> Mat4x3;
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
         */
        }
      } StaticConfigFileTypeRegisteringIntance;
    }

    std::string ConfigFile::KeyRestriction::toString() const{
      if(hasRange){
        return str(Range32f(min,max));
      }else if (hasValues){
        return "["+values+"]";
      }else{
        return "invalid key restriction";
      }
    }

    static inline bool is_text_node(const XMLNode &node){
      return node && node.type() == pugi::node_pcdata;
    }

    void ConfigFile::add_to_doc(XMLDocument &doc,
                                const std::string &name,
                                const std::string &type,
                                const std::string &value,
                                const ConfigFile::KeyRestriction *restr){
      std::vector<std::string> t = tok(name,".");
      ICLASSERT_RETURN(t.size()>1);
      ICLASSERT_RETURN(t[0]=="config");

      XMLNode n = doc.document_element();

      for(unsigned int i=1;i<t.size()-1;++i){
        XMLNode sn = n.find_child_by_attribute("section","id",t[i].c_str());
        if(!sn){
          n = n.append_child("section");
          XMLAttribute id = n.append_attribute("id");
          id.set_value(t[i].c_str());
        }else{
          n = sn;
        }
      }

      XMLNode data = n.find_child_by_attribute("data","id",t.back().c_str());
      if(!data){
        data = n.append_child("data");
        XMLAttribute id = data.append_attribute("id");
        id.set_value(t.back().c_str());
      }

      XMLAttribute typeAtt = data.attribute("type");
      if(!typeAtt){
        typeAtt = data.append_attribute("type");
      }
      typeAtt.set_value(type.c_str());

      XMLNode text;
      if( (text = data.find_child(is_text_node)) ){
        text.set_value(value.c_str());
      }else{
        data.append_child(pugi::node_pcdata).set_value(value.c_str());
      }

      if(restr){
        XMLAttribute rangeAtt = data.attribute("range");
        XMLAttribute valuesAtt = data.attribute("values");
        if(restr->hasRange){
          if(valuesAtt){
            data.remove_attribute(valuesAtt);
          }
          if(!rangeAtt) rangeAtt = data.append_attribute("range");
          rangeAtt.set_value(restr->toString().c_str());
        }else if(restr->hasValues){
          if(rangeAtt){
            data.remove_attribute(rangeAtt);
          }
          if(!valuesAtt) valuesAtt = data.append_attribute("values");
          valuesAtt.set_value(restr->toString().c_str());
        }else{
          WARNING_LOG("NULL KeyRestriction detected (this is ignored)");
        }
      }
    }

    static std::string get_id_path(XMLNode n){
      std::list<std::string> l;

      while(n.parent()){
        l.push_front(n.attribute("id").value());
        n = n.parent();
      }

      std::ostringstream str;
      str << "config";
      std::copy(l.begin(),--l.end(),std::ostream_iterator<std::string>(str,"."));
      str << l.back();
      return str.str();
    }

    void ConfigFile::load_internal(){
      //m_doc->removeAllComments();
      m_entries.clear();


      static const XPathQuery query("//data[@id and @type]");
      XPathNodeSet ds = m_doc->document_element().select_nodes(query);

      std::string pfx = m_sDefaultPrefix;
      if(pfx.length() && pfx[pfx.length()-1] != '.') pfx+='.';

      for(XPathNodeSet::const_iterator it = ds.begin(); it != ds.end(); ++it){
        XMLNode n = it->node();
        if(!n) throw ICLException("XML node expected, but attribute found??");

        const std::string key = pfx+get_id_path(n);

        //      DEBUG_LOG("adding key " << key);

        if(contains(key)) throw InvalidFileFormatException("Key: '" + key + "' was found at least twice!");

        Entry &e = m_entries[key];
        e.parent = this;
        e.id = key;
        e.value = n.first_child().value();
        Maps &maps  = getMapsInstanceRef();
        std::map<std::string,std::string>::const_iterator jt = maps.typeMapReverse.find(n.attribute("type").value());
        if(jt == maps.typeMapReverse.end()) throw UnregisteredTypeException(n.attribute("type").value());
        e.rttiType = jt->second;

        XMLAttribute rangeAtt = n.attribute("range");

        if(rangeAtt){
          std::string mm = rangeAtt.value();
          if(mm.size()<5 || mm[0]!='[' || mm[mm.length()-1] != ']'){
            throw InvalidFileFormatException("unable to parse range attribute: syntax [min,max]");
          }
          std::vector<double> mmv = parseVecStr<double>(mm.substr(1,mm.size()-2),",");
          if(mmv.size() != 2){
            throw InvalidFileFormatException("unable to parse range attribute: syntax [min,max]");
          }
          setRestriction(key,KeyRestriction(mmv[0],mmv[1]));
          continue;
        }

        XMLAttribute valuesAtt = n.attribute("values");
        if(valuesAtt){
          std::string vl = valuesAtt.value();
          setRestriction(key,vl.substr(1,vl.size()-2));
        }
      }
    }

    void ConfigFile::load(const std::string &filename) {
      // {{{ open

      m_doc = SmartPtrBase<XMLDocument,XMLDocumentDelOp>(new XMLDocument);
      pugi::xml_parse_result res = m_doc->load_file(filename.c_str());
      if(res.status != pugi::status_ok){
        if(res.status == pugi::status_file_not_found){
          throw FileNotFoundException(filename);
        }else{
          throw InvalidFileFormatException(res.description());
        }
      }


      load_internal();
    }

    // }}}


    ConfigFile::ConfigFile(){
      // {{{ open

      m_doc = SmartPtrBase<XMLDocument,XMLDocumentDelOp>(new XMLDocument);

      m_doc->load("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
                  "<config>\n"
                  "</config>\n");
    }

    // }}}

    ConfigFile::ConfigFile(std::istream &stream) {

      XMLDocument *doc = new XMLDocument;
      doc->loadNext(stream);
      *this = ConfigFile(doc);
    }


    void ConfigFile::setRestriction(const std::string &id, const ConfigFile::KeyRestriction &r) {
      // {{{ open
      get_entry_internal(m_sDefaultPrefix+id).restr = SmartPtr<KeyRestriction>(new KeyRestriction(r));

      // Search the corresponding node ...
    }

    // }}}

    const ConfigFile::KeyRestriction *ConfigFile::getRestriction(const std::string &id) const {
      // {{{ open
      return get_entry_internal(m_sDefaultPrefix+id).restr.get();
    }

    // }}}


    ConfigFile::ConfigFile(const std::string &filename){
      // {{{ open
      load(filename);
    }

    // }}}

    ConfigFile::ConfigFile(XMLDocument *handle) :
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
	  if (!m_doc->save_file(filename.c_str())) {
		  throw ICLException("failed to open file");
	  }
    }

    // }}}

    void ConfigFile::clear(){
      *this = ConfigFile();
    }

    bool ConfigFile::check_type_internal(const std::string &id, const std::string &rttiTypeID) const {
      const Entry &e = get_entry_internal(m_sDefaultPrefix+id);
      return e.rttiType == rttiTypeID;
      /*
          DEBUG_LOG("rtti type is " << rttiTypeID << "#registered types:" << s_typeMap.size());
          std::map<std::string,std::string>::const_iterator it=s_typeMap.find(rttiTypeID);
          if(it == s_typeMap.end()) throw UnregisteredTypeException(rttiTypeID);
          return (it->second == e.rttiType);
      */
    }

    ConfigFile::Entry &ConfigFile::get_entry_internal(const std::string &id) {
      std::map<std::string,Entry>::iterator it = m_entries.find(id);
      if(it == m_entries.end()) throw EntryNotFoundException(id);
      else return it->second;
    }
    const ConfigFile::Entry &ConfigFile::get_entry_internal(const std::string &id) const {
      return const_cast<ConfigFile*>(this)->get_entry_internal(id);
    }



    void ConfigFile::set_internal(const std::string &idIn, const std::string &val, const std::string &type)
      {
      Maps &maps = getMapsInstanceRef();
      std::map<std::string,std::string>::iterator it = maps.typeMap.find(type);
      if(it == maps.typeMap.end()) throw UnregisteredTypeException(type);
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
        if(it->second.restr) std::cout << "(restriction: " << it->second.restr->toString() << ")" <<  std::endl;
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
      id(id),cf(&cf){}


    void ConfigFile::setPrefix(const std::string &defaultPrefix) const{
      m_sDefaultPrefix = defaultPrefix;
    }

    const std::string &ConfigFile::getPrefix() const {
      return m_sDefaultPrefix;
    }

    ConfigFile::Data ConfigFile::operator [](const std::string &id){
      return Data(id,*this);
    }

    const ConfigFile::Data ConfigFile::operator[](const std::string &id) const {
      if(!contains(id)){
        throw EntryNotFoundException(m_sDefaultPrefix+id);
      }
      return Data(id,const_cast<ConfigFile&>(*this));
    }



    std::ostream &operator<<(ostream &s, const ConfigFile &cf){
      cf.m_doc->save(s);
      return s;
    }

    /// Singelton object
    ConfigFile ConfigFile::s_oConfig;


    std::vector<ConfigFile::Data> ConfigFile::find(const std::string &regex){
      std::vector<Data> ret;
      for(std::map<std::string,Entry>::const_iterator it = m_entries.begin();
          it != m_entries.end(); ++it){
        if(match(it->first, regex)){
          ret.push_back(Data(it->first,*this));
        }
      }
      return ret;
    }
  } // namespace utils
}
