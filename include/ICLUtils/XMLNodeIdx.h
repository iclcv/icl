/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/XMLNodeIdx.h                          **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_XML_NODE_IDX_H
#define ICL_XML_NODE_IDX_H

#include <string>

namespace icl{
  
  /// Simple utility structure, that can be used to reference certain subnodes \ingroup XML
  /** The XMLNodeIdx structure can be passed to the XMLNode's []-operator to 
      reference the k'th subnode with given tag-name. However this is not even
      close to the capabilities of XPath, this might be quiet useful sometimes.
      
      A special global utility function icl::xmlidx create XMLNodeIdx instances:
      @see xmlidx
      
      For a convenient access to first and last subnodes, a special type is
      can be handled internally as well. Simple use the macros 
      LAST_CHILD and FIRST_CHILD as indices for the []-operator of XMLNode's
  */
  struct XMLNodeIdx{
    
    /// internal special type
    enum Special {FIRST, LAST, NOTHING};
    
    /// special constructor 
    XMLNodeIdx(Special s):m_special(s){}
    
    /// constructor with given tag name and index
    XMLNodeIdx(const std::string &tag, unsigned int idx):m_idx(idx),m_tag(tag),m_special(NOTHING){}

    /// node index
    int m_idx;

    /// node tag
    std::string m_tag;
    
    /// node special flag
    Special m_special;
    
  };
  
  /// simply create an XMLNodeIndex with this global function \ingroup XML
  inline XMLNodeIdx xmlidx(const std::string &tag, unsigned int idx){
    return XMLNodeIdx(tag,idx);
  }

  /// \ingroup XML
#define LAST_CHILD XMLNodeIdx(XMLNodeIdx::LAST)

  /// \ingroup XML
#define FIRST_CHILD XMLNodeIdx(XMLNodeIdx::FIRST)

}

#endif
