#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "coder.h"
#include <cstdio>
#include <cmath>

using namespace std;
using namespace cv;
using namespace Coder;

uchar *read_file(const char* filename, int& size, int& num_packets) {
	FILE *fd = fopen(filename, "rb");
	
	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	num_packets = (int) ceil((double) size / DATA);
	uchar *buf =  new uchar[DATA * num_packets];
	memset(buf, 0, DATA * num_packets);
	
	for (int i = 0; i != num_packets; ++i) {
		fread(buf + (i * DATA), sizeof(uchar), DATA, fd);
	}

	fclose(fd);
	return buf;
}

int main() {
	int size, num_packets;
	uchar *data = read_file("msys.ico", size, num_packets);
	Mat region(1080, 1920, CV_8UC3), m;
	memset(region.data, 255, 1920*1080*3);
	uchar buf[PACK];

	// init frame
	memset(buf, 0, PACK);
	*buf = TYPE_INIT;
	*((short *) (buf + TYPE)) = (short) num_packets;
	*((int *) (buf + HEADER)) = size;
	encode(buf, PACK, m, 10);
	Mat dst = region(Rect(100, 135, m.cols, m.rows));
	m.copyTo(dst);
	
	namedWindow("sender", CV_WINDOW_NORMAL);
	setWindowProperty("sender", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	imshow("sender", region);

	waitKey(0);

	for (int i = 0; i < num_packets; ++i) {
		memset(region.data, 255, 1920*1080*3);

		// left frame
		*buf = TYPE_DATA;
		*((short *) (buf + TYPE)) = (short) i;
		memcpy(buf + HEADER, data + (i * DATA), DATA);
		encode(buf, PACK, m, 10);
		//dst = region(Rect(500, 135, m.cols, m.rows));
		dst = region(Rect(100, 135, m.cols, m.rows));
		m.copyTo(dst);

		// right frame
		if (++i == num_packets) { // no extra frame
			memset(buf, 0, PACK);
			*((short *) buf) = TYPE_LAST_EXTRA;
		} else {
			*buf = TYPE_DATA;
			*((short *) (buf + TYPE)) = (short) i;
			memcpy(buf + HEADER, data + (i * DATA), DATA);
		}
		encode(buf, PACK, m, 10);
		dst = region(Rect(1000, 135, m.cols, m.rows));
		m.copyTo(dst);
	
		setWindowProperty("sender", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
		imshow("sender", region);

		if (waitKey(100) == 27) break;
	}

	waitKey(0);
	delete[] data;
	return 0;
}