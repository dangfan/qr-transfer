#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include "coder.h"

using namespace std;
using namespace cv;
using namespace Coder;

const int width = 640;
const int height = 480;

VideoCapture get_cap() {
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		return -1;
	}
	cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);
	return cap;
}

void split(Mat& m, Mat& l, Mat& r) {
	l = m(Rect(0, 0, width / 2, height));
	r = m(Rect(width / 2, 0, width / 2, height));
}

int main() {
	VideoCapture cap = get_cap();

	// set up window
	Mat region(1080, 1920, CV_8UC3);
	namedWindow("receiver", CV_WINDOW_NORMAL);
	setWindowProperty("receiver", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

	// set up buffers
	Mat frame, left, right, m;
	Decoder decoder(false);
	uchar buf[2000];
	int size, num_packets;

	// wait for ready
	while (true) {
		cap >> frame;
		split(frame, left, right);
		if (decoder.decode_black(left, buf, 2000)) {
			if (*buf == TYPE_INIT) {
				num_packets = *((short *) (buf + TYPE));
				size = *((int *) (buf + HEADER));
				break;
			}
		}
	}

	// send ack
	memset(buf, 0, PACK);
	*((short *) buf) = TYPE_ACK;
	encode(buf, PACK, m, 10);
	Mat dst = region(Rect(500, 135, m.cols, m.rows));
	m.copyTo(dst);
	putText(region, "READY", Point(800, 100), 0, 3, Scalar(0,0,0), 5);
	imshow("receiver", region);

	// create file buffer
	uchar *data = new uchar[size];
	bool *result = new bool[num_packets];
	memset(resize, 0, num_packets * sizeof(bool));
	
	// receive data
	while (true) {
		cap >> frame;
		split(frame, left, right);
		if (decoder.decode_black(left, buf, 2000)) {
			short seq = *((short *) (buf + TYPE));
			if (seq == num_packets - 1) {
				memcpy(data + seq * DATA, buf + TYPE, size - seq * DATA);
			} else {
				memcpy(data + seq * DATA, buf + TYPE, DATA);
			}
			result[seq] = true;
		}
		if (decoder.decode_black(right, buf, 2000)) {
			if (*buf == TYPE_LAST_EXTRA) break;
			short seq = *((short *) (buf + TYPE));
			if (seq == num_packets - 1) {
				memcpy(data + seq * DATA, buf + TYPE, size - seq * DATA);
			} else {
				memcpy(data + seq * DATA, buf + TYPE, DATA);
			}
			result[seq] = true;
		}
	}

	for (int i = 0; i != num_packets; ++i) {
		cout << i << " " << result[i] << endl;
	}

	delete[] data;
	delete[] result;
	return 0;
}