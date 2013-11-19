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
	uchar *tmp = new uchar[length * 2];

	cvtColor(frame, frame, CV_BGR2GRAY);

	zbar::Image image(frame.cols, frame.rows, "Y800", frame.data, frame.cols * frame.rows);
	scanner.scan(image);
	size_t ori_len = get_data(image, tmp);
	if (!ori_len) {
		delete[] tmp;
		return 0;
	}

	LPWSTR wbuf = new WCHAR[length];
	int len = MultiByteToWideChar(CP_UTF8, 0, (LPCCH) tmp, ori_len, wbuf, length);
	if (!len) return 0;

	for (int i = 0; i != len; ++i) {
		buf[i] = (uchar) wbuf[i];
	}

	delete[] tmp;
	delete[] wbuf;
	return len;
}

//Decoder::Decoder(bool debug)
//	: debug(debug) {
//		if (debug) {
//			cv::namedWindow("red", CV_WINDOW_AUTOSIZE);
//			cv::namedWindow("blue", CV_WINDOW_AUTOSIZE);
//		}
//}
//
//void Decoder::get_red_frame(cv::Mat &m) {
//	auto p = m.data;
//	for (int i = 0; i != m.cols * m.rows; ++i) {
//		if (*(p+1) > WHITE_THRESHOLD) *(p+1) = 255;        // Too bright
//		else if (*(p+1) < BLACK_THRESHOLD) *(p+1) = 0;     // Too dark
//		else if (*p < 30 || *p > 150) *(p+1) = 0;          // Red
//		else if (*p > 80) *(p+1) = 255;                    // Blue
//		p += 3;
//	}
//	cvtColor(m, m, CV_HLS2BGR);
//	cvtColor(m, m, CV_BGR2GRAY);
//	cv::medianBlur(m, m, 1);
//	if (debug) cv::imshow("red", m);
//}
//
//void Decoder::get_blue_frame(cv::Mat &m) {
//	auto p = m.data;
//	for (int i = 0; i != m.cols * m.rows; ++i) {
//		if (*(p+1) > WHITE_THRESHOLD) *(p+1) = 255;        // Too bright
//		else if (*(p+1) < BLACK_THRESHOLD) *(p+1) = 0;     // Too dark
//		else if (*p < 30 || *p > 150) *(p+1) = 255;        // Red
//		else if (*p > 80) *(p+1) = 0;                      // Blue
//		p += 3;
//	}
//	cvtColor(m, m, CV_HLS2BGR);
//	cvtColor(m, m, CV_BGR2GRAY);
//	cv::medianBlur(m, m, 1);
//	if (debug) cv::imshow("blue", m);
//}

//size_t Decoder::decode_color(cv::Mat& frame, uchar *buf, int length) {
//	uchar *tmp = new uchar[length * 2];
//
//	cvtColor(frame, frame, CV_BGR2HLS);	
//	auto red = frame.clone();
//	auto blue = frame.clone();
//	get_red_frame(red);
//	get_blue_frame(blue);
//
//	if (debug) cvtColor(frame, frame, CV_HLS2BGR);
//
//	zbar::Image redImage(red.cols, red.rows, "Y800", red.data, red.cols * red.rows);
//	scanner.scan(redImage);
//	size_t len_red = get_data(redImage, tmp);
//	if (!len_red) {
//		delete[] tmp;
//		return 0;
//	}
//
//	zbar::Image blueImage(blue.cols, blue.rows, "Y800", blue.data, blue.cols * blue.rows);
//	scanner.scan(blueImage);
//	size_t len_blue = get_data(blueImage, tmp + len_red);
//	if (!len_blue) {
//		delete[] tmp;
//		return 0;
//	}
//
//	LPWSTR wbuf = new WCHAR[length];
//	int len = MultiByteToWideChar(CP_UTF8, 0, (LPCCH) tmp, len_red + len_blue, wbuf, length);
//	for (int i = 0; i != len; ++i) {
//		buf[i] = (uchar) wbuf[i];
//	}
//
//	delete[] tmp;
//	delete[] wbuf;
//	return len;
//}
