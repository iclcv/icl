// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Viktor Richter, Christof Elbrechter

#include <dlfcn.h>

#include <ICLUtils/IppInterface.h>
#include <ICLUtils/Macros.h>
#include <mutex>

using namespace icl;
using namespace icl::utils;

const char* def_ipp_path_var = "IPP_LIB_DIR";
const char* def_iomp_path_var = "IOMP_LIB_DIR";

const char* def_ipp_search_path = "/vol/nivision/share/IPP/7.07/ipp/lib/intel64/";
const char* def_iomp_search_path = "/vol/nivision/share/IPP/7.07/lib/intel64/";

std::recursive_mutex current_path_lock;
std::string current_ipp_search_path = "";
std::string current_iomp_search_path = "";

void loadLib(const char* path, const char* name, std::map<std::string, void*, std::less<>>& map){
  DEBUG_LOG("Try to load lib" << name << ".so ... ");
  void* lib = dlopen(name, RTLD_LAZY);
  if(!lib){
    ERROR_LOG("Could not load directly, search in path ... ");
    std::stringstream libname;
    libname << path << "/" << name;
    lib = dlopen(libname.str().c_str(), RTLD_LAZY);
  }
  if(!lib){
    char* error = dlerror();
    ERROR_LOG(error);
    throw ICLDynamicLibLoadException(error);
  } else{
    DEBUG_LOG("done.");
    map[name] = lib;
  }
}

void* loadFunction(void* lib, const char* name){
  //clear error
  dlerror();

  // load function
  DEBUG_LOG("Try to load function '" << name << "' ... ");
  void* fn = dlsym(lib, name);

  // fn may be correctly null so check dlerror()
  char* error = dlerror();
  if(error){
    ERROR_LOG(error);
    throw ICLDynamicFunctionLoadException(error);
  } else {
    DEBUG_LOG("done.");
  }
  return fn;
}

IppInterface::IppInterface(){
  std::scoped_lock<std::recursive_mutex> l(current_path_lock);

  DEBUG_LOG("Getting paths from environment.")
      // update ipp search path from environment
      char* ippPath = getenv(def_ipp_path_var);
  if(ippPath){
    DEBUG_LOG("IPP path found: " << ippPath);
    current_ipp_search_path = ippPath;
  } else {
    DEBUG_LOG("IPP path not found");
    if(current_ipp_search_path.empty()){
      current_ipp_search_path = def_ipp_search_path;
    }
    DEBUG_LOG("IPP path set to: " << current_ipp_search_path);
  }

  // update iomp search path from environment
  char* iompPath = getenv(def_iomp_path_var);
  if(iompPath){
    DEBUG_LOG("IOMP path found: " << iompPath);
    current_iomp_search_path = iompPath;
  } else {
    DEBUG_LOG("IOMP path not found");
    if(current_iomp_search_path.empty()){
      current_iomp_search_path = def_iomp_search_path;
    }
    DEBUG_LOG("IOMP path set to: " << current_iomp_search_path);
  }

  // load neccessary libs
  loadLib(current_iomp_search_path.c_str(), "libiomp5.so", m_LibHandles);
  loadLib(current_ipp_search_path.c_str(), "libippcore.so", m_LibHandles);
  loadLib(current_ipp_search_path.c_str(), "libippm.so", m_LibHandles);
  loadLib(current_ipp_search_path.c_str(), "libippi.so", m_LibHandles);
  loadLib(current_ipp_search_path.c_str(), "libipps.so", m_LibHandles);
  loadLib(current_ipp_search_path.c_str(), "libippcv.so", m_LibHandles);
  loadLib(current_ipp_search_path.c_str(), "libippcc.so", m_LibHandles);
}

IppInterface::~IppInterface(){
  std::map<std::string,void*, std::less<>>::iterator it;
  for(it = m_LibHandles.begin(); it != m_LibHandles.end(); ++it){
    dlclose(it->second);
  }
}

IppInterface* IppInterface::get(){
  static IppInterface interface;
  return &interface;
}

icl32s IppInterface::statusNoError(){
  return 0;
}

const char* IppInterface::ippGetStatusString(icl32s StsCode)
{
  static char* (*fn)(icl32s) = reinterpret_cast<char*(*)(icl32s)>(loadFunction(
        m_LibHandles["libippcore.so"], "ippGetStatusString"));
  return fn(StsCode);
}

icl32s IppInterface::ippmEigenValuesVectorsSym_m_32f(
    const icl32f* pSrc, icl32s srcStride1, icl32s srcStride2, icl32f* pBuffer,
    icl32f* pDstVectors, icl32s dstStride1, icl32s dstStride2,
    icl32f* pDstValues,  icl32s widthHeight)
{
  static icl32s (*fn)(const icl32f*,icl32s,icl32s,icl32f*,icl32f*,icl32s,icl32s,icl32f*,icl32s)
      = reinterpret_cast<icl32s(*)(const icl32f*,icl32s,icl32s,icl32f*,icl32f*,icl32s,icl32s,icl32f*,icl32s)>(
      loadFunction(m_LibHandles["libippm.so"], "ippmEigenValuesVectorsSym_m_32f"));
  return fn(pSrc, srcStride1, srcStride2, pBuffer, pDstVectors, dstStride1, dstStride2, pDstValues, widthHeight);
}

