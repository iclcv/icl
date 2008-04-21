#include "iclConfigFile.h"
#include <iclTypes.h>
#include <iclMacros.h>
#include <map>

#include <iclStringUtils.h>
#include <QDomDocument>
#include <QFile>
#include <iclSize.h>
#include <iclRect.h>
#include <iclPoint.h>
#include <iclRange.h>
#include <QTextStream>
#include <list>
#include <vector>

using namespace std;

namespace icl{

  /// Singelton object
  ConfigFile ConfigFile::s_oConfig;
  
  class ConfigFile::XMLDocHandle : public QDomDocument{
  public:
    XMLDocHandle():QDomDocument(){
      // default initialization:
      /*
          <?xml version="1.0" encoding="ISO-8859-1"?>
          <config>
            <title>Configuration File for "object_detection3"</title>
          </config>
      */
      QString errMsg;
      int errLine(0),errCol(0);
      static const QString DEF_CONTENT = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
                                         "<config>\n"
                                         "   <title>no title defined\"</title>\n"
                                         "</config>\n";
      if(!setContent(DEF_CONTENT,&errMsg,&errLine,&errCol)){
        ERROR_LOG("unable to setup empty default document (" << errMsg.toLatin1().data() << 
                  ", line:" << errLine << " row:" << errCol << ")" );
      }
      
    }
  };
  
  void ConfigFile::XMLDocHandleDelOp::delete_func(XMLDocHandle *h){
    ICL_DELETE(h);
  }


  namespace{
    
    struct FileCloser{
      // {{{ open

      QFile *f;
      FileCloser(QFile *f):f(f){}
      ~FileCloser(){ f->close(); }
    };

    // }}}
    
    template<class T>
    void add_f(DataStore&,const std::string&, const std::string&){}

    template<class T> 
    std::string get_type_str(){ return "";}
    
    template<class T>
    std::string get_value_str(const T&){ return "";}

    // {{{ add_f specializations

    // each add_function<T> defines how to transform XML's text 
    // element string into an element of type T

#define SPECIALIZE(T,VAL,VAL_STR)                          \
    template<> void add_f<T>(DataStore &ds,             \
                             const std::string &id,        \
                             const std::string &value){    \
      ds.allocValue(string("config.")+id,VAL);             \
    }                                                      \
    template<> std::string get_type_str<T>(){              \
      return #T;                                           \
    }                                                      \
    template<> std::string get_value_str<T>(const T &val){ \
      return VAL_STR;                                      \
    }
    
    SPECIALIZE(char,value[0],string()+val);
    SPECIALIZE(unsigned char,to8u(value),str(val));
    SPECIALIZE(short,to16s(value),str(val));
    SPECIALIZE(unsigned short,(unsigned short)to32s(value),str(icl32s(val)));
    SPECIALIZE(int,to32s(value),str(val));
    SPECIALIZE(unsigned int,(unsigned int)to32s(value),str(val));
    SPECIALIZE(float,to32f(value),str(val));
    SPECIALIZE(double,to64f(value),str(val));
    SPECIALIZE(string,value,val);
    SPECIALIZE(Size,translateSize(value),translateSize(val));
    SPECIALIZE(Point,translatePoint(value),translatePoint(val));
    SPECIALIZE(Rect,translateRect(value),translateRect(val));
    SPECIALIZE(Range<int>,translateRange<int>(value),translateRange(val));
    SPECIALIZE(Range<float>,translateRange<float>(value),translateRange(val));
#undef SPECIALIZE


    // }}}
    
    // {{{ static map of add-functions

    // map of add functions (key = type-string, params = variable-ID
    // and variable XML-Text string representation
    typedef void (*add_func)(DataStore&,const std::string&,const std::string&);
    std::map<std::string,add_func> addFunctions;

    // }}}
    
    // {{{ AddFuncionInitializer struct

