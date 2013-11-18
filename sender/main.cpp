#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "coder.h"
#include <cstdio>
#include <cmath>

using namespace std;
using namespace cv;
using namespace Coder;

const int EMPTY_LEN = 1;
const int SEQ_LEN = 2;
const int LEN_LEN = 4;
const int HEADER_LEN = EMPTY_LEN + SEQ_LEN + LEN_LEN;
const int PACK_LEN = 1172 / 2;
const int DATA_LEN = PACK_LEN - HEADER_LEN;

uchar *read_file(const char* filename, int& size, int& num_packets) {
	FILE *fd = fopen(filename, "rb");
	
	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	num_packets = (int) ceil((double) size / DATA_LEN);
	uchar *buf =  new uchar[DATA_LEN * num_packets];
	memset(buf, 0, DATA_LEN * num_packets);
	
	for (int i = 0; i != num_packets; ++i) {
		fread(buf + (i * DATA_LEN), sizeof(uchar), DATA_LEN, fd);
	}

	fclose(fd);
	return buf;
}

int main() {
	int size, num_packets;
	uchar *data = read_file("msys.ico", size, num_packets);
	Mat region(1080, 1920, CV_8UC3), m;
	memset(region.data, 255, 1920*1080*3);

	uchar buf[PACK_LEN];
	memset(buf, 0, PACK_LEN);
	encode_black(buf, PACK_LEN, m, 10);

	Mat dst = region(Rect(500, 135, m.cols, m.rows));
	m.copyTo(dst);
	
	namedWindow("sender", CV_WINDOW_NORMAL);
	setWindowProperty("sender", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	imshow("sender", region);

	waitKey(0);

	for (int i = 0; i < num_packets; ++i) {
		memset(region.data, 255, 1920*1080*3);

		// left frame
		*buf = 1;
		*((short *) (buf + EMPTY_LEN)) = (short) i;
		*((int *) (buf + EMPTY_LEN + SEQ_LEN)) = size;
		memcpy(buf + HEADER_LEN, data + (i * DATA_LEN), DATA_LEN);
		encode_black(buf, PACK_LEN, m, 10);
		dst = region(Rect(500, 135, m.cols, m.rows));
		//Mat dst = region(Rect(100, 135, m.cols, m.rows));
		m.copyTo(dst);

		//// right frame
		//if (++i == num_packets) { // no extra frame
		//	memset(buf, 0, PACK_LEN);
		//} else {
		//	*((short *) buf) = (short) i;
		//	*((int *) (buf+SEQ_LEN)) = size;
		//	memcpy(buf + HEADER_LEN, data + (i * DATA_LEN), DATA_LEN);
		//}
		//encode(buf, PACK_LEN, m, 10);
		//dst = region(Rect(1000, 135, m.cols, m.rows));
		//m.copyTo(dst);
	
		setWindowProperty("sender", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
		imshow("sender", region);

		if (waitKey(200) == 27) break;
	}

	waitKey(0);
	delete[] data;
	return 0;
}