#include <opencv2/highgui/highgui.hpp>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <vector>
#include <iostream>
#include "coder.h"
#include "io.h"
#include "zlib.h"

#pragma comment(lib, "../core/lib/zdll.lib")

using namespace std;
using namespace cv;

vector<int> lst;
bool ready = false;
int fps;
int compressed;

void onMouse(int event, int x, int y, int flags, void*) {
	if (event != EVENT_LBUTTONDOWN) return;
	ready = true;
}

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

	if (compressed) {
		uLong bufLen;
		uchar *compBuf = new uchar[size * 2];
		compress(compBuf, &bufLen, buf, size);
		uchar *t = buf;
		buf = compBuf;
		size = bufLen;
		num_packets = (int) ceil((double) size / MAX_DATA);
		delete[] t;
	}

	fclose(fd);
	return buf;
}

void calibrate(IOController &controller) {
	frame frame_a, frame_b;
	memset(&frame_a, 0, sizeof(frame));
	frame_a.type = frame_type::INIT;
	controller.send(frame_a, frame_a);

	int counter = 0;
	time_t past[20];
	char text[20];
	memset(past, 0, sizeof(past));
	while (!ready) {
		controller.receive(frame_a, frame_b);
		if (frame_a.type == frame_type::INIT && frame_b.type == frame_type::INIT) {
			sprintf(text, "%.2f fps, %d", 20. / (clock() - past[counter]) * CLK_TCK, counter);
			controller.showmsg(text);
			past[counter] = clock();
			if (++counter == 20) {
				counter = 0;
			}
		}
	}
}

bool send(IOController &controller, uchar *data) {
	if (lst.empty()) return false;

	frame frame_a, frame_b;

	for (vector<int>::iterator p = lst.begin(); p != lst.end(); ++p) {
		frame_a.type = frame_type::DATA;
		frame_a.seq = *p;
		memcpy(frame_a.data, data + *p * MAX_DATA, MAX_DATA);

		if (++p == lst.end()) {
			frame_b.type = frame_type::END;
			controller.send(frame_a, frame_b);
			waitKey(fps);
			return true;
		} else {
			frame_b.type = frame_type::DATA;
			frame_b.seq = *p;
			memcpy(frame_b.data, data + *p * MAX_DATA, MAX_DATA);
			controller.send(frame_a, frame_b);
		}

		waitKey(fps);
	}
	
	frame_a.type = frame_type::END;
	frame_b.type = frame_type::END;
	controller.send(frame_a, frame_b);
	waitKey(fps);
	
	return true;
}

void setList(frame &f) {
	short len = f.seq;
	lst.clear();
	for (int i = 0; i != len; ++i) {
		lst.push_back(*((short *) f.data + i));
	}
}

void send_meta(IOController &controller, int num_pkts, int size, char *filename) {
	frame a, b;
	a.type = frame_type::META;
	a.seq = num_pkts;
	*(int *)a.data = size;
	strcpy((char *) (a.data + 4), filename);
	controller.send(a, a);
	while (true) {
		controller.receive(a, b);
		if (a.type == frame_type::METAACK || b.type == frame_type::METAACK) {
			break;
		}
	}
}

int main(int argc, char* args[]) {
	if (argc != 6) return -1;

	int width = atoi(args[2]);
	int height = atoi(args[3]);
	fps = 1000 / atoi(args[4]);
	compressed = atoi(args[5]);

	IOController controller(width, height);
	setMouseCallback("w", onMouse);
	calibrate(controller);
	
	controller.showmsg("Sending");

	int size, num_packets;
	uchar *data = read_file(args[1], size, num_packets);
	send_meta(controller, num_packets, size, args[1]);
	for (int i = 0; i != num_packets; ++i)
		lst.push_back(i);

	frame frame_a, frame_b;

	while (true) {
		if (!send(controller, data)) break;
		while (true) {
			controller.receive(frame_a, frame_b);
			if (frame_a.type == frame_type::ACK) {
				setList(frame_a);
				break;
			} else if (frame_b.type == frame_type::ACK) {
				setList(frame_b);
				break;
			}
			if (waitKey(1) == 27) {
				delete[] data;
				return 0;
			}
		}
	}
}