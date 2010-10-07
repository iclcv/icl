/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/FileWriter.h                             **
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

#ifndef ICL_FILE_WRITER2_H
#define ICL_FILE_WRITER2_H

#include <ICLUtils/Uncopyable.h>
#include <ICLIO/ImageOutput.h>
#include <ICLIO/FilenameGenerator.h>
#include <ICLIO/FileWriterPlugin.h>
#include <ICLCore/Types.h>
#include <string>
#include <map>

namespace icl{

  ///  File Writer implementation writing images to the hard disc \ingroup FILEIO_G
  /** \section OVERVIEW Overview 
      The implementation has been re-designed to provide a structured more flexible
      plugin based interface for writing images using most different file formats.
      Currently the following formats are supported:
      - <b>ppm</b> (8 Bit 3-channel interleaved color image format - 24Bit per pixel ) 
        multi channel images must have a channel count which is a multiple of 3. The
        original channel information is encoded into the header. Other image viewers
        will show e.g. a 6 channel image as a default 3 channel image where the channels
        3,4 and 5 are put <b>under</b> the channels 0, 1 and 2 (<b>vertical extended</b>)
      - <b>pgm</b> ( 8 Bit mono format ) Multi channel images are written by a vertical 
        of the image
      - <b>pnm</b> on of ppm or pgm (the viewer or grabber will determine the actual format
        by parsing the first line of the image file
      - <b>icl</b> icl file format . For best compatibility the icl format includes a header
        which is denoted by trailing '#' characters and which determines the images
        parameters. In the data section, the channel-data is put planar into the file
        channel by channel. Apart from the ".csv"-format, the ICL format is the only
        format, which provides different data types (in particular floats and doubles).
      - <b>csv</b> Comma Separated Values format. To provide a convenient interface to
        math-programs as gnuplot, matlab or maple this format can be use. In contrast 
        to all the other formats, the csv-format is <em>human readable</em>, which means,
        that each value is written as ASCII text.
      - <b>jpg</b> Format of the <b>J</b>oint <b>P</b>hotographic <b>E</b>xperts <b>G</b>roup.
        Loss-full compressed image format (libjpeg required and -DWITH_JPEG_SUPPORT must be
        defined, which is performed automatically by the makefile system)
      
      \section ZLIB Z-Lib support
        All supported file formats (except jpg) can be written/read as gzipped file. This 
        feature is available if the libz is found by the makefile system, which automatically
        defines the -DWITH_ZLIB_SUPPORT then. To write a file with zip compression, you
        just have to add an additional ".gz"-suffix to the file name.
      
      \section SPEED IO-Speed
        Dependent on the particular format, the IO process needs a larger or smaller
        amount of time. The following table shows a summary of the I/O Times. The times
        are taken on a 1.6GHz pentium-m notebook, so the actual times may fluctuate:
      
      <table>
      <tr> <td>          </td>  <td><b>Writing (gz)</b></td>  <td><b>Reading (gz)</b></td>  <td><b>File-Size (640x480-Parrot) (gz)</b></td></tr>
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
      The following example illustrates using the file grabber:
      \code
      
      #include <ICLIO/FileWriter.h>
      #include <ICLQuick/Quick.h>

      int main(){
         // create an image
         Img8u a = cvt8u(scale(create("parrot"),640,480));

         // create the file writer
         FileWriter writer("image_####.jpg");

         // write the file
         writer.write(&a);
      }
      \endcode
  **/
  class FileWriter : public ImageOutput, public Uncopyable{
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
    void write(const ImgBase *image);

    /// wraps write to implement ImageOutput interface
    virtual void send(const ImgBase *image) { write(image); }
    
    /// as write but in stream manner
    FileWriter &operator<<(const ImgBase *image);

    /// sets a format specific option
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

}

#endif
