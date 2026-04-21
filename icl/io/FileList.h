// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <string>
#include <vector>
#include <memory>

namespace icl::io {
  /** \cond */
  class FileListImpl;
  /** \endcond */

  /// Utility class implementing a list of files \ingroup UTILS_G
  class ICLIO_API FileList{
    public:

    /// Create a null file-list
    FileList();

    /// create a file list from an explicit set of filenames
    /** Double filenames are allowed in this mode. Sequence files are not
        handled in a special way. */
    FileList(const std::vector<std::string> &filenames);

    /// Expand a pattern into a FileList.
    /** `pattern` can be:
        - A glob like `images/ *.ppm` or `images/ *.p[gnp]m`
        - A single file path like `./theimage.jpg`
        - A sequence file (postfix `.seq`) — a newline-separated list
          of filenames (or other `.seq` files).  Sequence files are
          expanded recursively with cycle-detection, and cannot themselves
          contain glob patterns.

        @param pattern    glob / path / `.seq` file
        @param omitDoubledFiles  skip filenames that have already been
                                 added (default: keep all matches as-is). */
    static FileList glob(const std::string &pattern,
                         bool omitDoubledFiles = false);

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

    /// returns whether the file list is null (not initialized)
    bool isNull() const { return !impl; }

    private:
    std::shared_ptr<FileListImpl> impl;
  };
  } // namespace icl::io