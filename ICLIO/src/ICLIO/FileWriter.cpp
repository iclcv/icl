/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/FileWriter.cpp                         **
** Module : ICLIO                                                  **
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

#include <ICLIO/FileWriter.h>
#include <ICLUtils/StringUtils.h>

// plugins
#include <ICLIO/FileWriterPluginPNM.h>
#include <ICLIO/FileWriterPluginCSV.h>
#include <ICLIO/FileWriterPluginBICL.h>
#ifdef ICL_HAVE_LIBJPEG
#include <ICLIO/FileWriterPluginJPEG.h>
#endif
#ifdef ICL_HAVE_IMAGEMAGICK
#include <ICLIO/FileWriterPluginImageMagick.h>
#endif

#ifdef ICL_HAVE_LIBPNG
#include <ICLIO/FileWriterPluginPNG.h>
#endif

using namespace icl::utils;
using namespace icl::core;
using namespace std;

namespace icl{
  namespace io{

    map<string,FileWriterPlugin*> FileWriter::s_mapPlugins;

    class FileWriterPluginMapInitializer{
    public:
      // {{{ open

      FileWriterPluginMapInitializer(){
        FileWriter::s_mapPlugins[".ppm"] = new FileWriterPluginPNM;
        FileWriter::s_mapPlugins[".pgm"] = new FileWriterPluginPNM;
        FileWriter::s_mapPlugins[".pnm"] = new FileWriterPluginPNM;
        FileWriter::s_mapPlugins[".icl"] = new FileWriterPluginPNM;
        FileWriter::s_mapPlugins[".csv"] = new FileWriterPluginCSV;
        FileWriter::s_mapPlugins[".bicl"] = new FileWriterPluginBICL;
        FileWriter::s_mapPlugins[".rle1"] = new FileWriterPluginBICL("rlen","1");
        FileWriter::s_mapPlugins[".rle4"] = new FileWriterPluginBICL("rlen","4");
        FileWriter::s_mapPlugins[".rle6"] = new FileWriterPluginBICL("rlen","6");
        FileWriter::s_mapPlugins[".rle8"] = new FileWriterPluginBICL("rlen","8");



  #ifdef ICL_HAVE_LIBJPEG
        FileWriter::s_mapPlugins[".jpeg"] = new FileWriterPluginJPEG;
        FileWriter::s_mapPlugins[".jpg"] = new FileWriterPluginJPEG;
        FileWriter::s_mapPlugins[".jicl"] = new FileWriterPluginBICL("jpeg","85");
  #elif ICL_HAVE_IMAGEMAGICK
        FileWriter::s_mapPlugins[".jpeg"] = new FileWriterPluginImageMagick;
        FileWriter::s_mapPlugins[".jpg"] = new FileWriterPluginImageMagick;
  #endif

  #ifdef ICL_HAVE_LIBZ
        FileWriter::s_mapPlugins[".ppm.gz"] = new FileWriterPluginPNM;
        FileWriter::s_mapPlugins[".pgm.gz"] = new FileWriterPluginPNM;
        FileWriter::s_mapPlugins[".pnm.gz"] = new FileWriterPluginPNM;
        FileWriter::s_mapPlugins[".icl.gz"] = new FileWriterPluginPNM;
        FileWriter::s_mapPlugins[".csv.gz"] = new FileWriterPluginCSV;
        FileWriter::s_mapPlugins[".bicl.gz"] = new FileWriterPluginBICL;
        FileWriter::s_mapPlugins[".rle1.gz"] = new FileWriterPluginBICL("rlen","1");
        FileWriter::s_mapPlugins[".rle4.gz"] = new FileWriterPluginBICL("rlen","4");
        FileWriter::s_mapPlugins[".rle6.gz"] = new FileWriterPluginBICL("rlen","6");
        FileWriter::s_mapPlugins[".rle8.gz"] = new FileWriterPluginBICL("rlen","8");

  #endif

  #ifdef ICL_HAVE_LIBPNG
        FileWriter::s_mapPlugins[".png"] = new FileWriterPluginPNG;
  #endif

  #ifdef ICL_HAVE_IMAGEMAGICK

        static const char *imageMagickFormats[] = {
  #ifndef ICL_HAVE_LIBPNG
          "png",
  #endif
          "gif","pdf","ps","avs","bmp","cgm","cin","cur","cut","dcx",
          "dib","dng","dot","dpx","emf","epdf","epi","eps","eps2","eps3",
          "epsf","epsi","ept","fax","gplt","gray","hpgl","html","ico","info",
          "jbig","jng","jp2","jpc","man","mat","miff","mono","mng","mpeg","m2v",
          "mpc","msl","mtv","mvg","palm","pbm","pcd","pcds","pcl","pcx","pdb",
          "pfa","pfb","picon","pict","pix","ps","ps2","ps3","psd","ptif","pwp",
          "rad","rgb","pgba","rla","rle","sct","sfw","sgi","shtml","sun","svg",
          "tga","tiff","tim","ttf","txt","uil","uyuv","vicar","viff","wbmp",
          "wmf","wpg","xbm","xcf","xpm","xwd","ydbcr","ycbcra","yuv",0
        };
        for(const char **pc=imageMagickFormats;*pc;++pc){
          FileWriter::s_mapPlugins[std::string(".")+*pc] = new FileWriterPluginImageMagick;
        }


  #endif
        // add plugins
      }
      ~FileWriterPluginMapInitializer(){
        for(std::map<string,FileWriterPlugin*>::iterator it = FileWriter::s_mapPlugins.begin();
            it != FileWriter::s_mapPlugins.end(); ++it){
          delete it->second;
        }
      }
    };

