// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/detail/file-plugins/FileWriterPluginCSV.h>
#include <icl/core/Types.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/prop/Constraints.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {

  FileWriterPluginCSV::FileWriterPluginCSV() {
    addProperty("extend-file-name", utils::prop::Flag{}, m_extendFileName,
                "If true, encode image params into the file name (paired "
                "with the CSV reader plugin's decoder).");
    registerCallback([this](const utils::Configurable::Property &p) {
      if (p.name == "extend-file-name") m_extendFileName = p.as<bool>();
    });
  }

  FileWriterPluginCSV &FileWriterPluginCSV::instance() {
    static FileWriterPluginCSV inst;
    return inst;
  }

  void FileWriterPluginCSV::write(File &file, const ImgBase *image){

    //////////////////////////////////////////////////////////////////////
    /// WRITE HEADER DATA DEPENDEND ON THE CURRENT EXTEND-FLAG-VALUE  ////
    //////////////////////////////////////////////////////////////////////

    if(m_extendFileName){
      std::ostringstream os;
      os << file.getDir() << file.getBaseName() << "-ICL:" << image->getSize() << 'x'
         << image->getChannels() << ':' <<image->getDepth() << ':' <<image->getFormat()
         << file.getSuffix();
      file = File(os.str());
    }

    file.open(File::writeText);

    if(!m_extendFileName){
      std::ostringstream os;
      static const std::string H = "# ";
      Rect roi = image->getROI();
      os << H << "Size " << image->getWidth() << ' ' << image->getHeight() << std::endl
         << H << "Channels " << image->getChannels() << std::endl
         << H << "ROI" << roi.x << ' ' << roi.y << ' '  << roi.width << ' ' << roi.height << std::endl
         << H << "Format " << image->getFormat() << std::endl
         << H << "ImageDepth " << image->getDepth() << std::endl
         << H << "TimeStamp " << image->getTime() << std::endl;
      file << os.str();
    }

    //////////////////////////////////////////////////////////////////////
    /// WRITE THE IMAGE DATA TEMPLATE BASED  /////////////////////////////
    //////////////////////////////////////////////////////////////////////

    for(int c=0;c<image->getChannels();++c){
      for(int y=0;y<image->getHeight();++y){
        switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D:{                                \
          const icl##D *p = image->asImg<icl##D>()->getROIData(c,Point(0,y));   \
          for(int x=0;x<image->getWidth();++x){                                 \
            file << str<icl##D>(p[x]) << ",";                                   \
          }                                                                     \
          break;}
          ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        }
        file << "\n";
      }
    }
  }
  } // namespace icl::io

#include <icl/io/file/FileWriter.h>  // REGISTER_FILE_WRITER_PLUGIN / REGISTER_FILE_WRITER_CONFIG
namespace { using icl::io::FileWriterPluginCSV; }
#define ICL_CSV_REG(TAG, EXT)                                                 \
  REGISTER_FILE_WRITER_PLUGIN(TAG, EXT,                                       \
    [](icl::utils::File &f, const icl::core::ImgBase *img) {                  \
      FileWriterPluginCSV::instance().write(f, img);                          \
    })
ICL_CSV_REG(csv, ".csv");
#ifdef ICL_HAVE_LIBZ
ICL_CSV_REG(csv_gz, ".csv.gz");
#endif
#undef ICL_CSV_REG

REGISTER_FILE_WRITER_CONFIG(csv, "csv",
  []() -> icl::utils::Configurable* { return &FileWriterPluginCSV::instance(); });