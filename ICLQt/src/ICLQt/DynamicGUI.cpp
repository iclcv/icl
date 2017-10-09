/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DynamicGUI.cpp                         **
** Module : ICLQt                                                  **
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

#include <ICLQt/DynamicGUI.h>
#include <ICLUtils/PugiXML.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/StringUtils.h>
#include <fstream>
#include <sstream>
#include <set>
#include <deque>

using namespace pugi;
namespace icl{

  using namespace utils;

  namespace qt{

    struct DynamicGUI::Data{
      DynamicGUI::ParseTreePtr parseTree;
      Data(){}
    };

    DynamicGUI::DynamicGUI(const std::string &cfgFileName, QWidget *parent):
      GUI("vbox",parent), m_data(new Data){
      if(cfgFileName.length()){
        load(cfgFileName);
      }
    }

    DynamicGUI::~DynamicGUI(){
      release();
      delete m_data;
    }

    DynamicGUI::Node::Node(const std::string &name, SmartPtr<Node> parent, int level):
      name(name),parent(parent),level(level){
    }


    SmartPtr<DynamicGUI::Node> DynamicGUI::Node::appendChild(const std::string &name, SmartPtr<Node> parent, int level){
      children.push_back(SmartPtr<Node>(new Node(name,parent,level)));
      return children.back();
    }

    void DynamicGUI::Node::grabArgs(const pugi::xml_node &n){
      for(xml_attribute_iterator it = n.attributes_begin();
          it != n.attributes_end();++it){
        args.push_back(std::pair<std::string,std::string>((*it).name(), (*it).value()));
        if(args.back().first == "args"){
          directArgs = args.back().second;
          args.pop_back();
        }
      }
    }

    bool DynamicGUI::Node::isContainer() const {
      static std::set<std::string> cs;
      if(!cs.size()){
        std::vector<std::string> ts = tok("hbox,vbox,hsplit,vsplit,hscroll,vscroll,tab,border",",");
        cs = std::set<std::string>(ts.begin(), ts.end());
      }
      return cs.count(name);
    }

    bool DynamicGUI::Node::hasHandle() const{
      return getHandleName().length();
    }
    std::string DynamicGUI::Node::getHandleName() const{
      for(size_t i=0;i<args.size();++i){
        if(args[i].first == "handle") return args[i].second;
      }
      return "";
    }

    std::ostream &operator<<(std::ostream &s, const DynamicGUI::Node &n){
      s << std::string(n.level*2,' ') << "<" << n.name;
      std::deque<std::pair<std::string,std::string> > args(n.args.begin(),n.args.end());
      if(n.directArgs.length()){
        args.push_front(std::pair<std::string,std::string>("args",n.directArgs));
      }

      if(args.size()) s << " ";
      for(int i=0;i<(int)args.size();++i){
        s << args[i].first << "=\"" << args[i].second << "\"";
        if(i < (int)args.size()-1){
          s << " ";
        }
      }
      if(n.children.size()){
        s << ">" << std::endl;
        for(size_t i=0;i<n.children.size();++i){
          s << *n.children[i];
        }
        s << std::string(n.level*2,' ') << "</" << n.name << ">" << std::endl;
      }else{
        s << "/>" << std::endl;
      }
      return s;
    }

    void DynamicGUI::traverse_tree(const xml_node &n, int level, SmartPtr<DynamicGUI::Node> target){
      std::string name = n.name();
      target->name = name;
      target->grabArgs(n);

      for(xml_node_iterator it = n.begin(); it != n.end();++it){
        traverse_tree(*it, level+1, target->appendChild(name,target,level+1));
      }
    }

    void DynamicGUI::create_gui(Node &n){
      if(n.name == "include" || n.name == "embed"){
        // first and only arg must be href
        if(n.args.size() != 1 ||
           ( n.args[0].first != "href" && n.args[0].first != "file")){
          throw ICLException("error in <include ..> -tag href|file missing!");
        }
        n.parsedGUI = DynamicGUI(n.args[0].second);
        return;
      }

      if(n.isContainer()){
        for(size_t i=0;i<n.children.size();++i){
          create_gui(*n.children[i]);
        }
      }
      std::ostringstream def;
      def << n.name;
      if(n.directArgs.length()){
        def << "(" << n.directArgs << ")";
      }
      if(n.args.size()){
        def << "[";
        for(size_t i=0;i<n.args.size();++i){
          def << "@" << n.args[i].first << "=" << n.args[i].second;
        }
        def << "]";
      }
      n.parsedGUI = GUI::create_gui_from_string(def.str(),0);

      for(size_t i=0;i<n.children.size();++i){
        n.parsedGUI << n.children[i]->parsedGUI;
      }
    }

    void DynamicGUI::initialize(const std::string &cfgXMLString){
      xml_document doc;
      xml_parse_result r = doc.load_buffer(cfgXMLString.c_str(), cfgXMLString.size());
      if(!r){
        throw ICLException("error parsing input string:" + str(r.description()));
      }
      // traverse xml-dom-tree
      xml_node root = doc.document_element();
      initInternal(root);
    }

    void DynamicGUI::load(const std::string &cfgFileName){
      xml_document doc;
      xml_parse_result r = doc.load_file(cfgFileName.c_str());
      if(!r){
        throw ICLException("error parsing input xml-file:" + str(r.description()));
      }

      // traverse xml-dom-tree
      xml_node root = doc.document_element();
      initInternal(root);
    }

    void DynamicGUI::initInternal(pugi::xml_node &root){
      release();

      m_data->parseTree = new Node(root.name());
      traverse_tree(root, 0, m_data->parseTree);
      //    SHOW(*target);
      create_gui(*m_data->parseTree);
      *this << m_data->parseTree->parsedGUI;
    }

    DynamicGUI::ParseTreePtr DynamicGUI::getParseTree() {
      return m_data->parseTree;
    }

    void DynamicGUI::release(){
      // not implemented properly
    }

  }

}