    // }}}

    static FileWriterPluginMapInitializer __static_filewriter_plugin_initializer__;


    FileWriter::FileWriter(){
      // {{{ open

    }

    // }}}

    FileWriter::FileWriter(const std::string &filepattern):
      // {{{ open

      m_oGen(filepattern){}

    // }}}

    FileWriter::FileWriter(const FilenameGenerator &gen):
      // {{{ open

      m_oGen(gen){}

    // }}}

    FileWriter::~FileWriter(){
      // {{{ open

    }

    // }}}

    const FilenameGenerator &FileWriter::getFilenameGenerator() const{
      // {{{ open

      return m_oGen;
    }

    // }}}

    void FileWriter::write(const ImgBase *image){
      // {{{ open
      ICLASSERT_RETURN(image);
      ICLASSERT_RETURN(image->getDim());
      ICLASSERT_RETURN(image->getChannels());
      ICLASSERT_RETURN(!m_oGen.isNull());
      ICLASSERT_RETURN(m_oGen.filesLeft());

      File file(m_oGen.next());

      std::map<string,FileWriterPlugin*>::iterator it = FileWriter::s_mapPlugins.find(toLower(file.getSuffix()));
      if(it == s_mapPlugins.end()){
        ERROR_LOG("No Plugin to write files with suffix " << file.getSuffix() << " available");
        return;
      }

      it->second->write(file,image);
    }

    // }}}

    FileWriter &FileWriter::operator<<(const ImgBase *image){
      // {{{ open

      write(image);
      return *this;
    }

    // }}}

    void FileWriter::setOption(const std::string &option, const std::string &value){
      if(option == "csv:extend-file-name"){
        if(toLower(value) == "true"){
          FileWriterPluginCSV::setExtendFileName(true);
        }else if(toLower(value) == "false"){
          FileWriterPluginCSV::setExtendFileName(false);
        }else{
          ERROR_LOG("Undefined value \"" << value <<"\" for option \"" << option << "\"");
        }
      }else if(option == "jpg:quality"){
  #ifdef WITH_JPEG_SUPPORT
        FileWriterPluginJPEG::setQuality(parse<int>(value));
  #else
        ERROR_LOG("Unable to set option \"jpg:quality\" (JPEG support is currently disabled!)");
  #endif
      }else{
        ERROR_LOG("Unsupported Option \"" << option << "\" (value: \"" << value << "\")");
      }

    }


  } // namespace io
}


