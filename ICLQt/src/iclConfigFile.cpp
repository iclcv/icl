#include "iclConfigFile.h"
#include <iclTypes.h>
#include <iclMacros.h>
#include <map>

#include <iclStringUtils.h>
//#include <QDomDocument>
//#include <QFile>
#include <iclSize.h>
#include <iclRect.h>
#include <iclPoint.h>
#include <iclRange.h>
//#include <QTextStream>
#include <list>
#include <vector>
#include <iclColor.h>
#include <iclException.h>
#include <iclSize32f.h>
#include <iclPoint32f.h>
#include <iclRect32f.h>

#include <iclXMLDocument.h>
#include <fstream>
using namespace std;

namespace icl{

  
  void XMLDocumentDelOp::delete_func(XMLDocument *h){
    ICL_DELETE(h);
  }


  namespace {
    // each add_function<T> defines how to transform XML's text 
    // element string into an element of type T

    // map of add functions (key = type-string, params = variable-ID
    // and variable XML-Text string representation
    typedef void (*add_func)(DataStore&,const std::string&,const std::string&);
    std::map<std::string,add_func> addFunctions;

    template<class T>
    void add_f(DataStore&,const std::string&, const std::string&){}
  }
  
  template<class T> 
  std::string ConfigFile::get_type_str(){ return "";}
  
#define REGISTER_TYPE(NR,T)                                             \
  template<>                                                            \
  std::string ConfigFile::get_type_str<T>(){ return #T; }               \
  namespace {                                                           \
    template<> void add_f<T>(DataStore &ds,                             \
                             const std::string &id,                     \
                             const std::string &value){                 \
      ds.allocValue("config."+id,icl::parse<T>(value));                 \
    }                                                                   \
    struct ADD_##NR{                                                    \
      ADD_##NR(){ addFunctions[#T] = add_f<T>; }                        \
    } add_##NR;                                                         \
  }
    
  REGISTER_TYPE(1,char);
  REGISTER_TYPE(2,unsigned char);
  REGISTER_TYPE(3,short);
  REGISTER_TYPE(4,unsigned short);
  REGISTER_TYPE(5,int);
  REGISTER_TYPE(6,unsigned int);
  REGISTER_TYPE(7,float);
  REGISTER_TYPE(8,double);
  REGISTER_TYPE(9,string);
  REGISTER_TYPE(10,Size);
  REGISTER_TYPE(11,Point);
  REGISTER_TYPE(12,Rect);
  REGISTER_TYPE(13,Size32f);
  REGISTER_TYPE(14,Point32f);
  REGISTER_TYPE(15,Rect32f);
  REGISTER_TYPE(16,Range32s);
  REGISTER_TYPE(17,Range32f);
  REGISTER_TYPE(18,Color);
  REGISTER_TYPE(19,long int);
  REGISTER_TYPE(20,bool);
  // }}}
  

    
  static void ds_push(DataStore &ds,const std::string &id, const std::string &type, const std::string &value){
    ds.lock();
    std::map<std::string,add_func>::iterator it = addFunctions.find(type);
    if(it == addFunctions.end()){
      throw ICLException(str(__FUNCTION__ )+ str(":unable to add ") + type + 
                         str(" to data store (") + str(__FILE__) + str(":") + str(__LINE__) + str(")"));
    }
    it->second(ds,id,value);
    ds.unlock();
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
      std::vector<XMLNode*> sn = n->getAllChildNodes(XMLNodeFilterByTag("section") &
                                                     XMLNodeFilterByAttrib("id",t[i]));
      switch(sn.size()){
        case 0:
          n->addSingleNode("section");
          n = &n->getLastChildNode();
          (*n)("id")=t[i];
          break;
        case 1:
          ICLASSERT_RETURN(!sn[0]->isTextNode());
          n = sn[0];
          break;
        default:
          ERROR_LOG("Warning more then one sub-path found for key \"" << name << "\" [using first!]");
          ICLASSERT_RETURN(!sn[0]->isTextNode());
          n = sn[0];
          break;
      }
    }

