#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cstdlib>
#include <ppltasks.h>
#include <array>
#include "io.h"
#include "coder.h"

using namespace std;
using namespace cv;
using namespace concurrency;

IOController::IOController(int width, int height)
	: cap(0), screen(1080, 1920, CV_8U), empty(TOP, 1920, CV_8U) {
	cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);
	
	memset(screen.data, 0xff, 1080 * 1920);
	memset(empty.data, 0xff, TOP * 1920);
	namedWindow("w", CV_WINDOW_NORMAL);
	setWindowProperty("w", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	imshow("w", screen);
}

void IOController::split(Mat &m, Mat &l, Mat &r) {
	l = m(Rect(0, 0, m.cols / 2 + 25, m.rows)).clone();
	r = m(Rect(m.cols / 2 - 25, 0, m.cols / 2 + 25, m.rows)).clone();
}

void IOController::set_info(int num_pkts, int size, uchar *data, bool *received) {
	this->num_pkts = num_pkts;
	this->size = size;
	this->data = data;
	this->received = received;
}

void IOController::fill_data(frame &f) {
	if (f.seq == num_pkts - 1) { // last frame
		memcpy(data + f.seq * MAX_DATA, f.data, size - f.seq * MAX_DATA);
	} else { // normal frame
		memcpy(data + f.seq * MAX_DATA, f.data, MAX_DATA);
	}
	received[f.seq] = true;
}

void IOController::receive_sync(frame &left, frame &right) {
	Mat pic, l, r;
	cap >> pic;
	split(pic, r, l);
	if (!decoder.decode(l, (uchar *)&left, MAX_PKT))
		left.type = frame_type::MISS;
	if (!decoder.decode(r, (uchar *)&right, MAX_PKT))
		right.type = frame_type::MISS;
}

void IOController::receive(frame &left, frame &right) {
	Mat pic, l, r;
	cap >> pic;
	split(pic, r, l);

	parallel_invoke(
		[&] {
			if (!decoder.decode(l, (uchar *)&left, MAX_PKT)) return;
			if (left.type == frame_type::DATA) {
				fill_data(left);
			} else if (left.type == frame_type::END) {
				stop = true;
			}
		},
		[&] {
			if (!decoder.decode(r, (uchar *)&right, MAX_PKT)) return;
			if (right.type == frame_type::DATA) {
				fill_data(right);
			} else if (right.type == frame_type::END) {
				stop = true;
			}
		}
	);
}

void IOController::send(frame &left, frame &right) {
	Mat m1, m2, dst1, dst2;
	array<task<void>, 2> tasks = 
	{
		create_task([&] {
			encode((uchar *)&left, MAX_PKT, m1, SIZE);
			dst1 = screen(Rect(LEFT, TOP, m1.cols, m1.rows));
			m1.copyTo(dst1);
		}),
		create_task([&] {
			encode((uchar *)&right, MAX_PKT, m2, SIZE);
			dst2 = screen(Rect(LEFT2, TOP, m2.cols, m2.rows));
			m2.copyTo(dst2);
		})
	};
	auto joinTask = when_all(begin(tasks), end(tasks));
	joinTask.wait();
	imshow("w", screen);
	waitKey(1);
}

void IOController::showmsg(const char *msg) {
	Mat block = screen(Rect(0, 0, 1920, TOP));
	empty.copyTo(block);
	putText(screen, msg, Point(800, 60), 0, 2, Scalar(0), 5);
	imshow("w", screen);
	waitKey(1);
}

void IOController::showtime(double time) {
	char text[10];
	sprintf(text, "%.2fs", time);
	memset(screen.data, 0xff, 1080 * 1920);
	putText(screen, text, Point(800, 400), 0, 5, Scalar(0), 8);
	imshow("w", screen);
	waitKey(1);
}