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
    using XMLNodeType = pugi::xml_node_type;

    /// XML encoding type \ingroup XML
    using XMLEncoding = pugi::xml_encoding;

    /// XML writer interface \ingroup XML
    using XMLWriter = pugi::xml_writer;

    /// XML writer implementation for streams \ingroup XML
    using XMLStreamWriter = pugi::xml_writer_stream;

    /// XML writer implementation for files \ingroup XML
    using XMLFileWriter = pugi::xml_writer_file;

    /// XML Attribute class \ingroup XML
    using XMLAttribute = pugi::xml_attribute;

    /// XML Node class \ingroup XML
    using XMLNode = pugi::xml_node;

    /// Iterator for XMLNodes \ingroup XML
    using XMLNodeIterator = pugi::xml_node_iterator;

    /// Iterator for XMLAttributes \ingroup XML
    using XMLAttributeIterator = pugi::xml_attribute_iterator;

    /// XML-Treewalker class \ingroup XML
    using XMLTreeWalker = pugi::xml_tree_walker;

    /// Parsing status enumeration \ingroup XML
    using XMLParseStatus = pugi::xml_parse_status;

    /// Parsing status class \ingroup XML
    using XMLParseResult = pugi::xml_parse_result;

    /// Main XML Document class \ingroup XML
    using XMLDocument = pugi::xml_document;

    /// Type enumeration for xpath values \ingroup XML
    using XPathValueType = pugi::xpath_value_type;

    /// Parse Result class for XPath expressions\ingroup XML
    using XPathParseResult = pugi::xpath_parse_result;

    /// Variable Type for XPath expressions \ingroup XML
    using XPathVariable = pugi::xpath_variable;

    /// Set of XPathVariables \ingroup XML
    using XPathVariableSet = pugi::xpath_variable_set;

    /// Precompiled XPath expression \ingroup XML
    using XPathQuery = pugi::xpath_query;

    /// Exception type for xpath expressions \ingroup XML
    using XPathException = pugi::xpath_exception;

    /// Special node type for XPath query results \ingroup XML
    using XPathNode = pugi::xpath_node;

    /// Set of XPath nodes \ingroup XML
    using XPathNodeSet = pugi::xpath_node_set;
  } // namespace utils
}
