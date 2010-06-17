/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/XCFPublisher.cpp                             **
** Module : ICLIO                                                  **
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

#ifdef HAVE_XCF

#include <ICLIO/XCFPublisher.h>
#include <ICLIO/XCFUtils.h>

namespace icl{
  
  XCFPublisher::XCFPublisher() : m_publisher(0) {}
  
  XCFPublisher::XCFPublisher(const std::string &streamName, 
                             const std::string &imageURI) : 
    m_uri(imageURI),m_streamName(streamName) {
    m_publisher = XCF::Publisher::create(streamName);
    m_ctu = new XCF::CTU();
    m_btu = new XCF::Binary::TransportUnit();
  }
  
  XCFPublisher::~XCFPublisher() {
    if (m_publisher) {
      m_publisher->destroy();
    }
  }
  
  void XCFPublisher::createPublisher(const std::string &streamName, 
                                const std::string &imageURI) {
    if (!m_publisher) {
      m_uri = imageURI;
      m_streamName = streamName;
      m_publisher= XCF::Publisher::create(m_streamName);
      m_ctu = new XCF::CTU();
      m_btu = new XCF::Binary::TransportUnit();
    }
  }
  
  void XCFPublisher::publish(const ImgBase *image){
    xmltio::Location loc = XCFUtils::createXMLDoc(image,m_uri,"");
    std::string xml = loc.getText();
    m_ctu->setXML(xml);
    m_btu = XCFUtils::ImageToCTU(image,m_btu);
    m_ctu->deleteAllBinaries();
    m_ctu->addBinary(m_uri,m_btu);
    m_publisher->send(m_ctu);
  }
  
}

#endif
