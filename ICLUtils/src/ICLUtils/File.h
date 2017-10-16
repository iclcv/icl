/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/File.h                           **
** Module : ICLUtils                                               **
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
#include <ICLUtils/BasicTypes.h>
#include <string>
#include <vector>


namespace icl{
  namespace utils{

    /** \cond */
    class FileImpl;
    struct ICLUtils_API FileImplDelOp { static void delete_func(FileImpl *i); };
    /** \endcond */

    /// Utility class for file handling (reading files is buffered) \ingroup UTILS_G
    /** The File class implements an implicit gzip writing and reading.
        This feature is enabled if ICL_HAVE_LIB_Z is defined during the
        compilation process.\n
        If the File's given filename has a ".gz" postfix, read and
        write calls are applied using gzread and gzwrite from the libz.\n

        In addition the implementation of the File class was split into two
        parts: The File itself and its certain implementation, which is
        defined invisibly for the user. This implementation is shared
        by shallow copied instances using the ICL SmartPtr class. This mechanism
        provides save shallow copies using reference counting.
    **/
    class ICLUtils_API File : public utils::ShallowCopyable<FileImpl, FileImplDelOp>{
      public:

      /// mode to open files
      enum OpenMode{
        readBinary = 0,  /**!< open file for reading binary data */
        readText = 1,    /**!< open file for reading ascii data */
        writeBinary = 2, /**!< open file for reading writing binary data */
        writeText = 3,   /**!< open file for reading writing ascii data */
        notOpen = 4      /**!< for error handling -> null-return value  */
      };

      /// static utility method that reads a whole file as single string
      static std::string read_file(const std::string &filename, bool textMode=true);

      /// static utility method that reads a whole file as lines (text-mode only)
      static std::vector<std::string> read_lines(const std::string &filename);

      /// static utility method that writes a single string to a file
      static void write_file(const std::string &filename, const std::string &text,
                             bool textMode=true);

      /// static utility method that writes lines to a file (adding \n after each line)
      static void write_lines(const std::string &filename, const std::vector<std::string> &lines);


      /// Create a null file
      File();

      /// Create a file with given filename
      File(const std::string &name);

      /// Create a file with given filename which is opened imediately with given openmode
      File(const std::string &name, OpenMode om);

      /// Destructor (enshures that the file is closed)
      ~File();

      /// returns whether the file is open, or can be opened for reading
      bool exists() const;

      /// returns whether the file is a directory
      bool isDirectory() const;

      /// returns whether the file is already opened
      bool isOpen() const;

      /// returns if the file was opened for binary data transfer
      bool isBinary() const;

      /// returns the path postfix of the files url ( "../data.txt" -> "../")
      std::string getDir() const;

      /// returns the files basename ( "../data.txt" -> "data")
      std::string getBaseName() const;

      /// returns the files suffix ( "../data.txt" -> ".txt")
      std::string getSuffix() const;

      /// returns whole file url ( "../data.txt" -> "../data.txt")
      std::string getName() const;

      /// writes len bytes from data into the file
      void write(const void *data, int len);

      /// writes len bytes from data into the file and an additional Line-Break
      /** only in non-binary mode! */
      void writeLine(const void *data, int len);

      /// writes a string into the file
      void write(const std::string &text);

      /// sets floating point precision
      void setPrecision(const std::string &p="%8f");

      /// writes a character into the file
      File &operator<<(char c);

      /// writes an unsigned  character into the file
      File &operator<<(unsigned char uc);

      /// writes an integer into the file
      File &operator<<(int i);

      /// writes a unsigned integer into the file
      File &operator<<(unsigned int ui);

      /// writes a float into the file
      File &operator<<(float f);

      /// writes a double into the file
      File &operator<<(double d);

      /// writes a string into the file
      File &operator<<(const std::string &s);

      /// reads the next line of the file (buffered)
      /** only available in non-binary mode
      **/
      std::string readLine() const;

      /// reads the next line of the file into a given string
      /** breaks on newlines or on eof
          @return the given input string
      **/
      std::string &readLine(std::string &dst) const;

      /// reads the whole data of the file into an internal buffer and returns it
      const std::vector<icl8u> &readAll() const;

      /// returns the pointer to the internal file buffer at current position
      const icl8u* getCurrentDataPointer() const;

      /// returns the pointer to the internal file buffers begin
      const icl8u* getFileDataPointer() const;

      /// reads len bytes from the file (buffered)
      /** if less than len bytes are available in the file, a
          zero length vector is returned
      **/
      std::vector<icl8u> read(int len) const;

      /// reads max. len bytes into the destination pointer
      /** @return number of bytes actually read */
      int read(int len, void *dst) const;

      /// returns whether more lines can be read from this file
      bool hasMoreLines() const;

      /// returns the number of to-be-read bytes in the file
      int bytesAvailable() const;

      /// returns the number of bytes in this file only in read-mode
      int getFileSize() const;

      /// opens the file with given openmode ("rw" is not yet supported!)
      /** Throws a FileOpenException if file can not be opened */
      void open(OpenMode om);

      /// re-opens the file with given openmode ("rw" is not yet supported!)
      /** Throws a FileOpenException if file can not be opened */
      void reopen(OpenMode om);

      /// closes the file
      void close();

      /// erases the file
      void erase();

      /// jumps to the beginning of the file (only in read mode)
      void reset();

      /// returns the file handle
      void *getHandle() const;

      /// returns whether this file was opened with read access
      bool canRead() const;

      /// returns whether this file was opened with write access
      bool canWrite() const;

      /// returns the current OpenMode of this file of notOpen
      OpenMode getOpenMode() const;

    };
  } // namespace utils
}

