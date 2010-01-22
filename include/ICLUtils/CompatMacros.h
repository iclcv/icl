#ifndef ICL_COMPAT_MACROS_H
#define ICL_COMPAT_MACROS_H

#ifdef SYSTEM_WINDOWS
#	define IPP_DECL __stdcall
#	ifndef _USE_MATH_DEFINES
#	define _USE_MATH_DEFINES
#	endif
#	define rint  ::System::Math::Round
#	define round ::System::Math::Round
#else
#	define IPP_DECL
#endif

#endif
