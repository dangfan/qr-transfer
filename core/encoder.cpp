#include "libqrcode/qrencode.h"
#include <opencv2/core/core.hpp>
#include "coder.h"

inline void set_color(cv::Mat &m, int r, int c, int size, int color) {
	for (int i = 0; i != size; ++i) {
		for (int j = 0; j != size; ++j) {
			auto p = m.data + 3 * (m.cols * (r * size + i) + c * size + j);
			switch (color) {
			case 0:	// white
				*p = *(p+1) = *(p+2) = 255;
				break;
			case 1: // blue
				*p = 0xff; *(p+1) = 0xaa; *(p+2) = 0;
				break;
			case 2: // red
				*p = 0; *(p+1) = 0x55; *(p+2) = 0xff;
				break;
			case 3: // black
			case 4:
				*p = 0; *(p+1) = 0; *(p+2) = 0;
				break;
			}
		}
	}
}

int encode_color(uchar *buf, size_t length, cv::Mat &m, int size) {
	if (length & 1) return -1;

	length >>= 1;

	auto qrcode_red = QRcode_encodeData(length, buf, 0, QR_ECLEVEL_L);
	auto qrcode_green = QRcode_encodeData(length, buf + length, 0, QR_ECLEVEL_L);
	int width = qrcode_red->width;

	m = cv::Mat(width * size, width * size, CV_8UC3);

	auto pr = qrcode_red->data;
	auto pg = qrcode_green->data;
	for (int r = 0; r != width; ++r) {
		for (int c = 0; c != width; ++c) {
			set_color(m, r, c, size, ((*pr++ & 1) << 1) + (*pg++ & 1));
		}
	}
	
	QRcode_free(qrcode_red);
	QRcode_free(qrcode_green);
	return 0;
}

int encode_black(uchar *buf, size_t length, cv::Mat &m, int size) {
	auto qrcode = QRcode_encodeData(length, buf, 0, QR_ECLEVEL_L);
	int width = qrcode->width;

	m = cv::Mat(width * size, width * size, CV_8UC3);

	auto pr = qrcode->data;
	for (int r = 0; r != width; ++r) {
		for (int c = 0; c != width; ++c) {
			set_color(m, r, c, size, (*pr++ & 1) << 2);
		}
	}
	
	QRcode_free(qrcode);
	return 0;
}