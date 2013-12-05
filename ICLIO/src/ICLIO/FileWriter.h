/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/FileWriter.h                           **
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

#pragma once

#include <ICLUtils/Uncopyable.h>
#include <ICLIO/ImageOutput.h>
#include <ICLIO/FilenameGenerator.h>
#include <ICLIO/FileWriterPlugin.h>
#include <ICLCore/Types.h>
#include <string>
#include <map>

namespace icl{
  namespace io{
  
    ///  File Writer implementation writing images to the hard disc \ingroup FILEIO_G
    /** \section OVERVIEW Overview 
        The implementation has been re-designed to provide a structured more flexible
        plugin based interface for writing images using most different file formats.
        Currently the following formats are supported:
        - <b>ppm</b> (8 Bit 3-channel interleaved color image core::format - 24Bit per pixel ) 
          multi channel images must have a channel count which is a multiple of 3. The
          original channel information is encoded into the header. Other image viewers
          will show e.g. a 6 channel image as a default 3 channel image where the channels
          3,4 and 5 are put <b>under</b> the channels 0, 1 and 2 (<b>vertical extended</b>)
        - <b>pgm</b> ( 8 Bit mono core::format ) Multi channel images are written by a vertical 
          of the image
        - <b>pnm</b> on of ppm or pgm (the viewer or grabber will determine the actual format
          by parsing the first line of the image file
        - <b>icl</b> icl file core::format . For best compatibility the icl core::format includes a header
          which is denoted by trailing '#' characters and which determines the images
          parameters. In the data section, the channel-data is put planar into the file
          channel by channel. Apart from the ".csv"-core::format, the ICL core::format is the only
          core::format, which provides different data types (in particular floats and doubles).
        - <b>csv</b> Comma Separated Values core::format. To provide a convenient interface to
          math-programs as gnuplot, matlab or maple this core::format can be use. In contrast 
          to all the other formats, the csv-core::format is <em>human readable</em>, which means,
          that each value is written as ASCII text.
        - <b>jpg</b> Format of the <b>J</b>oint <b>P</b>hotographic <b>E</b>xperts <b>G</b>roup.
          Loss-full compressed image core::format (libjpeg required and -DWITH_JPEG_SUPPORT must be
          defined, which is performed automatically by the makefile system)
        - <b>bicl</b> ICL's built-in binary core::format. This core::format does also support loading
          and saving of image meta data
        - <b>rle1</b> for run-length encoded binary images (best core::format for low-noise
          binary images, does also support writing and reading of image meta data)
        - <b>rle4,rle6,rle8</b> for non-binary images with run-length encoding (other than this,
          as rle1
        - <b>jicl</b> only supported with jpeg support, like bicl, but with jpeg compressed
          image data (jpeg compression is set to 70%, does also support saving meta data).
  
        
        \section ZLIB Z-Lib support
          All supported file formats (except jpg) can be written/read as gzipped file. This 
          feature is available if the libz is found by the makefile system, which automatically
          defines the -DWITH_ZLIB_SUPPORT then. To write a file with zip compression, you
          just have to add an additional ".gz"-suffix to the file name.
        
        \section SPEED IO-Speed
          Dependent on the particular core::format, the IO process needs a larger or smaller
          amount of time. The following table shows a summary of the I/O Times. The times
          are taken on a 1.6GHz pentium-m notebook, so the actual times may fluctuate:
        
        <table>
        <tr> <td>          </td>  <td><b>Writing (gz)</b></td>  <td><b>Reading (gz)</b></td>  <td><b>File-utils::Size (640x480-Parrot) (gz)</b></td></tr>
        <tr> <td><b>ppm</b></td>  <td>10ms (100ms)</td>         <td>6ms (25ms)</td>           <td>901K (545K)</td> </tr>
        <tr> <td><b>pnm</b></td>  <td>10ms (100ms)</td>         <td>6ms (25ms)</td>           <td>901K (545K)</td> </tr>
        <tr> <td><b>pgm</b></td>  <td>7ms (120ms)</td>          <td>7ms (25ms)</td>           <td>901K (562K)</td> </tr>
  
        <tr> <td><b>icl</b></td>  <td>4ms (122ms)</td>          <td>7ms (26ms)</td>           <td>901K (562K)</td> </tr>
  
        <tr> <td><b>csv</b></td>  <td>800ms (1800ms)</td>       <td>780ms (820ms)</td>        <td>2901K (690K)</td> </tr>
        
        <tr> <td><b>jpg 10%</b></td>  <td>5ms (not supported)</td> <td>5ms (not supported)</td> <td>15K (not supported)</td> </tr>
        <tr> <td><b>jpg 50%</b></td>  <td>6ms (not supported)</td> <td>5ms (not supported)</td> <td>41K (not supported)</td> </tr>
        <tr> <td><b>jpg 90%</b></td>  <td>5ms (not supported)</td> <td>5ms (not supported)</td> <td>101K (not supported)</td> </tr>
        <tr> <td><b>jpg 100%</b></td>  <td>7ms (not supported)</td> <td>3ms (not supported)</td> <td>269K (not supported)</td> </tr>
        </table>
        
        \section EX Example
        The following example illustrates using the file writer:
        \code
        
        #include <ICLIO/FileWriter.h>
        #include <ICLQt/Quick.h>

        int main(){
           // create an image
           icl::core::Img8u a = cvt8u(scale(create("parrot"),640,480));

           // create the file writer
           icl::io::FileWriter writer("image_####.jpg");

           // write the file
           writer.write(&a);
        }
        \endcode
    **/
    class ICL_IO_API FileWriter : public ImageOutput{
      public:
      /// initializer class
      friend class FileWriterPluginMapInitializer;
  
      /// creates an empty file writer
      FileWriter();
      
      /// Creates a new filewriter with given filepattern
      /** @param filepattern this string is passed to the member FilenameGenerator
                 @see FilenameGenerator 
      **/
      FileWriter(const std::string &filepattern);
      
      /// Creates a new FileWriter with given FilenameGenerator
      FileWriter(const FilenameGenerator &gen);
      
      /// Destructor
      ~FileWriter();
      
      /// returns the wrapped filename generator reference
      const FilenameGenerator &getFilenameGenerator() const;
  
      /// writes the next image
      void write(const core::ImgBase *image);
  
      /// wraps write to implement ImageOutput interface
      virtual void send(const core::ImgBase *image) { write(image); }
      
      /// as write but in stream manner
      FileWriter &operator<<(const core::ImgBase *image);
  
      /// sets a core::format specific option
      /** currently allowed options are:
          - "jpg:quality"  values of type int in range [0,100] 
          - "csv:extend-file-name" value of type bool ("true" or "false")
      **/
      void setOption(const std::string &option, const std::string &value);
      
      private:
      /// internal generator for new filenames
      FilenameGenerator m_oGen;
      
      /// static map of writer plugins
      static std::map<std::string,FileWriterPlugin*> s_mapPlugins;
    };
  
  } // namespace io
}

