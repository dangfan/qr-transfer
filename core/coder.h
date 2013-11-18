#ifndef CODER_H
#define CODER_H

#include <opencv2/opencv.hpp>
#include <zbar.h>

#define WHITE_THRESHOLD 150

namespace Coder {
	class Decoder {
	private:
		bool debug;
		zbar::ImageScanner scanner;
		void get_red_frame(cv::Mat &m);
		void get_green_frame(cv::Mat &m);
		size_t get_data(zbar::Image& img, uchar *buf);
	public:
		Decoder(bool debug = false);
		size_t decode(cv::Mat& frame, uchar *buf, int len);
		size_t decode_black(cv::Mat& frame, uchar *buf, int len);
	};
}

int encode(uchar *buf, size_t length, cv::Mat &m, int size = 10);
int encode_black(uchar *buf, size_t length, cv::Mat &m, int size = 10);

#endif