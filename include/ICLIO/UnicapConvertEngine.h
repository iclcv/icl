/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/UnicapConvertEngine.h                    **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_Unicap_CONVERT_ENGINE_H
#define ICL_Unicap_CONVERT_ENGINE_H
#include <ICLCore/Types.h>
#include <ICLCore/ImgParams.h>
namespace icl{
  /** \cond */
  class UnicapDevice;
  /** \endcond */
  
  /// Interface class for unicap convert engines \ingroup UNICAP_G
  struct UnicapConvertEngine{
    
    /// Default constructor (just saving the device)
    UnicapConvertEngine(UnicapDevice *device):m_poDevice(device){}
    
    /// Empty destructor
    virtual ~UnicapConvertEngine(){}
    
    /// Conversion function
    /** This function must be implemented for each specific convert engine class. The raw data must be converted
        into the destination image. If possible, the desired params and depth should be regarded. If not, the 
        parent UnicapGrabber will perform the conversion using a Converter 
        @param rawData incomming raw data of current image
        @param desiredParams params (size, format,...) for the result image
        @param desiredDepth depth for the result image
        @param ppoDst destination image pointer-pointer. This must be adapted to the image params that are actually
                      produce by this converter 
    **/
    virtual void cvt(const icl8u *rawData, const ImgParams &desiredParams, depth desiredDepth, ImgBase **ppoDst)=0;

    /// returns whether this engine is able to provide given paramters
    virtual bool isAbleToProvideParams(const ImgParams &desiredParams, depth desiredDepth) const = 0;
    
    /// converts given frame to native sized rgb depth8u image
    virtual void cvtNative(const icl8u *rawData, ImgBase **ppoDst);
    protected:
    
    /// internal storage of the associated UnicapDevice
    UnicapDevice *m_poDevice;
  };
}

#endif
