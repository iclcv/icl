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

#ifndef XML_NODE_CONST_ITERATOR_H
#define XML_NODE_CONST_ITERATOR_H

#include <ICLUtils/XMLNodeIterator.h>

namespace icl{

  /// Shallow wrapper of a XMLNodeIterator instance that provides const access to nodes \ingroup XML
  /** @see XMLNodeIterator */
  class XMLNodeConstIterator : public std::iterator<std::forward_iterator_tag,XMLNode>{
    
    /// Wrapped uncost iterator type
    XMLNodeIterator it;
    
    /// Private 'real' constructor only available by the friend class XMLNode
    XMLNodeConstIterator(const XMLNode &origin, const XMLNodeFilter &filter):it(origin,filter){}

    public:

    /// Empty constructor creating an end-iterator
    XMLNodeConstIterator(){}
    
    /// For tight integration with XMLNode class
    friend class XMLNode;
    
    /// pre-increment operator
    XMLNodeConstIterator operator++(int){
      XMLNodeConstIterator t(*this);  ++it;  return t;
    }
   
    /// post-increment operator
    XMLNodeConstIterator &operator++(){
      ++it; return *this;
    }
    
    /// dereference operator (providing access to current node) (const only)
    const XMLNode &operator*() const{
      return it.operator*();
    }

    /// pointer operator (providing access to current node) (const only)
    const XMLNode* operator->() const{
      return it.operator->();
    }

    /// inequality comparison operator
    bool operator!=(const XMLNodeConstIterator &other) const{
      return it != other.it;
    }
    /// equality comparison operator
    bool operator==(const XMLNodeConstIterator &other) const{
      return it == other.it;
    }
  };

}
#endif
