#include "libqrcode/qrencode.h"
#include <opencv2/core/core.hpp>
#include "coder.h"

inline void set(cv::Mat &m, int r, int c, int size, bool black) {
	for (int i = 0; i != size; ++i) {
		for (int j = 0; j != size; ++j) {
			auto p = m.data + m.cols * (r * size + i) + c * size + j;
			*p = black ? 0 : 255;
		}
	}
}

void encode(uchar *buf, size_t length, cv::Mat &m, int size) {
	auto qrcode = QRcode_encodeData(length, buf, 0, QR_ECLEVEL_L);
	int width = qrcode->width;

	m = cv::Mat(width * size, width * size, CV_8U);

	auto pr = qrcode->data;
	for (int r = 0; r != width; ++r) {
		for (int c = 0; c != width; ++c) {
			set(m, r, c, size, *pr++ & 1);
		}
	}
	
	QRcode_free(qrcode);
}