    // Utility struct once instantiated statically
    // to instantiate all add_function_templates "into"
    // the addFunctions map
    static struct AddFunctionsInitializer{
      AddFunctionsInitializer(){
#define ADD_TEMPL(X) addFunctions[(#X)] = add_f<X>; 
        ADD_TEMPL(char);
        ADD_TEMPL(unsigned char); 
        ADD_TEMPL(short); 
        ADD_TEMPL(unsigned short);
        ADD_TEMPL(int);
        ADD_TEMPL(unsigned int);
        ADD_TEMPL(float);
        ADD_TEMPL(double);
        ADD_TEMPL(string);
        ADD_TEMPL(Size);
        ADD_TEMPL(Rect);
        ADD_TEMPL(Range<int>);
        ADD_TEMPL(Range<float>);
        ADD_TEMPL(Point);
#undef ADD_TEMPL
      }
    } addFunctionInitializer;

    // }}}
    
    void dsPush(DataStore &ds,const QString &id, const QString &type, const QString &value){
      // {{{ open

    ds.lock();
    addFunctions[type.toLatin1().data()](ds,id.toLatin1().data(),value.toLatin1().data());
    ds.unlock();
  }

  // }}}
  
    // recursivly add given QDomNode to the data store
    // each new section id is added to the prefix delimited with 
    // a single '.' character
    void load_rec(DataStore &ds,QDomNode n, QString prefix){
      // {{{ open

      while(!n.isNull()) {
        if(n.isElement()){
          QDomElement e = n.toElement();
          if(e.tagName() == "data"){
            ICLASSERT_RETURN(e.hasAttribute("id"));
            ICLASSERT_RETURN(e.hasAttribute("type"));
            
            QDomNode t = e.firstChild();
            ICLASSERT_RETURN(t.isText());
            QString idStr = e.attribute("id");
            QString typeStr = e.attribute("type");
            dsPush(ds,prefix==""?idStr:prefix+"."+idStr,typeStr,t.toText().data());
          }else if(e.tagName() == "section"){
            ICLASSERT_RETURN(e.hasAttribute("id"));
            QString idStr = e.attribute("id");
            load_rec(ds,e.firstChild(),prefix==""?idStr:prefix+"."+idStr);
          }else if(e.tagName() == "title"){  
            QDomNode title = e.firstChild();
            ICLASSERT_RETURN(title.isText());
            dsPush(ds,"config.title","string",title.toText().data());
          }
        }
        n = n.nextSibling();
      }
    }

    // }}}

    typedef SmartPtr<QDomNode,PointerDelOp> QDomNodePtr;
    typedef std::list<std::string> TokenList;

    QDomNodePtr get_node_rec(QDomNode n, std::list<string> &parts){
      // {{{ open

      //  printf("get_node_rec called with this content:" );
      //for(std::list<string>::iterator it = parts.begin();it != parts.end();++it){
      //  printf((*it+".").c_str());
      //}
      //printf("\n");

      if(parts.size() == 1){
        while(!n.isNull()){
          if(n.isElement()){
            QDomElement e = n.toElement();
            if(e.tagName() == "data"){
              ICLASSERT_RETURN_VAL(e.hasAttribute("id"),0);
              ICLASSERT_RETURN_VAL(e.hasAttribute("type"),0);
              if(e.attribute("id") == QString(parts.front().c_str())){
                return new QDomNode(e.firstChild());
              }
            }else if(e.tagName() == "section"){
              ICLASSERT_RETURN_VAL(e.hasAttribute("id"),0);
              if(e.attribute("id") == QString(parts.front().c_str())){
                return new QDomNode(e.firstChild());
              }
            }else if(e.tagName() == "title"){
              QDomNode title = e.firstChild();
              ICLASSERT_RETURN_VAL(title.isText(),0);
              return new QDomNode(title);
            }
          }
          n = n.nextSibling();
        }
        return 0;
      }else{
        while(!n.isNull()) {
          if(n.isElement()){
            QDomElement e = n.toElement();
            if(e.tagName() == "section"){
              ICLASSERT_RETURN_VAL(e.hasAttribute("id"),0);
              if(e.attribute("id") == QString(parts.front().c_str())){
                parts.pop_front();
                return get_node_rec(e.firstChild(),parts);
              }
            }
          }
          n = n.nextSibling();
        }
        return 0;
      }
    }

