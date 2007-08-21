#include "iclBorderHandle.h"

#include <QGroupBox>

namespace icl{
  std::string BorderHandle::getTitle() const{
    const BorderHandle &bh = *this;//const_cast<BorderHandle*>(this);
    const QGroupBox *gb = *bh;
    if(gb){
      return gb->title().toLatin1().data();
    }
    return "undefined!";
  }
  void BorderHandle::operator=(const std::string &title){
    if(**this) (**this)->setTitle(title.c_str());
    (**this)->update();
  }
  
  
}
