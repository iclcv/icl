#ifndef ICLSONYFWGRABBER_H
#define ICLSONYFWGRABBER_H

#ifdef WIN32

#include <iclGrabber.h>
//#include <iclConverter.h>
#include "iidcapi.h"
#include <iostream>
#include <vector>
#include <string>

#define SONY_GAIN 470
#define SONY_SHUTTER 562
#define SONY_WHITEBALANCE_U 45
#define SONY_WHITEBALANCE_V 12
#define SONY_HUE 15

using namespace std;

namespace icl {

	class SonyFwGrabber : public Grabber {

	public:
		SonyFwGrabber(void);
		SonyFwGrabber(const Size &s, float fFps=30, int iDevice = 0);
		~SonyFwGrabber(void);


		/// grabbing function  
		/** \copydoc icl::Grabber::grab(icl::ImgBase**)  **/    
		virtual const ImgBase* grab(ImgBase **poDst=0);

		/** @{ @name properties and parameters */
    
		/// interface for the setter function for video device properties 
		/** \copydoc icl::Grabber::setProperty(const std::string&,const std::string&) **/
		virtual void setProperty(const std::string &property, const std::string &value) {}

		/// returns a list of properties, that can be set using setProperty
		/** @return list of supported property names **/
		virtual std::vector<std::string> getPropertyList() {std::vector<std::string> s; return s;}

		virtual bool supportsProperty(const std::string &property) { return false; }


		/// get type of property
		/** \copydoc icl::Grabber::getType(const std::string &)*/
		virtual std::string getType(const std::string &name) {return "";}

		/// get information of a property valid values
		/** \copydoc icl::Grabber::getInfo(const std::string &)*/
		virtual std::string getInfo(const std::string &name) {return "";}

		/// returns the current value of a property or a parameter
		virtual std::string getValue(const std::string &name) {return "";}
		/** @} */

		//void GetLeftImage (Img8u *image);
		//void GetRightImage (Img8u *image);
		//void GetStereoImage (Img8u *leftImage, Img8u *rightImage);

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
		BYTE ***m_pppImgBuffer;
		BYTE **m_ppImgBufLeft;
		BYTE **m_ppImgBufRight;
		/// camera handels
		HIIDC m_hCamera[10];
    
		//Img8u *imageLeft;
		//Img8u *imageRight;

		void GetCamAllString(long camIndex, char *strCamera);

	};

}

#endif //WIN32
#endif //SONYFWGRABBER_H