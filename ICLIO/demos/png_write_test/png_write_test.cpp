#include <ICLQt/Quick.h>
#include <ICLIO/GenericImageOutput.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::io;

GenericImageOutput out;

int main(int argc, char **argv) {
    out.init("file","image_######.png");

    Img16s test(Size(300,300),1);

    test.fill(1000);

    out.send(&test);

    return 0;
}
