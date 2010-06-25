/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/XCFUtils.h                               **
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
#ifndef ICL_XCF_UTILS_H
#define ICL_XCF_UTILS_H

#include <string>
#include <xmltio/xmltio.hpp>
#include <ICLCore/ImgBase.h>
#include <ICLCC/Bayer.h>
#include <ICLCC/Converter.h>
#include <xcf/RemoteServer.hpp>
#include <xmltio/Location.hpp>
#include <Memory/Interface.hpp>
#include <vector>


namespace icl{
  class XCFUtils{
    public:
    static const std::string &createEmptyXMLDoc();
    static xmltio::Location createXMLDoc(const ImgBase* image, const std::string& uri, const std::string& bayerPattern = "");
    /*
        static xmltio::Location createXMLDoc(const std::vector<ImgBase*> images,
        const std::vector<std::string> &uris,
        const std::vector<std::string> &bayerPatterns);
    */
    
    static void CTUtoImage(const XCF::CTUPtr ctu, const xmltio::Location& l, ImgBase** ppoImage);

    static  XCF::Binary::TransportUnitPtr ImageToCTU(const ImgBase* image, XCF::Binary::TransportUnitPtr btu);

    static void createOutputImage(const xmltio::Location& l, 
                                  ImgBase *poSrc, 
                                  ImgBase *poOutput, 
                                  ImgBase **poBayerBuffer, 
                                  BayerConverter *poBC, 
                                  Converter *poConv);
    
    static void getImage(memory::interface::MemoryPtr &mem, 
                         const std::string &xmlDoc,
                         ImgBase **dst,
                         memory::interface::Attachments *reusableAttachment=0,
                         const std::string &xpath="//IMAGE");
    
    static void attachImage(memory::interface::MemoryPtr &mem, 
                            xmltio::Location &anchor,
                            const std::string &imageURI,
                            const ImgBase *image,
                            memory::interface::Attachments *reusableAttachment=0,
                            bool insertInsteadOfReplace=true);
    

    struct ImageDescription{
      std::string uri;
      Size size;
      depth imagedepth;
      int channels;
      format imageformat;
      Rect roi;
      Time time;
      void show();
    };
    static ImageDescription getImageDescription(const xmltio::Location &l);
    
    static void serialize(const ImgBase *image, std::vector<unsigned char> &dst);
    static void unserialize(const std::vector<unsigned char> &src, const ImageDescription &descr, ImgBase **dst);
  };  
}


#endif // ICL_XCF_UTILS_H

#endif // HAVE_XCF
