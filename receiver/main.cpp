#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include "coder.h"

using namespace std;
using namespace cv;
using namespace Coder;

int main() {
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		return -1;
	}
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

	namedWindow("captured", CV_WINDOW_AUTOSIZE);

	Decoder decoder(true);
	uchar buf[2000];
	
	while (true) {
		Mat frame, channels[3];
		cap >> frame;

		if (int l = decoder.decode_black(frame, buf, 2000)) {
			if (!*buf) {
				int size = *((int*)(buf+1));
				cout << "READY\n";
			}
			else {
				short seq = *((short *)(buf+1));
				cout << seq << endl;
			}
			//for (int i = 0; i != l; ++i)
			//	cout << (int)buf[i] << endl;
			//cout << endl;
			//cout << l;
		}

		imshow("captured", frame);

		waitKey(10);
	}
	return 0;
}