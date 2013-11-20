#include <opencv2/opencv.hpp>
#include <ctime>
#include <cstdio>

using namespace cv;
using namespace std;

int main() {
	int width[] = {640, 1024, 1280}, height[] = {480, 768, 720};

	VideoCapture cap(0);
	Mat frame;
	
	for (int i = 0; i != 3; ++i) {
		cap.set(CV_CAP_PROP_FRAME_WIDTH, width[i]);
		cap.set(CV_CAP_PROP_FRAME_HEIGHT, height[i]);
		int counter = 0;
		time_t start = clock();
		while (true) {
			cap >> frame;
			if (++counter == 100) {
				double fps = 100. / (clock() - start) * CLK_TCK;
				printf("Target: %d*%d Real: %d*%d FPS: %.2f\n", width[i], height[i], frame.cols, frame.rows, fps);
				break;
			}
		}
	}

	system("pause");

	return 0;
}