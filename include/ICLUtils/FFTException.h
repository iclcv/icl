#ifndef FFTEXCEPTION_H_
#define FFTEXCEPTION_H_
#include<ICLUtils/Exception.h>

namespace icl{
class FFTException : public ICLException{
public:
	FFTException(const std::string &msg):ICLException(msg){}
};
}
#endif /* FFTEXCEPTION_H_ */
