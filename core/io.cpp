#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cstdlib>
#include "io.h"
#include "coder.h"

using namespace std;
using namespace cv;

IOController::IOController(int width, int height)
	: cap(0), screen(1080, 1920, CV_8U), empty(135, 1920, CV_8U) {
	cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);
	
	memset(screen.data, 0xff, 1080 * 1920);
	memset(empty.data, 0xff, 135 * 1920);
	namedWindow("w", CV_WINDOW_NORMAL);
	setWindowProperty("w", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	imshow("w", screen);
}

void IOController::split(Mat &m, Mat &l, Mat &r) {
	l = m(Rect(0, 0, m.cols / 2, m.rows));
	r = m(Rect(m.cols / 2, 0, m.cols / 2, m.rows));
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
	encode_black((uchar *)&left, MAX_PKT, m, 10);
	dst = screen(Rect(100, 135, m.cols, m.rows));
	m.copyTo(dst);
	encode_black((uchar *)&right, MAX_PKT, m, 10);
	dst = screen(Rect(1000, 135, m.cols, m.rows));
	m.copyTo(dst);
	imshow("w", screen);
}

void IOController::showfps(double fps) {
	char text[10];
	sprintf(text, "%.2f fps", fps);
	Mat block = screen(Rect(0, 0, 1920, 135));
	empty.copyTo(block);
	putText(screen, text, Point(800, 100), 0, 3, Scalar(0), 5);
	imshow("w", screen);
}