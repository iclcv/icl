// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/ExternalViewer.h>
#include <icl/io/file/FileWriter.h>
#include <icl/utils/time/Time.h>
#include <icl/utils/thread/Thread.h>
#include <icl/utils/Exception.h>
#include <icl/utils/Macros.h>

#include <cstdio>
#include <cstdlib>
#include <string>

namespace icl::io {

  void show(const core::Image &image,
            const std::string &showCommand,
            long msec_to_rm_call,
            const std::string &rmCommand) {
    if(image.isNull()) return;

    std::string timeStr = utils::Time::now().toString();
    for(unsigned int i=0;i<timeStr.length();i++){
      if(timeStr[i]=='/' || timeStr[i]==' ' || timeStr[i]==':') timeStr[i]='_';
    }

    const std::string name = std::string(".tmpImage.") + timeStr + ".bicl";
    try {
      FileWriter(name).write(image);
    } catch(utils::FileOpenException &) {
      ERROR_LOG("unable to show image (invalid permissions to write a temporary\n"
                "                      image file in the current working directory)");
      return;
    } catch(utils::ICLException &) {
      ERROR_LOG("unable to show image (image could not be written to a temporary file)");
      return;
    }

    char showCommandStr[500];
    std::snprintf(showCommandStr, sizeof(showCommandStr),
                  showCommand.c_str(), name.c_str());

    int errorCode = std::system((std::string(showCommandStr) + " &").c_str());
    if(errorCode != 0)
      WARNING_LOG("Error code of system call unequal 0!");

    if(rmCommand.length()) {
      utils::Thread::msleep(msec_to_rm_call);
      char rmCommandStr[500];
      std::snprintf(rmCommandStr, sizeof(rmCommandStr),
                    rmCommand.c_str(), name.c_str());
      errorCode = std::system((std::string(rmCommandStr) + " &").c_str());
      if(errorCode != 0)
        WARNING_LOG("Error code of system call unequal 0!");
    }
  }

  void xv(const core::Image &image, const std::string &nameIn, long msec) {
    if(image.isNull()) return;

    std::string name = nameIn;
    if(image.getChannels() != 3) name += ".pgm";

    try {
      FileWriter(name).write(image);
    } catch(utils::FileOpenException &) {
      ERROR_LOG("unable to show image (invalid permissions to write a temporary\n"
                "                      image file in the current working directory)");
      return;
    } catch(utils::ICLException &) {
      ERROR_LOG("unable to show image (image could not be written to a temporary file)");
      return;
    }

    int errorCode = std::system(std::string("xv ").append(name).append(" &").c_str());
    if(errorCode != 0)
      WARNING_LOG("Error code of system call unequal 0!");

    utils::Thread::msleep(msec);

    errorCode = std::system(std::string(ICL_SYSTEMCALL_RM).append(name).c_str());
    if(errorCode != 0)
      WARNING_LOG("Error code of system call unequal 0!");
  }

} // namespace icl::io
