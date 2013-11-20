#ifndef IO_H
#define IO_H

#include <opencv2/opencv.hpp>
#include "coder.h"

#define BLACK_MODE
#define VERSION 19

#if VERSION == 16
#define QR_SIZE 586
#elif VERSION == 17
#define QR_SIZE 644
#elif VERSION == 18
#define QR_SIZE 718
#elif VERSION == 19
#define QR_SIZE 792
#endif

const int WIDTH = (VERSION * 4 + 17) * 10;
const int TOP = (1080 - WIDTH) / 2 + 20;
const int LEFT = (1920 - 2 * WIDTH) / 3;
const int LEFT2 = LEFT * 2 + WIDTH;

#ifdef BLACK_MODE
const int MAX_PKT = QR_SIZE;
#else
const int MAX_PKT = QR_SIZE * 2;
#endif

enum class frame_type : unsigned char { INIT, DATA, ACK, END, MISS, META, METAACK };
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
	void showmsg(const char *msg);
	void showtime(double time);
private:
	cv::VideoCapture cap;
	cv::Mat screen;
	cv::Mat empty;
	Decoder decoder;
	void split(cv::Mat &m, cv::Mat &l, cv::Mat &r);
};

#endif