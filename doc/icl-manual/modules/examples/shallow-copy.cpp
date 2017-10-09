#include <ICLCore/Img.h>
#include <ICLUtils/Random.h>

using namespace icl;

int main(){
  // original image
  core::Img8u a(utils::Size::VGA,1);

  // shallowly copied instance
  core::Img8u b = a;

  // initialize c with deep copy of a
  core::Img8u c = a.detached();

  // fills b's pixel with random numbers
  // note: a is also affected, but not c
  b.fill(utils::URand(0,255));

  // make b independent from a
  b.detach();

  // clears only b, a remains filled randomly
  b.clear();

  // another shallow copy
  core::Img8u d = a;

  // scaling d entails having to resize the
  // internal data pointers; d becomes
  // independent from a, a is not scaled
  d.scale(utils::Size(100,100));
}
