/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/XMLNodeIterator.cpp                       **
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

#include <ICLUtils/XMLNodeIterator.h>
#include <ICLUtils/XMLNode.h>


namespace icl{
  
  struct XMLNodeIterator::XMLNodeIteratorImpl{
    XMLNodeIteratorImpl():idx(-1){}
    XMLNodeIteratorImpl(const std::vector<XMLNode*> &v):
      v(v),idx(0){}
    std::vector<XMLNode*> v;
    int idx;
  };
  
  XMLNodeIterator::XMLNodeIterator(){
    impl = new XMLNodeIteratorImpl;
  }
  
  XMLNodeIterator::XMLNodeIterator(const XMLNode &origin, const XMLNodeFilter &filter){
    std::vector<XMLNode*> v = const_cast<XMLNode&>(origin).getAllChildNodes(filter);
    impl = new XMLNodeIteratorImpl(v);
  }

  XMLNodeIterator::XMLNodeIterator(const XMLNodeIterator &other){
    *impl = *other.impl;
  }

  XMLNodeIterator& XMLNodeIterator::operator=(const XMLNodeIterator &other){
    *this->impl = *other.impl;
    return *this;
  }

  XMLNodeIterator::~XMLNodeIterator(){
    ICL_DELETE(impl);
  }
  
  XMLNodeIterator XMLNodeIterator::operator++(int){
    XMLNodeIterator it = *this;
    ++impl->idx;
    return it;
  }

  XMLNodeIterator &XMLNodeIterator::operator++(){
    ++impl->idx;
    return *this;
  }

  XMLNode* XMLNodeIterator::operator->(){
    return impl->v[impl->idx];
  }

  const XMLNode* XMLNodeIterator::operator->() const{
    return impl->v[impl->idx];
  }

  
  XMLNode &XMLNodeIterator::operator*(){
    return *impl->v[impl->idx];
  }

  const XMLNode &XMLNodeIterator::operator*() const{
    return *impl->v[impl->idx];
  }
  
  bool XMLNodeIterator::operator!=(const XMLNodeIterator &other) const{
    return !operator==(other);
  }
  
  bool XMLNodeIterator::operator==(const XMLNodeIterator &other) const{
    if(impl->idx == -1 && other.impl->idx == -1) return true;
    if(impl->idx == -1) return other.impl->idx == (int)other.impl->v.size();
    if(other.impl->idx == -1) return impl->idx == (int)impl->v.size();
    return impl->idx == other.impl->idx;
  }
}
