#ifndef CODER_H
#define CODER_H

#include <opencv2/opencv.hpp>
#include <zbar.h>

class Decoder {
private:
	zbar::ImageScanner scanner;
	size_t get_data(zbar::Image& img, uchar *buf);
public:
	size_t decode_black(cv::Mat& frame, uchar *buf, int len);
};

int encode_black(uchar *buf, size_t length, cv::Mat &m, int size = 10);

#endif