    // }}}

    TokenList get_token_list(const std::string &name){
      // {{{ open

      std::vector<std::string> tokens = tok(name,".");
      ICLASSERT_RETURN_VAL(tokens.size()>1,std::list<std::string>());
      ICLASSERT_RETURN_VAL(tokens[0]=="config",std::list<std::string>());
      TokenList tokenList;

      std::copy(tokens.begin()+1,tokens.end(),back_inserter(tokenList));
      
      return tokenList;
    }

    // }}}

    QDomNodePtr get_node(QDomDocument &doc,const std::string &name){
      // {{{ open

      TokenList tokenList = get_token_list(name);
      ICLASSERT_RETURN_VAL(tokenList.size(),0);
      
      return get_node_rec(doc.documentElement().firstChild(),tokenList);
    }

    // }}}
    
    void add_to_doc_rec(QDomDocument &doc, QDomNode nIn, TokenList &parts, const std::string &type, const std::string &value){
      // {{{ open

      ICLASSERT_RETURN(!nIn.isNull());
      //printf("add_to_doc_rec called: front_elem is -%s- \n",parts.front().c_str());

      QDomNode n = nIn.firstChild();
      if(parts.size() == 1){
        // printf("parts size is 1 entry is %s \n",parts.front().c_str());
        while(!n.isNull()){
          if(n.isElement()){
            QDomElement e = n.toElement();
            if(e.tagName() == "data"){
              ICLASSERT_RETURN(e.hasAttribute("id"));
              ICLASSERT_RETURN(e.hasAttribute("type"));
              if(e.attribute("id") == QString(parts.front().c_str())){
                printf("warning overwriting entry \"%s\"\n",parts.front().c_str());
                if(e.attribute("type") != type.c_str()){
                  printf("warning type changes from %s to %s \n",e.attribute("type").toLatin1().data(),type.c_str());
                }
                e.setAttribute("type",type.c_str());
                QDomText t = e.firstChild().toText();
                ICLASSERT_RETURN(!t.isNull());
                t.setData(value.c_str());
                return;
              }
            }else if(e.tagName() == "section"){
              ICLASSERT_RETURN(e.hasAttribute("id"));
              if(e.attribute("id") == QString(parts.front().c_str())){
                ERROR_LOG("cannot add a value into a section directly (id:" << parts.front() << ")");
                return;
              }
            }else if(e.tagName() == "title" && parts.front() == "title"){
              ERROR_LOG("entry \"config.title\" cannot be set in this way: use setTitle instead!");
              return;
            }
          }
          n = n.nextSibling();
        }
        // ok entry was not found yet! -> create the new entry
        QDomElement newEntry = doc.createElement("data");
        newEntry.setAttribute("type",type.c_str());
        newEntry.setAttribute("id",parts.front().c_str());

        nIn.appendChild(newEntry);

        QDomText dataText = doc.createTextNode(value.c_str());
        nIn.lastChild().appendChild(dataText);    

        //        printf("this is the document now: -------\n%s\n---------\n",doc.toString().toLatin1().data());    
        return;
      }else{
        while(!n.isNull()) {
          
          if(n.isElement()){
            QDomElement e = n.toElement();
            //  printf("testing node with tag: -%s-\n",e.tagName().toLatin1().data());
            
            if(e.tagName() == "section"){
              ICLASSERT_RETURN(e.hasAttribute("id"));
             
              //printf("found  a section with id = -%s- \n",e.attribute("id").toLatin1().data());
              if(e.attribute("id") == QString(parts.front().c_str())){
                parts.pop_front();
                add_to_doc_rec(doc,e,parts,type,value);
                return;
              }
            }
          }
          n = n.nextSibling();
        }
        //printf("found no section: creating \n");
        QDomElement newEntry  = doc.createElement("section");
        newEntry.setAttribute("id",parts.front().c_str());
        nIn.appendChild(newEntry);

        //printf("this is the document now: -------\n%s\n---------\n",doc.toString().toLatin1().data());

        parts.pop_front();
        add_to_doc_rec(doc,newEntry,parts,type,value);
        return;
      }
      
    }

