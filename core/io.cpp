#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cstdlib>
#include "io.h"
#include "coder.h"

using namespace std;
using namespace cv;

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
	l = m(Rect(0, 0, m.cols / 2 + 25, m.rows));
	r = m(Rect(m.cols / 2 - 25, 0, m.cols / 2 + 25, m.rows));
}

void IOController::receive(frame &left, frame &right) {
	Mat pic, l, r;
	cap >> pic;
	split(pic, r, l);

	if (!decoder.decode_black(l, (uchar *)&left, MAX_PKT))
		left.type = frame_type::MISS;

	if (!decoder.decode_black(r, (uchar *)&right, MAX_PKT))
		right.type = frame_type::MISS;
}

void IOController::send(frame &left, frame &right) {
	Mat m, dst;
	encode_black((uchar *)&left, MAX_PKT, m);
	dst = screen(Rect(LEFT, TOP, m.cols, m.rows));
	m.copyTo(dst);
	encode_black((uchar *)&right, MAX_PKT, m);
	dst = screen(Rect(LEFT2, TOP, m.cols, m.rows));
	m.copyTo(dst);
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