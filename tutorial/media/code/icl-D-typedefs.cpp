#ifdef HAVE_IPP
  typedef Ipp64f icl64f; // 64Bit floating point type
  typedef Ipp32f icl32f; // 32Bit floating point type
  typedef Ipp32s icl32s; // 32bit signed integer type
  typedef Ipp16s icl16s; // 16bit signed integer type
  typedef Ipp8u icl8u;   // 8Bit unsigned integer type
#else
  typedef double icl64f; // 64Bit floating point type
  typedef float icl32f;  // 32Bit floating point type
  typedef int32_t icl32s;// 32bit signed integer type
  typedef int16_t icl16s;// 16bit signed integer type
  typedef uint8_t icl8u; // 8Bit unsigned integer type
#endif
