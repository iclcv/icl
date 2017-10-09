#include <ICLUtils/Random.h>
#include <vector>
#include <algorithm>

using namespace icl;

int main(){
  // intialize with current system time
  utils::randomSeed();

  // uniformly distributed random number between 0 and 1
  float f = utils::random(0,1);

  // Gaussian random number with mean 0 and variance 1
  float g = utils::gaussRandom(0,1);

  // buffer
  std::vector<float> ns(100);

  // fill each element with a random number
  // here uniformly distributed in [10,20]
  std::fill(ns.begin(),ns.end(), utils::URand(10,20));

  // or with gaussian random numbers
  std::fill(ns.begin(),ns.end(), utils::GRand(0,3));

  // create random indices (int values between 0 and given max)
  std::vector<int> is(100);
  std::fill(is.begin(),is.end(),utils::URandI(99));
}
