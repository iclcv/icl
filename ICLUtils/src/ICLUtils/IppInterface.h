/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/IppInterface.h                   **
** Module : ICLUtils                                               **
** Authors: Viktor Richter                                         **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <map>

#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Mutex.h>

/*
TODOS:
- this represents IppStatus-enum as signed integer. is that okay?
- cleaner debug output
- choose IPP environment variables
- what about building icl? something like IppInterface::available()?
- windows version
*/

namespace icl{
  namespace utils{

    class ICLDynamicLibLoadException : ICLException {
      public:
        ICLDynamicLibLoadException (const char* error) noexcept :
          ICLException (std::string("Can't load library: ") + std::string(error)) {}
        virtual ~ICLDynamicLibLoadException() noexcept {}
    };
    class ICLDynamicFunctionLoadException : ICLException {
      public:
        ICLDynamicFunctionLoadException (const char* error) noexcept :
          ICLException (std::string("Can't load function from library: ") + std::string(error)) {}
        virtual ~ICLDynamicFunctionLoadException() noexcept {}
    };

    class IppInterface : Uncopyable{
      public:
        /// getter function for singleton class
        static IppInterface* get();

        /// returns the no error status value of ipp as icl32s
        static icl32s statusNoError();

        /// the passed StsCode needs to be an ipp status code, otherwise the behaviour is not defined
        const char* ippGetStatusString(icl32s StsCode);

        icl32s ippmEigenValuesVectorsSym_m_32f(
            const icl32f* pSrc, icl32s srcStride1, icl32s srcStride2, icl32f* pBuffer,
            icl32f* pDstVectors, icl32s dstStride1, icl32s dstStride2,
            icl32f* pDstValues,  icl32s widthHeight);

        icl32s ippmEigenValuesVectorsSym_m_64f(
            const icl64f* pSrc, icl32s srcStride1, icl32s srcStride2, icl64f* pBuffer,
            icl64f* pDstVectors, icl32s dstStride1, icl32s dstStride2,
            icl64f* pDstValues,  icl32s widthHeight);

        icl32s ippmInvert_m_32f(
            const icl32f* pSrc, icl32s srcStride1, icl32s srcStride2, icl32f* pBuffer,
            icl32f* pDst, icl32s dstStride1, icl32s dstStride2, icl32s widthHeight);

        icl32s ippmInvert_m_64f(
            const icl64f* pSrc, icl32s srcStride1, icl32s srcStride2, icl64f* pBuffer,
            icl64f* pDst, icl32s dstStride1, icl32s dstStride2, icl32s widthHeight);

        icl32s ippmDet_m_32f(
            const icl32f*  pSrc, icl32s srcStride1, icl32s srcStride2, icl32s widthHeight,
            icl32f* pBuffer, icl32f*  pDst);

        icl32s ippmDet_m_64f(
            const icl64f*  pSrc, icl32s srcStride1, icl32s srcStride2, icl32s widthHeight,
            icl64f* pBuffer, icl64f*  pDst);

        /// a getter for the address of an ipp symbol determined by given name
        /**
        This function searches for the address of a symbol and returns a void
        pointer to it.
        When a symbol cant be found this function will thow an exception.
        The symbol addresses, once searched for, will be saved in a map for
        faster recall.

        @param symbol_name The name of the symbol (e.g. function name
               ippmDet_m_32f)
        @param lib_name the name of the shared library (e.g. libippm.so)
               when the string is empty the symbol is searched in all
               (in IppInterface) loaded libs and the first found symbol is
               returned
        @return the address of the symbol (e.g a function pointer)
        **/
        void* ippSymbolPointer(std::string symbol_name, std::string lib_name="");


      private:
        /// private constructor to ensure singleton
        IppInterface();
        ~IppInterface();

        /// A map holding handles to all dynamically loaded libs
        std::map<std::string,void*> m_LibHandles;

        /// A mutex for the function handle map
        Mutex m_FunctionHandleMutex;
        /// A map holding handles to all dynamically loaded functions
        std::map<std::string,void*> m_FunctionHandles;
    };


  } // namespace utils
} // namespace icl