    // }}}
    
    void add_to_doc(QDomDocument &doc, const std::string &name, const std::string &type, const std::string &value){
      // {{{ open

      TokenList tokenList = get_token_list(name);
      ICLASSERT_RETURN(tokenList.size());
      
      add_to_doc_rec(doc,doc.documentElement(),tokenList,type,value);
      
    }

    // }}}
    
  }// end of anonymous namespace
  


  


  void ConfigFile::load(){
    // {{{ open

    load(m_sFileName);
  }

  // }}}
  
  void ConfigFile::load(const std::string &filename){
    // {{{ open

    ICLASSERT_RETURN(filename != "");

    QDomDocument doc(filename.c_str());
    QFile file(filename.c_str());
    FileCloser fc(&file);
    if(!file.open(QIODevice::ReadOnly)){
      m_sFileName = "";
      ERROR_LOG("Unable to open config file \"" << filename << "\"");
      return;
    }
    if(!doc.setContent(&file)){
      m_sFileName = "";
      ERROR_LOG("Unable parse document file (XML-Error) in \"" << filename << "\"");
      return;
    }
    
    m_sFileName = filename;
    static_cast<QDomDocument&>(*m_spXMLDocHandle) = doc;
    
    clear();


    
    load_rec(*this,doc.documentElement().firstChild(),"");


    updateTitleFromDocument();
  }

  // }}}
  
  void ConfigFile::updateTitleFromDocument(){
    QDomNodePtr titleDomNode = get_node(*m_spXMLDocHandle,"config.title");
    ICLASSERT_RETURN(titleDomNode);
    ICLASSERT_RETURN(titleDomNode->isText());
    
    QString title = titleDomNode->toText().data();
    
    if(contains("config.title")){
      getValue<std::string>("config.title") = title.toLatin1().data();
    }else{
      allocValue("config.title",std::string(title.toLatin1().data()));
    }
  }
  
  ConfigFile::ConfigFile():
    // {{{ open
    m_spXMLDocHandle(new XMLDocHandle){
        
    updateTitleFromDocument();
  }

  // }}}

  ConfigFile::ConfigFile(const std::string &filename):
    // {{{ open
    m_spXMLDocHandle(new XMLDocHandle){

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

    QDomNodePtr titleDomNode = get_node(*m_spXMLDocHandle,"config.title");
    ICLASSERT_RETURN(titleDomNode);
    ICLASSERT_RETURN(titleDomNode->isText());
    
    titleDomNode->toText().setData(title.c_str());
    
    getValue<std::string>("config.title") = title;
  }

  // }}}
  
  template<class T>
  void ConfigFile::add(const std::string &id, const T &val){
    if(contains(id) && !checkType<T>(id)){
      ERROR_LOG("id " << id << "is already set with different type!");
      return;
    }
      
    if(contains(id)){
      getValue<T>(id) = val;
    }else{
      allocValue<T>(id,val);
    }
    add_to_doc(*m_spXMLDocHandle,id,get_type_str<T>(),get_value_str<T>(val));
  }

#define X(T) template void ConfigFile::add<T>(const std::string&,const T&)
  X(char); X(unsigned char); X(short); X(unsigned short); X(int); X(unsigned int);
  X(float); X(double); X(string); X(Size); X(Point); X(Rect); X(Range<int>); X(Range<float>);
#undef X

  void ConfigFile::save(const std::string &filename) const{
    // {{{ open
    QFile file(filename.c_str());
    if(!file.open(QIODevice::WriteOnly)){
      ERROR_LOG("unable to write ConfigFile filename " << filename << "[aborting]");
      return;
    }
    QTextStream stream(&file);
    stream << static_cast<QDomNode&>(*m_spXMLDocHandle);
    file.close();
  }
  
  // }}}
  
  void ConfigFile::save() const {
    // {{{ open

    save(m_sFileName);
  }

  // }}}
}
