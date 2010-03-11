/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_IO_UTILS_H
#define ICL_IO_UTILS_H

#include <ICLIO/Grabber.h>

#include <string>
#include <vector>
#include <ICLUtils/Time.h>

namespace icl{
  
  /// an internally used namespace to avoid symbol naming conflicts \ingroup UTILS_G
  namespace ioutils{
    
    /// converts a Time::value_type (long int) into a string
    std::string time2str(Time::value_type x);

    /// cops trailing whitespaces of a string
    std::string skipWhitespaces(const std::string &s);
    
    /// converts a string into an integer
    int ti(const std::string &value);
    
    /// converts a string into a long int
    long tl(const std::string &value);
    
    /// creates a vector of 3 elements v = (a,b,c)
    std::vector<int> vec3(int a, int b, int c);

    /// returns whether a given string ends with a given suffix
    bool endsWith(const std::string &s,const std::string &suffix);
    
    /// returns whether a given string begins with a given prefix
    bool startsWith(const std::string &s, const std::string &prefix);

    /// analyses a file pattern with hash-characters
    /** This function is e.g. used by the FilennameGenerator to extract a patterns hash count
        e.g. the pattern "image_###.ppm" shall be used to generate filenames like 
        "image_000.ppm", "image_001.ppm" and so on. This function returns the count of found
        hashes and the position in the string where the suffix begins. E.g. if the pattern is
        "image_##.ppm.gz", the hash-count is 2 and the suffix-pos becomes 8.
    **/
    void analyseHashes (const std::string &sFileName, unsigned int& nHashes, std::string::size_type& iPostfixPos);
  }
}

#endif
