// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/FileWriter.h>
#include <icl/utils/StringUtils.h>

// plugins
#include <icl/io/FileWriterPluginPNM.h>
#include <icl/io/FileWriterPluginCSV.h>
#include <icl/io/FileWriterPluginBICL.h>
#ifdef ICL_HAVE_LIBJPEG
#include <icl/io/FileWriterPluginJPEG.h>
#endif
#ifdef ICL_HAVE_IMAGEMAGICK
#include <icl/io/FileWriterPluginImageMagick.h>
#endif

#ifdef ICL_HAVE_LIBPNG
#include <icl/io/FileWriterPluginPNG.h>
#endif

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
  std::map<std::string,FileWriterPlugin*, std::less<>> FileWriter::s_mapPlugins;

  class FileWriterPluginMapInitializer{
  public:

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
      for(std::map<std::string,FileWriterPlugin*, std::less<>>::iterator it = FileWriter::s_mapPlugins.begin();
          it != FileWriter::s_mapPlugins.end(); ++it){
        delete it->second;
      }
    }
  };


  static FileWriterPluginMapInitializer __static_filewriter_plugin_initializer__;


  FileWriter::FileWriter(){

  }


  FileWriter::FileWriter(const std::string &filepattern):

    m_oGen(filepattern){}


  FileWriter::FileWriter(const FilenameGenerator &gen):

    m_oGen(gen){}


  FileWriter::~FileWriter(){

  }


  const FilenameGenerator &FileWriter::getFilenameGenerator() const{

    return m_oGen;
  }


  void FileWriter::write(const ImgBase *image){
    ICLASSERT_RETURN(image);
    ICLASSERT_RETURN(image->getDim());
    ICLASSERT_RETURN(image->getChannels());
    ICLASSERT_RETURN(!m_oGen.isNull());
    ICLASSERT_RETURN(m_oGen.filesLeft());

    File file(m_oGen.next());

    if(auto it = FileWriter::s_mapPlugins.find(toLower(file.getSuffix())); it == s_mapPlugins.end()){
      ERROR_LOG("No Plugin to write files with suffix " << file.getSuffix() << " available");
      return;
    } else {
      it->second->write(file,image);
    }
  }


  FileWriter &FileWriter::operator<<(const ImgBase *image){

    write(image);
    return *this;
  }


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
      ERROR_LOG("Unable to std::set option \"jpg:quality\" (JPEG support is currently disabled!)");
#endif
    }else{
      ERROR_LOG("Unsupported Option \"" << option << "\" (value: \"" << value << "\")");
    }

  }


  } // namespace icl::io