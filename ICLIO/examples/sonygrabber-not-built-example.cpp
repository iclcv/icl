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

#include <ICLIO/FileWriter.h>
#include <ICLIO/SonyFwGrabber.h>

using namespace std;
using namespace icl;

int main(int n, char **ppc) {
#ifdef WIN32
#ifdef WITH_SONYIIDC
	SonyFwGrabber *grabber = new SonyFwGrabber();
	if (!grabber->init()) {
		cout << "init failed!" << endl;
		Sleep(5000);
	}

	FileWriter writer("sony_bayer.pgm");

	const ImgBase* img = grabber->grab();
	writer.write(img);

	ImgBase *left, *right;
	left = 0;
	right = 0;
	
	grabber->grabStereo(&left, &right);
	writer.setFileName("left_0.pgm");
	writer.write(left);
	writer.setFileName("right_0.pgm");
	writer.write(right);

	grabber->grabStereo(&left, &right);
	writer.setFileName("left_1.pgm");
	writer.write(left);
	writer.setFileName("right_1.pgm");
	writer.write(right);

	delete grabber;

	return 0;
#endif
#endif

}

