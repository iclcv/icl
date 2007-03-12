#ifdef WIN32
#ifndef _ICL_WIN32_H_
#define _ICL_WIN32_H_

#define NOMINMAX
#include <windows.h>
#define usleep(T) Sleep ((T)/1000)

#endif
#endif