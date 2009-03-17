#ifndef ICL_ICON_FACTORY_H
#define ICL_ICON_FACTORY_H

#include <QPixmap>
#include <QIcon>
#include <iclImg.h>

namespace icl{

  /// Simple utility class providing static functions to create some icons
  class IconFactory{
    public:
    static const QPixmap &create_icl_window_icon_as_qpixmap();
    static const QIcon &create_icl_window_icon_as_qicon();

    static const QIcon &create_icon(const std::string &id);
    static const Img8u &create_image(const std::string &id);
  };
  
}
#endif
