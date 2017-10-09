/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/FileList.h                             **
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
    class FileListImpl;
    struct FileListImplDelOp { static void delete_func(FileListImpl *i); };
    /** \endcond */

    /// Utility class implementing a list of files \ingroup UTILS_G
    class ICLIO_API FileList : public utils::ShallowCopyable<FileListImpl, FileListImplDelOp>{
      public:

      /// Create a null file-list
      FileList();

      /// Create a file-list of given type
      /** pattern migtht be a file pattern like "images/ *.ppm"
          a single file like "./theimage.jpg" or a file seqenece file
          with postfix ".seq" which is a textfile where each line is
          single filename.\n

          \section Sequence File (postfix ".seq")
          Sequence files which are determined by their postfix ".seq" are
          are treated in a special way. A sequence file must contain a new-line
          separated list of filename (in particular other sequence files).
          Each entry of the seqence file is then added using a recursive "add"-
          function. If a sequence file contains other sequence files, this files
          are parsed recursively in the same way. Sequence files may not contain
          file patterns like "*.ppm".\n
          To avoid infinite recursion, the FileLists implementation internally
          holds a list of all already contained sequence files, so adding the
          same sequence more than once (even indirect by other sequence files)
          will have no effect.

          \section Omiting Doubled Files
          Another feature of the FileList class is provided using an additional
          constructor flag "omitDoubledFiles". If this flag is set to true
          (it is false by default), the FileLists implementation will internally
          skip files, which have already been added.


          @param pattern the file pattern Either something like
                         images/ *.p[gnp]m or a seqence file name
          @param omitDoubledFiles flag to control the creation of double file names. Double file names creation is not allowed at default.
      **/
      FileList(const std::string &pattern, bool omitDoubledFiles=false);

      /// create a file list by given set of filenames
      /** double filenames are allowed in this mode. Sequence files are not
          handled in a special way.*/
      FileList(const std::vector<std::string> &filenames);

      /// does nothing
      ~FileList();

      /// returns the number of files in this list
      int size() const;

      /// returns the i-th filename in this list
      const std::string &operator[](int i) const;

      /// adds all files from another FileList to this
      void join(const FileList &other);

      /// shows the filelist to std::out
      void show() const;

      /// generates a ".seq" from this FileList
      void toSequenceFile(const std::string &seqFileName) const;

      /// translates a hashpattern to a regular expression
      static std::string translateHashPattern(const std::string &hashPattern);
    };
  } // namespace io
}