icl32s IppInterface::ippmEigenValuesVectorsSym_m_64f(
    const icl64f* pSrc, icl32s srcStride1, icl32s srcStride2, icl64f* pBuffer,
    icl64f* pDstVectors, icl32s dstStride1, icl32s dstStride2,
    icl64f* pDstValues,  icl32s widthHeight)
{
  static icl32s (*fn)(const icl64f*,icl32s,icl32s,icl64f*,icl64f*,icl32s,icl32s,icl64f*,icl32s)
      = reinterpret_cast<icl32s(*)(const icl64f*,icl32s,icl32s,icl64f*,icl64f*,icl32s,icl32s,icl64f*,icl32s)>(
      loadFunction(m_LibHandles["libippm.so"], "ippmEigenValuesVectorsSym_m_64f"));
  return fn(pSrc, srcStride1, srcStride2, pBuffer, pDstVectors, dstStride1, dstStride2, pDstValues, widthHeight);
}

icl32s IppInterface::ippmInvert_m_32f(
    const icl32f* pSrc, icl32s srcStride1, icl32s srcStride2, icl32f* pBuffer,
    icl32f* pDst, icl32s dstStride1, icl32s dstStride2, icl32s widthHeight)
{
  static icl32s (*fn)(const icl32f*,icl32s,icl32s,icl32f*,icl32f*,icl32s,icl32s,icl32s)
      = reinterpret_cast<icl32s(*)(const icl32f*,icl32s,icl32s,icl32f*,icl32f*,icl32s,icl32s,icl32s)>(
      loadFunction(m_LibHandles["libippm.so"], "ippmInvert_m_32f"));
  return fn(pSrc, srcStride1, srcStride2, pBuffer, pDst, dstStride1, dstStride2, widthHeight);
}

icl32s IppInterface::ippmInvert_m_64f(
    const icl64f* pSrc, icl32s srcStride1, icl32s srcStride2, icl64f* pBuffer,
    icl64f* pDst, icl32s dstStride1, icl32s dstStride2, icl32s widthHeight)
{
  static icl32s (*fn)(const icl64f*,icl32s,icl32s,icl64f*,icl64f*,icl32s,icl32s,icl32s)
      = reinterpret_cast<icl32s(*)(const icl64f*,icl32s,icl32s,icl64f*,icl64f*,icl32s,icl32s,icl32s)>(
      loadFunction(m_LibHandles["libippm.so"], "ippmInvert_m_64f"));
  return fn(pSrc, srcStride1, srcStride2, pBuffer, pDst, dstStride1, dstStride2, widthHeight);
}

icl32s IppInterface::ippmDet_m_32f(
    const icl32f*  pSrc, icl32s srcStride1, icl32s srcStride2, icl32s widthHeight,
    icl32f* pBuffer, icl32f*  pDst)
{
  static icl32s (*fn)(const icl32f*,icl32s,icl32s,icl32s,icl32f*,icl32f*)
      = reinterpret_cast<icl32s(*)(const icl32f*,icl32s,icl32s,icl32s,icl32f*,icl32f*)>(
      loadFunction(m_LibHandles["libippm.so"], "ippmDet_m_32f"));
  return fn(pSrc, srcStride1, srcStride2, widthHeight, pBuffer, pDst);
}

icl32s IppInterface::ippmDet_m_64f(
    const icl64f*  pSrc, icl32s srcStride1, icl32s srcStride2, icl32s widthHeight,
    icl64f* pBuffer, icl64f*  pDst)
{
  static icl32s (*fn)(const icl64f*,icl32s,icl32s,icl32s,icl64f*,icl64f*)
      = reinterpret_cast<icl32s(*)(const icl64f*,icl32s,icl32s,icl32s,icl64f*,icl64f*)>(
      loadFunction(m_LibHandles["libippm.so"], "ippmDet_m_64f"));
  return fn(pSrc, srcStride1, srcStride2, widthHeight, pBuffer, pDst);
}

void* IppInterface::ippSymbolPointer(std::string symbol_name, std::string lib_name)
{
  std::scoped_lock<std::recursive_mutex> l(m_FunctionHandleMutex);
  // check if function is already loaded
  if(m_FunctionHandles.contains(symbol_name)){
    return m_FunctionHandles[symbol_name];
  } else {
    if(lib_name.empty()){
      // check all loaded libs
      std::map<std::string,void*, std::less<>>::iterator it;
      for(it = m_LibHandles.begin(); it != m_LibHandles.end(); ++it){
        try{
          void* fn = loadFunction(it->second, symbol_name.c_str());
          m_FunctionHandles[symbol_name] = fn;
          return fn;
        } catch(ICLDynamicFunctionLoadException& e){
          // nothing to do here
        }
      }
      // could not find symbol
      throw ICLDynamicFunctionLoadException(("can not find function " + std::string(symbol_name)).c_str());
    } else {
      // check whether lib is available
      if(!m_LibHandles.contains(lib_name)){
        // load lib
        loadLib(current_ipp_search_path.c_str(), lib_name.c_str(), m_LibHandles);
      }
      // at this point the lib is availabe. load function
      void* fn = loadFunction(m_LibHandles[lib_name], symbol_name.c_str());
      m_FunctionHandles[symbol_name] = fn;
      return fn;
    }
  }
}
