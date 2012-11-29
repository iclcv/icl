/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/PCDFileGrabber.h                       **
** Module : ICLGeom                                                **
** Authors: Patrick Nobou                                          **
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

#pragma once

#include <ICLGeom/PointCloudGrabber.h>


namespace icl{

  namespace geom{

  class PCDFileGrabber : public PointCloudGrabber{
      
    public:
    /// Default constructor
    PCDFileGrabber();

    /// creates a new PCD file grabber instance
    /** @param filename to be grabbed PCD file name or file pattern. (e.g. files/ *.pcd)
        @param repeat specifies whether to play PCD file in an endless loop or not.
        @param timestamp time stamp to render the files.
        @param forceExactPCLType if this flag is set to true, the PointCloudObjectBase-reference
        given to grab must have exactly the same fiels as the pcl-file
        @param offset Similar to the offset variable in the file pcd_io.h.
        
        
        Comment from pcl: offset of where to expect the PCD Header in the file (optional parameter).
        One usage example for setting the offset parameter is for reading data from a
        TAR "archive containing multiple PCD files: TAR files always add a 512 byte header
        in front of the actual file, so set the offset to the next byte after the header (e.g., 513).
        
        TODO: timestampe und timestamp feature raus!
        */
    PCDFileGrabber(const std::string &filepattern, bool loop = true,
                   bool forceExactPCLType = false, const int offset = 0);
    
    /// creates a new PCD file grabber instance TODO: ganz raus!
    /** @param filename to be grabbed PCD file name or file pattern. (e.g. files/ *.pcd)
        @param offset Similar to the offset variable in the file pcd_io.h.
        Here repeat is always true and timestamp is 0
        */
    //PCDFileGrabber(const std::string &filename, const int offset = 0);
    
    /// Destructor
    virtual ~PCDFileGrabber() throw();
    
    /// grab implementation
    virtual void grab(PointCloudObjectBase &dst);
    
    private:
    struct Data;  // !< pimpl type
    Data *m_data; // !< pimpl pointer
    
    ///Help function to extract the pointtype in the loaded file header.
    /** Its return as string with the point type. E.g.: Pointxyz */
    std::vector<std::string> getFields() const;
    };
  }
}
