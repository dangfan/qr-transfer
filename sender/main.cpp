#include <opencv2/highgui/highgui.hpp>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <vector>
#include "coder.h"
#include "io.h"

using namespace std;
using namespace cv;

vector<int> lst;

uchar *read_file(const char *filename, int &size, int &num_packets) {
	FILE *fd = fopen(filename, "rb");
	
	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	num_packets = (int) ceil((double) size / MAX_DATA);
	uchar *buf =  new uchar[MAX_DATA * num_packets];
	memset(buf, 0, MAX_DATA * num_packets);
	
	for (int i = 0; i != num_packets; ++i) {
		fread(buf + (i * MAX_DATA), sizeof(uchar), MAX_DATA, fd);
	}

	fclose(fd);
	return buf;
}

void connect(IOController &controller, short num_packets, int size) {
	frame frame_a, frame_b;
	frame_a.type = frame_type::INIT;
	frame_a.seq = num_packets;
	*(int *)frame_a.data = size;
	frame_b.type = frame_type::INIT;
	controller.send(frame_a, frame_b);

	int counter = 0;
	time_t start, end;
	time(&start);
	while (true) {
		controller.receive(frame_a, frame_b);
		if (frame_a.type == frame_type::INIT && frame_b.type == frame_type::INIT) {
			if (++counter == 10) {
				time(&end);
				controller.showfps(difftime(end, start));
				counter = 0;
				start = end;
			}
		}
		if (waitKey(10) == 32) break;
	}
}

void send(IOController &controller, uchar *data) {
	for (vector<int>::iterator p = lst.begin(); p != lst.end(); ++p) {
		frame frame_a, frame_b;
		frame_a.type = frame_type::DATA;
		frame_a.seq = *p;
		memcpy(frame_a.data, data + *p * MAX_DATA, MAX_DATA);

		if (++p == lst.end()) {
			frame_b.type = frame_type::EXTRA;
			controller.send(frame_a, frame_b);
			return;
		} else {
			frame_b.type = frame_type::DATA;
			frame_b.seq = *p;
			memcpy(frame_b.data, data + *p * MAX_DATA, MAX_DATA);
			controller.send(frame_a, frame_b);
		}

		waitKey(100);
	}
}

int main() {
	int size, num_packets;
	uchar *data = read_file("msys.ico", size, num_packets);
	IOController controller(640, 480);
	connect(controller, num_packets, size);
	waitKey(0);

	for (int i = 0; i != num_packets; ++i)
		lst.push_back(i);

	while (true) {
		send(controller, data);
		break;
	}

	delete[] data;
	return 0;
}