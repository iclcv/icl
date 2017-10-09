#include <ICLCore/Img.h>
#include <ICLFilter/UnaryCompareOp.h>

using namespace icl;

int main(){
  core::Img8u src (utils::Size::VGA,1),dst;

  filter::UnaryCompareOp cmp(">",128);

  // needs ImgBase** (but this does not work)
  // due to C++-issues
  // cmp.apply(&src,&&dst);

  // long version (but ugly)
  core::ImgBase *tmp = &dst;
  cmp.apply(&src,&tmp);

  // bpp-version
  cmp.apply(&src,core::bpp(dst));
}
