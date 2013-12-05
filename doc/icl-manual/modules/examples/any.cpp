#include <ICLUtils/Any.h>

#include <ICLMath/FixedMatrix.h>
#include <ICLUtils/Size.h>
#include <ICLCore/Types.h>
#include <ICLCore/CoreFunctions.h>

using namespace icl;

int main(){
  /// create Any instance from data types (implicitly)
  utils::Any i = 5;
  utils::Any d = M_PI;
  utils::Any s = utils::Size::VGA;
  utils::Any f = core::formatRGB;
  utils::Any m = math::FixedMatrix<float,3,2>(1,2,3,
                                              4,5,6);

  // show
  std::cout << " int:" << i << " double:" << d 
            << " size:" << s << " format:" << f 
            << std::endl << " matrix:" << m << std::endl;

  /// create data-type from Any instances (implicitly)
  int ii = i;
  double dd = d;
  utils::Size ss = s;
  core::format ff = f;
  math::FixedMatrix<float,3,2> mm = m;

  // show
  std::cout << " int:" << ii << " double:" << dd 
            << " size:" << ss << " format:" << ff 
            << std::endl << " matrix:" << mm << std::endl;

  
  
  // explicit cross-type parsing
  int x = d.as<int>();
  utils::Point y = s.as<utils::Point>();
  
  std::cout << "double as int:" << x << std::endl;

  std::cout << "size as point:" << y << std::endl;
  
}
