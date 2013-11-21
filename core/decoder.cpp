#include "coder.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <zbar.h>
#include <cstdlib>
#include <windows.h>


size_t Decoder::get_data(zbar::Image& img, uchar *buf) {
	auto p = img.symbol_begin();
	size_t len = p->get_data_length();
	if (len)
		std::memcpy(buf, p->get_data().c_str(), len);
	return len;
}

size_t Decoder::decode_black(cv::Mat& frame, uchar *buf, int length) {
	cvtColor(frame, frame, CV_BGR2GRAY);

	zbar::Image image(frame.cols, frame.rows, "Y800", frame.data, frame.cols * frame.rows);
	scanner.scan(image);
	return get_data(image, buf);
}