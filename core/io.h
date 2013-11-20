#ifndef IO_H
#define IO_H

#include <opencv2/opencv.hpp>
#include "coder.h"

#define BLACK_MODE

#define QR_SIZE 1172

#ifdef BLACK_MODE
const int MAX_PKT = QR_SIZE / 2;
#else
const int MAX_PKT = QR_SIZE;
#endif

enum class frame_type : unsigned char { INIT, DATA, ACK, EXTRA, MISS };
typedef unsigned short seq_nr;
typedef unsigned char uchar;
const int MAX_DATA = MAX_PKT - sizeof(frame_type) - sizeof(seq_nr);

#pragma pack(push)
#pragma pack(1)

struct frame {
	frame_type type;
	seq_nr seq;
	uchar data[MAX_DATA];
};

#pragma pack(pop)

class IOController {
public:
	IOController(int width, int height);
	void receive(frame &left, frame &right);
	void send(frame &left, frame &right);
	void showfps(double f, int counter);
	void showtime(double time);
private:
	cv::VideoCapture cap;
	cv::Mat screen;
	cv::Mat empty;
	Decoder decoder;
	void split(cv::Mat &m, cv::Mat &l, cv::Mat &r);
};

#endif