/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/XML.h                            **
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

#pragma once

#include <ICLUtils/PugiXML.h>

namespace icl{
  namespace utils{
    
    // note: this file basically contains ICL-specific typedefs for
    // The included PugiXML backend
    
    /// Node type \ingroup XML
    typedef pugi::xml_node_type XMLNodeType;
    
    /// XML encoding type \ingroup XML
    typedef pugi::xml_encoding XMLEncoding;
    
    /// XML writer interface \ingroup XML
    typedef pugi::xml_writer XMLWriter;
    
    /// XML writer implementation for streams \ingroup XML
    typedef pugi::xml_writer_stream XMLStreamWriter;
    
    /// XML writer implementation for files \ingroup XML
    typedef pugi::xml_writer_file XMLFileWriter;
    
    /// XML Attribute class \ingroup XML
    typedef pugi::xml_attribute XMLAttribute;
    
    /// XML Node class \ingroup XML
    typedef pugi::xml_node XMLNode;
    
    /// Iterator for XMLNodes \ingroup XML
    typedef pugi::xml_node_iterator XMLNodeIterator;
    
    /// Iterator for XMLAttributes \ingroup XML
    typedef pugi::xml_attribute_iterator XMLAttributeIterator;
    
    /// XML-Treewalker class \ingroup XML
    typedef pugi::xml_tree_walker XMLTreeWalker;
  
    /// Parsing status enumeration \ingroup XML
    typedef pugi::xml_parse_status XMLParseStatus;
    
    /// Parsing status class \ingroup XML
    typedef pugi::xml_parse_result XMLParseResult;
  
    /// Main XML Document class \ingroup XML
    typedef pugi::xml_document XMLDocument;
    
    /// Type enumeration for xpath values \ingroup XML
    typedef pugi::xpath_value_type XPathValueType;
  
    /// Parse Result class for XPath expressions\ingroup XML
    typedef pugi::xpath_parse_result XPathParseResult;
  
    /// Variable Type for XPath expressions \ingroup XML
    typedef pugi::xpath_variable XPathVariable;
  
    /// Set of XPathVariables \ingroup XML
    typedef pugi::xpath_variable_set XPathVariableSet;
  
    /// Precompiled XPath expression \ingroup XML
    typedef pugi::xpath_query XPathQuery;
  
    /// Exception type for xpath expressions \ingroup XML
    typedef pugi::xpath_exception XPathException;
  
    /// Special node type for XPath query results \ingroup XML
    typedef pugi::xpath_node XPathNode;
  
    /// Set of XPath nodes \ingroup XML
    typedef pugi::xpath_node_set XPathNodeSet;
  } // namespace utils
}

