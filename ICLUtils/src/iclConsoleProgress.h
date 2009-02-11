#ifndef ICL_CONSOLE_PROGRESS_H
#define ICL_CONSOLE_PROGRESS_H

#include <string>

namespace icl{
  
  /// static utility function for displaying some progress information in console
  void progress_init(const std::string &text="Creating LUT");
  
  /// static utility function for displaying some progress information in console
  void progress_finish();

  /// static utility function for displaying some progress information in console
  void progress(int curr, int max);

}

#endif
