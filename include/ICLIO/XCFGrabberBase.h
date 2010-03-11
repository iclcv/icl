/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifdef HAVE_XCF
#ifndef ICL_XCF_GRABBER_BASE_H
#define ICL_XCF_GRABBER_BASE_H

#include <ICLIO/Grabber.h>
#include <string>
#include <xmltio/Location.hpp>
#include <xcf/CTU.hpp>
#include <ICLCC/Bayer.h>

namespace icl {

  

   /// Grabber to access XCF Image Server and XCF Publisher \ingroup GRABBER_G
   /** XCFGrabberBase serves as a base class to XCFServerGrabber and 
       XCFPublisherGrabber. 
                  1GBit           100MBit
       Server     48 Hz           10 Hz
       Publisher  69 Hz           12 Hz
   */
  
   class XCFGrabberBase : public Grabber {
   public:
    
      /// Base constructor
    XCFGrabberBase();

    /// Destructor
    ~XCFGrabberBase();
    
    /// grabbing function  
    /** \copydoc icl::Grabber::grab(ImgBase**)*/
    virtual const ImgBase* grabUD(ImgBase **ppoDst=0);
      
    /// grabbing a whole image set, e.g. from a stereo camera
    /** The vector of images is resized to match the number of grabbed
          images. Existing images in the vector are adapted to the desired
        output depth and parameters. Ownership for all grabbed images goes
        to the caller! 
     */
    void grab (std::vector<ImgBase*>& vGrabbedImages);

    /// just imported from parent class
    Grabber::grab;
  
    /// TODO: use internally
    void setIgnoreDesiredParams(bool flag){
      m_bIgnoreDesired = flag;
    }
    
    /// TODO: use internally
    bool getIgnoreDesiredParams() const { 
      return m_bIgnoreDesired; 
    }
    
    protected:   
    /// retrieve most current image set in provided composite transport unit
    virtual void receive (XCF::CTUPtr& result) = 0;
    
    private:
    void makeOutput (const xmltio::Location& l, ImgBase* poOutput);
      
    XCF::CTUPtr          m_result;
    ImgBase*             m_poSource;
    BayerConverter*	 m_poBayerConverter;
    ImgBase*             m_poBayerBuffer;
    ImgBase*             m_poDesiredParamsBuffer;
    bool                 m_bIgnoreDesired;
  };
  
}

#endif
#endif // HAVE_XCF
