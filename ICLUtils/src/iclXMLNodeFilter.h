#ifndef ICL_XML_NODE_FILTER_H
#define ICL_XML_NODE_FILTER_H

#include <iclXML.h>

namespace icl{
  
  /// Utility interface to filter nodes  \ingroup XML
  /** XMLNodeFilter instances can be passed to XMLNode's getAllChlidNodes() functions
      The bool operator()(const XMLNode&) operator has to be defined in subclasses. Nodes 
      for which this function returns false, are filtered out.
      
      The default implementation is called XMLNodeFilterAll, which returns always true.
  */
  class XMLNodeFilter{
    public:
    /// filter the node here
    virtual bool operator()(const XMLNode &node) const= 0;
    virtual ~XMLNodeFilter(){}
    virtual XMLNodeFilter *deepCopy() const = 0;
  };
  
  
  /// Default implementation for XMLNodeFilter interface, which lets all nodes pass through  \ingroup XML
  class XMLNodeFilterAll : public XMLNodeFilter{
    public:
    /// returns always true
    virtual bool operator()(const XMLNode&) const{ return true; }
    
    /// deep copy function
    virtual XMLNodeFilter *deepCopy() const{ return new XMLNodeFilterAll; }
  };

  
  /// Can be used to filter nodes by Types \ingroup XML
  class XMLNodeFilterByType : public XMLNodeFilter{
    public:
    int nodeTypes; //<! internal node type accumulator

    /// Simply path XMLNode::Type enums here using | concatenation
    XMLNodeFilterByType(int nodeTypes):nodeTypes(nodeTypes){}
    
    /// returns always true
    virtual bool operator()(const XMLNode&) const;

    /// deep copy function
    virtual XMLNodeFilter *deepCopy() const{ return new XMLNodeFilterByType(nodeTypes); }
  };

  /// Can be used to filter nodes by Types \ingroup XML
  class XMLNodeFilterByTag : public XMLNodeFilter{
    public:
    std::string tag; //<! internal tag variable
    
    /// Create with given tag name
    XMLNodeFilterByTag(const std::string &tag):tag(tag){}
    
    /// returns always true
    virtual bool operator()(const XMLNode&) const;

    /// deep copy function
    virtual XMLNodeFilter *deepCopy() const{ return new XMLNodeFilterByTag(tag); }

  };

  /// Can be used to filter nodes by attributes \ingroup XML
  class XMLNodeFilterByAttrib : public XMLNodeFilter{
    public:
    std::string attrib; //<! attribute name
    std::string value;  //<! attribute value (if "" then only the presence of the attribute is neccessary)

    /// Create with given attribute and optinal desired value
    /** @param attrib attribute which presence should be mandatory 
        @param value optional mandatory attribute value if this is left ""
                     only the attribute name is made mandatory
        */
    XMLNodeFilterByAttrib(const std::string &attrib, const std::string value=""):
    attrib(attrib),value(value){}
    
    /// returns always true
    virtual bool operator()(const XMLNode&) const;

    /// deep copy function
    virtual XMLNodeFilter *deepCopy() const{ return new XMLNodeFilterByAttrib(attrib,value); }
  };

  /// Can be used to filter nodes by sub-pathes \ingroup XML
  /** E.g. doc["config"].recursive_begin(XMLNodeFilterByPathSubstring("config.section")); */
  class XMLNodeFilterByPathSubstring : public XMLNodeFilter{
    public:
    std::string pathPat; //<! internal path substring variable
    std::string delim;  //<! internal delimiter variable
    
    /// create with given path pattern and delimiter within this pattern
    XMLNodeFilterByPathSubstring(const std::string &pathPat,const std::string &delim="."):
    pathPat(pathPat),delim(delim){}
    
    /// returns always true
    virtual bool operator()(const XMLNode&) const;
    
    /// deep copy function
    virtual XMLNodeFilter *deepCopy() const{ return new XMLNodeFilterByPathSubstring(pathPat,delim); }
  };

  /// Can be used to filter nodes by regular expressions in their pathes \ingroup XML
  /** E.g. doc["config"].recursive_begin(XMLNodeFilterByPathRegex(".*section.*")); 
      Internally this function uses iclStringUtils' icl::match function
  */
  class XMLNodeFilterByPathRegex : public XMLNodeFilter{
    public:
    std::string regex; //<! internal path substring variable
    std::string delim;  //<! internal delimiter variable
    
