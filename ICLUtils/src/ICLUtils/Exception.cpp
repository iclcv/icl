// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter, Michael Goetting, Robert Haschke

#include <ICLUtils/Exception.h>
#include <ICLUtils/Macros.h>
/*
  ICLException.cpp

  Written by: Michael G�tting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/


namespace icl {
  namespace utils{

    void ICLException::report() {
       FUNCTION_LOG("");

       std::cout << "ICL Exception: " << what() << std::endl;
    }

  } // namespace utils
} // namespace icl
