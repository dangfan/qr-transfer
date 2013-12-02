#ifndef IO_H
#define IO_H

#include <opencv2/opencv.hpp>
#include "coder.h"

#define VERSION 25

#if VERSION == 16
#define MAX_PKT 586
#elif VERSION == 17
#define MAX_PKT 644
#elif VERSION == 18
#define MAX_PKT 718
#elif VERSION == 19
#define MAX_PKT 792
#elif VERSION == 25
#define MAX_PKT 1273
#elif VERSION == 29
#define MAX_PKT 1628
#elif VERSION == 34
#define MAX_PKT 2188
#endif

#if VERSION <= 19
#define SIZE 10
#elif VERSION == 25
#define SIZE 8
#elif VERSION == 29
#define SIZE 7
#else
#define SIZE 6
#endif

const int WIDTH = (VERSION * 4 + 17) * SIZE;
const int TOP = (1080 - WIDTH) / 2 + 30;
const int LEFT = (1920 - 2 * WIDTH) / 3;
const int LEFT2 = LEFT * 2 + WIDTH;

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
	cv::Mat receive_sync(frame &left, frame &right);
	void send(frame &left, frame &right);
	void showmsg(const char *msg);
	void showtime(double time);
	void set_info(int num_pkts, int size, uchar *data, bool *received);
	void set_pic(cv::Mat &pic);
	void clear();
	bool stop;
	uchar *data;
	bool *received;
private:
	cv::VideoCapture cap;
	cv::Mat screen;
	cv::Mat empty;
	Decoder decoder;
	int num_pkts;
	int size;
	void split(cv::Mat &m, cv::Mat &l, cv::Mat &r);
	void fill_data(frame &f);
};

#endif