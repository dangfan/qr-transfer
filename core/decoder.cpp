#include "coder.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <zbar.h>
#include <cstdlib>
#include <iostream>
#include <windows.h>

namespace Coder {
	Decoder::Decoder(bool debug)
		: debug(debug) {
			if (debug) {
				cv::namedWindow("red", CV_WINDOW_AUTOSIZE);
				cv::namedWindow("green", CV_WINDOW_AUTOSIZE);
			}
	}

	void Decoder::get_red_frame(cv::Mat &m) {
		auto p = m.data;
		for (int i = 0; i != m.cols * m.rows; ++i) {
			if (*(p+1) > WHITE_THRESHOLD) *(p+1) = 255;
			else if (*p < 30 || *p > 150) *(p+1) = 0; //R
			else if (*p > 30  && *p < 80) *(p+1) = 255; //G
			else if (*(p+1) < 80) *(p+1) = 0; //B
			p += 3;
		}
		cvtColor(m, m, CV_HLS2BGR);
		if (debug) cv::imshow("red", m);
		cvtColor(m, m, CV_BGR2GRAY);
	}

	void Decoder::get_green_frame(cv::Mat &m) {
		auto p = m.data;
		for (int i = 0; i != m.cols * m.rows; ++i) {
			if (*(p+1) > WHITE_THRESHOLD) *(p+1) = 255;
			else if (*p < 30 || *p > 150) *(p+1) = 255; //R
			else if (*p > 30  && *p < 80) *(p+1) = 0; //G
			else if (*(p+1) < 80) *(p+1) = 0; //B
			p += 3;
		}
		cvtColor(m, m, CV_HLS2BGR);
		if (debug) cv::imshow("green", m);
		cvtColor(m, m, CV_BGR2GRAY);
	}

	size_t Decoder::get_data(zbar::Image& img, uchar *buf) {
		auto p = img.symbol_begin();
		size_t len = p->get_data_length();
		if (len)
			std::memcpy(buf, p->get_data().c_str(), len);
		return len;
	}

	size_t Decoder::decode(cv::Mat& frame, uchar *buf, int length) {
		uchar *tmp = new uchar[length * 2];

		cvtColor(frame, frame, CV_BGR2HLS);	
		auto red = frame.clone();
		auto green = frame.clone();
		get_red_frame(red);
		get_green_frame(green);

		if (debug) cvtColor(frame, frame, CV_HLS2BGR);

		zbar::Image redImage(red.cols, red.rows, "Y800", red.data, red.cols * red.rows);
		scanner.scan(redImage);
		size_t len_red = get_data(redImage, tmp);
		if (!len_red) {
			delete[] tmp;
			return 0;
		}

		zbar::Image greenImage(green.cols, green.rows, "Y800", green.data, green.cols * green.rows);
		scanner.scan(greenImage);
		size_t len_green = get_data(greenImage, tmp + len_red);
		if (!len_green) {
			delete[] tmp;
			return 0;
		}

		LPWSTR wbuf = new WCHAR[length];
		int len = MultiByteToWideChar(CP_UTF8, 0, (LPCCH) tmp, len_red + len_green, wbuf, length);
		if (!len) return 0;

		for (int i = 0; i != len; ++i) {
			buf[i] = (uchar) wbuf[i];
		}

		delete[] tmp;
		delete[] wbuf;
		return len;
	}

	size_t Decoder::decode_black(cv::Mat& frame, uchar *buf, int length) {
		uchar *tmp = new uchar[length * 2];

		cvtColor(frame, frame, CV_BGR2GRAY);	

		zbar::Image image(frame.cols, frame.rows, "Y800", frame.data, frame.cols * frame.rows);
		scanner.scan(image);
		size_t len_red = get_data(image, tmp);
		if (!len_red) {
			delete[] tmp;
			return 0;
		}

		LPWSTR wbuf = new WCHAR[length];
		int len = MultiByteToWideChar(CP_UTF8, 0, (LPCCH) tmp, len_red, wbuf, length);
		if (!len) return 0;

		for (int i = 0; i != len; ++i) {
			buf[i] = (uchar) wbuf[i];
		}

		delete[] tmp;
		delete[] wbuf;
		return len;
	}
}