    /// create with given path pattern and delimiter within this pattern
    XMLNodeFilterByPathRegex(const std::string &regex,const std::string &delim=".") throw (InvalidRegularExpressionException);
    
    /// returns always true
    virtual bool operator()(const XMLNode&) const;
    
    /// deep copy function
    virtual XMLNodeFilter *deepCopy() const{ return new XMLNodeFilterByPathSubstring(regex,delim); }
  };



  /// Filters by given hierarchy depth (always absolute to document root, which has depth 0) 
  class XMLNodeFilterByLevel : public XMLNodeFilter{
    public:
    /// max absolute level allowed
    int maxLevel;
    
    /// create with given path pattern and delimiter within this pattern
    XMLNodeFilterByLevel(float maxLevel):
    maxLevel(maxLevel){}
    
    /// returns always true
    virtual bool operator()(const XMLNode&) const;
    
    /// deep copy function
    virtual XMLNodeFilter *deepCopy() const{ return new XMLNodeFilterByLevel(maxLevel); }
  };

  /// Filters by given node type of first child node
  class XMLNodeFilterByFirstChildNodeType : public XMLNodeFilter{
    public:
    /// ored node types allowed
    int types;
    
    /// create with given ored XMLNode::Type values
    XMLNodeFilterByFirstChildNodeType(int types):
    types(types){}
    
    /// returns always true
    virtual bool operator()(const XMLNode&) const;
    
    /// deep copy function
    virtual XMLNodeFilter *deepCopy() const{ return new XMLNodeFilterByFirstChildNodeType(types); }
  };

  /// Filters nodes by testing them for having any direct child nodes of one of given types
  class XMLNodeFilterByHasAnyChildOfType : public XMLNodeFilter{
    public:
    /// ored node types allowed
    int types;
    
    /// create with given ored XMLNode::Type values
    XMLNodeFilterByHasAnyChildOfType(int types):
    types(types){}
    
    /// returns always true
    virtual bool operator()(const XMLNode&) const;
    
    /// deep copy function
    virtual XMLNodeFilter *deepCopy() const{ return new XMLNodeFilterByHasAnyChildOfType(types); }
  };


  
  
  /// Utility class for node filter combinations  \ingroup XML
  /** XMLNodeFilterCombination instances can easily be created using the operators
      | and & and ^ on simple or even comples NodeFilter class instances 
  */
  class XMLNodeFilterCombination : public XMLNodeFilter{
    public:
    
    enum Type { AND_OP, OR_OP, XOR_OP };
    
    private:
    Type t;
    XMLNodeFilter *a, *b;
    
    public:
    /// Default constructor
    XMLNodeFilterCombination(Type t, XMLNodeFilter *a, XMLNodeFilter *b);

    /// Copy constructor
    XMLNodeFilterCombination(const XMLNodeFilterCombination &other);
    
    /// Assignment operator
    XMLNodeFilterCombination &operator=(const XMLNodeFilterCombination &other);
    
    /// Destructor
    virtual ~XMLNodeFilterCombination();

    /// applies combinated filter expression
    virtual bool operator()(const XMLNode &node) const;
    
    /// deep copy function
    virtual XMLNodeFilter *deepCopy() const;
  };
  
  /// Creates an AND combinatio of two XMLNodeFilter instances \ingroup XML
  inline XMLNodeFilterCombination operator&(const XMLNodeFilter &a, const XMLNodeFilter &b){
    return XMLNodeFilterCombination(XMLNodeFilterCombination::AND_OP,a.deepCopy(),b.deepCopy());
  }

  /// Creates an OR combinatio of two XMLNodeFilter instances \ingroup XML
  inline XMLNodeFilterCombination operator|(const XMLNodeFilter &a, const XMLNodeFilter &b){
    return XMLNodeFilterCombination(XMLNodeFilterCombination::OR_OP,a.deepCopy(),b.deepCopy());
  }

  /// Creates an OR combinatio of two XMLNodeFilter instances \ingroup XML
  inline XMLNodeFilterCombination operator^(const XMLNodeFilter &a, const XMLNodeFilter &b){
    return XMLNodeFilterCombination(XMLNodeFilterCombination::XOR_OP,a.deepCopy(),b.deepCopy());
  }

}

#endif
