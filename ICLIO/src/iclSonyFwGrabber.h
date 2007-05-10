#ifndef ICLSONYFWGRABBER_H
#define ICLSONYFWGRABBER_H

#ifdef WIN32

#include <iclGrabber.h>
//#include <iclConverter.h>
#include <sony/iidcapi.h>
#include <iostream>
#include <vector>
#include <string>

#define SONY_GAIN 470
#define SONY_SHUTTER 562
#define SONY_WHITEBALANCE_U 45
#define SONY_WHITEBALANCE_V 12
#define SONY_HUE 15

#define GET_FORMAT(formatindex)			((formatindex>>8)&0x07)
#define GET_MODE(formatindex)			((formatindex>>12)&0x07)
#define GET_COLORCODING(formatindex)	(formatindex & 0x0f)

using namespace std;

namespace icl {

	class SonyFwGrabber : public Grabber {

	public:
		SonyFwGrabber(void);
		~SonyFwGrabber(void);

		bool init();

		/// grabbing function  
		/** \copydoc icl::Grabber::grab(icl::ImgBase**)  **/    
		virtual const ImgBase* grab(ImgBase **poDst=0);

		//void GetLeftImage (Img8u *image);
		//void GetRightImage (Img8u *image);
		void GetStereoImage (ImgBase **poDstLeft, ImgBase **poDstRight);
		//void GetStereoImageTrigger (Img8u *leftImage, Img8u *rightImage);

		/** @{ @name properties and parameters */
    
		/// interface for the setter function for video device properties 
		/** \copydoc icl::Grabber::setProperty(const std::string&,const std::string&) **/
		virtual void setProperty(const std::string &property, const std::string &value);

		/// returns a list of properties, that can be set using setProperty
		/** @return list of supported property names **/
		virtual std::vector<std::string> getPropertyList();

		/// returns the current value of a property or a parameter
		virtual std::string getValue(const std::string &name);
		/** @} */

		icl::Size getSize() { return icl::Size(m_iWidth, m_iHeight); }

	private:
		/// current grabbing width
		int m_iWidth;
		/// current grabbing height
		int m_iHeight;
		/// current grabbing device
		int m_iDevice;
		/// current grabbing fps value
		float m_fFps;
		/// current number of devices
		long m_lNumCameras;
		/// current Img8u image buffer
		//Img8u *m_poRGB8Image;
		//Img8u *imageLeft;
		//Img8u *imageRight;
		BYTE ***m_pppImgBuffer;
	
		/// camera handels
		HIIDC m_hCamera[10];

		void GetCamAllString(long camIndex, char *strCamera);

	};

}

#endif //WIN32
#endif //SONYFWGRABBER_H