    std::vector<XMLNode*> sn = n->getAllChildNodes(XMLNodeFilterByType(XMLNode::TEXT) &
                                                   XMLNodeFilterByTag("data") &
                                                   XMLNodeFilterByAttrib("id",t.back()));
    switch(sn.size()){
      case 0:
        n->addTextNode("data",value);
        n->getLastChildNode()("type") = type;
        n->getLastChildNode()("id") = t.back();
        break;
      case 1:
        (*sn[0])("type") = type;
        sn[0]->setText(value);
        break;
      default:
        ERROR_LOG("Warning more then one sub-path found for key \"" << name << "\" (data tag doubled) [using first!]");
        (*sn[0])("type") = type;
        sn[0]->setText(value);
        break;
    }
    
  }
  


  


  void ConfigFile::load()throw(FileNotFoundException,InvalidFileFormatException){
    // {{{ open

    load(m_sFileName);
  }

  // }}}
  
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
    std::copy(l.begin(),--l.end(),std::ostream_iterator<std::string>(str,"."));
    str << l.back();
    return str.str();
  }

  void ConfigFile::load(const std::string &filename) throw(FileNotFoundException,InvalidFileFormatException){
    // {{{ open

    ICLASSERT_RETURN(filename != "");
    *m_doc = XMLDocument::load(filename);
    m_doc->removeAllComments();
    clear();
    const std::vector<XMLNode*> ns = m_doc->getRootNode()->getAllChildNodes( XMLNodeFilterByTag("data") &
                                                                             XMLNodeFilterByType(XMLNode::TEXT) &
                                                                             XMLNodeFilterByAttrib("id") &
                                                                             XMLNodeFilterByAttrib("type") );
    std::string pfx = m_sDefaultPrefix;
    if(pfx.length() && pfx[pfx.length()-1] != '.') pfx+='.';
    
    for(unsigned int i=0;i<ns.size();++i){
      const XMLNode &n = *ns[i];
      const std::string key = pfx+get_id_path(ns[i]);
      ds_push(*this,key,n("type"),n.getText());
      if(n.hasAttribute("range")){
        setRestriction("config."+key,n("range").as<Range64f>()); 
      }else if(n.hasAttribute("values")){
        std::string vl = n("values");
        setRestriction("config."+key,vl.substr(1,vl.size()-2));
      }
    }
    updateTitleFromDocument();
  }

  // }}}
  
  void ConfigFile::updateTitleFromDocument(){
    const std::vector<XMLNode*> ns = m_doc->getRootNode()->getAllChildNodes( XMLNodeFilterByLevel(1) &
                                                                                        XMLNodeFilterByPathSubstring("config.title") &
                                                                                        XMLNodeFilterByType(XMLNode::TEXT) );
    std::string title;
    switch(ns.size()){
      case 0:
        std::cout << "Warning no text node \"config.title\" found!" << std::endl;
        break;
      case 1:
        title = ns[0]->getText();
        break;
      default:
        std::cout << "Warning more than one text node \"config.title\" found [using first]!" << std::endl;
        title = ns[0]->getText();
        break;
    }
    if(contains("config.title")){
      getValue<std::string>("config.title") = title;
    }else{
      allocValue("config.title",title);
    }
  }
  
  ConfigFile::ConfigFile():
    // {{{ open
    
  
    m_doc(new XMLDocument("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
                          "<config>\n"
                          "   <title>no title defined\"</title>\n"
                          "</config>\n")),
    m_restrictions(new std::map<std::string,KeyRestriction>()){
    updateTitleFromDocument();
  }
  
  // }}}

  void ConfigFile::setRestriction(const std::string &id, const ConfigFile::KeyRestriction &r){
    // {{{ open
    // we need to add this restriction to the file ??
    (*m_restrictions.get())[id] = r;
  }

  // }}}
  
  const ConfigFile::KeyRestriction *ConfigFile::getRestriction(const std::string &id) const{
    // {{{ open
    std::map<std::string,KeyRestriction>::const_iterator it = m_restrictions->find(id);
    if(it != m_restrictions->end() ){
      return &it->second;
      //      return &(*m_restrictions.get())[id];
    }else{
      return 0;
    }
  }

  // }}}


  ConfigFile::ConfigFile(const std::string &filename)throw(FileNotFoundException,InvalidFileFormatException):
    // {{{ open
    m_doc(new XMLDocument),m_restrictions(new std::map<std::string,KeyRestriction>()){

    load(filename);    
  }

  // }}}
  
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

  void ConfigFile::setTitle(const std::string &title){
    // {{{ open
    std::vector<XMLNode*> ns = m_doc->getRootNode()->getAllChildNodes( XMLNodeFilterByLevel(1) &
                                                                                  XMLNodeFilterByPathSubstring("config.title") &
                                                                                  XMLNodeFilterByType(XMLNode::TEXT) );
    switch(ns.size()){
      case 0:
        m_doc->getRootNode()->addTextNode("title",title);
        break;
      case 1:
        ns[0]->setText(title);
        break;
      default:
        std::cout << "Warning more than one text node \"config.title\" found [using first]!" << std::endl;
        ns[0]->setText(title);
        break;
    }
    getValue<std::string>("config.title") = title;
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
  
  void ConfigFile::save() const {
    // {{{ open

    save(m_sFileName);
  }

  // }}}

  std::ostream &operator<<(ostream &s, const ConfigFile &cf){
    return s << *cf.m_doc;
  }

  /// Singelton object
  ConfigFile ConfigFile::s_oConfig;
  
}
