/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/FilenameGenerator.h                    **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/ShallowCopyable.h>
#include <string>
#include <vector>

namespace icl{
  namespace io{
    /** \cond */
    class FilenameGeneratorImpl;
    struct FilenameGeneratorImplDelOp { static void delete_func(FilenameGeneratorImpl *i); };
    /** \endcond */

    /// Utility class for generating a stream of filenames \ingroup UTILS_G
    /** This list can have a finite or an infinite size. The class provides functions to
        get the next filename (which inplicitly increases the internal counter), to get
        the count of remaining filenames and to reset the internal counter to first value.\n
        The FilenameGenerator class extends the ShallowCopyable class interface to provide
        cheap-copies using reference counting.
    **/
    class ICLIO_API FilenameGenerator : public utils::ShallowCopyable<FilenameGeneratorImpl, FilenameGeneratorImplDelOp>{
      public:
      static const int INFINITE_FILE_COUNT;
      /// Null constructor
      FilenameGenerator();

      /// Destructor
      ~FilenameGenerator();
      /// generate a new filename list with given maxFiles
      /** if the maxFile count is reached, the filegenerator is resetted internally
          and will produce start counting from the beginning on again. If
          maxFiles is -1, there is no stop criterion
          example:
          <pre>
          pattern = image_#.ppm
          maxFiles = 10
          list = { image_0.ppm, image_1.ppm, ..., image_10.ppm }

          pattern = image_##.ppm
          maxFiles = 10
          list = { image_00.ppm, image_01.ppm, ..., image_10.ppm }

          pattern = image_#####.ppm.gz
          maxFiles = -1
          list = { image_00000.ppm.gz, image_00001.ppm.gz, ... }
          </pre>
      **/
      FilenameGenerator(const std::string &pattern, int maxFiles=-1);

      /// generate a filename list with given prefix,postfix and object/image index range
      /** example:
          <pre>
          prefix = "image_"
          postfix = ".ppm"
          objectStart = 0
          objectEnd = 5
          imageStart = 10
          imageEnd = 20

          list = { image_0__10.ppm, image_0__11.ppm, ..., image_0__20.ppm,
                   image_1__10.ppm, image_1__11.ppm, ..., image_1__20.ppm,
                   ...
                   image_5__10.ppm, image_5__11.ppm, ..., image_5__20.ppm }
          </pre>
      */
      FilenameGenerator(const std::string& prefix,
                        const std::string& postfix,
                        int objectStart, int objectEnd,
                        int imageStart, int imageEnd);

      /// returns the next file from the list
      std::string next();

      /// returns the next file without incrementing the internal counter (preview of next filename)
      std::string showNext() const;

      /// returns the number of files left (-1) if the FileList's length is infinite
      int filesLeft() const;

      /// must be called if if files left is < 0
      void reset();

      /// returns a list of all files (only if the count if files if finite)
      std::vector<std::string> getList();

      /// shows all files to if( the filelist is finite, the 10 first files are show);
      void show();



    };
  } // namespace io
}

