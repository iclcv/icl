#ifndef ICL_COMPMACROS_H
#define ICL_COMPMACROS_H

#ifdef WIN32
#	define IPP_DECL __stdcall
#	define rint ::System::Math::Round
#	define round ::System::Math::Round
#elif
#	define IPP_DECL
#endif

#endif
