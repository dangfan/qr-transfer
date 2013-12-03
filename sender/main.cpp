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
int fps, compressed;
int size, num_pkts;
frame a, b;
uchar *data;
char *filename;

void onMouse(int event, int x, int y, int flags, void*) {
	if (event != EVENT_LBUTTONDOWN) return;
	ready = true;
}

void read_file() {
	FILE *fd = fopen(filename, "rb");
	
	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	num_pkts = (int) ceil((double) size / MAX_DATA);
	uchar *buf =  new uchar[MAX_DATA * num_pkts];
	memset(buf, 0, MAX_DATA * num_pkts);
	
	for (int i = 0; i != num_pkts; ++i) {
		fread(buf + (i * MAX_DATA), sizeof(uchar), MAX_DATA, fd);
	}

	if (compressed) {
		uLong bufLen;
		uchar *compBuf = new uchar[size * 2];
		compress(compBuf, &bufLen, buf, size);
		uchar *t = buf;
		buf = compBuf;
		size = bufLen;
		num_pkts = (int) ceil((double) size / MAX_DATA);
		delete[] t;
	}

	fclose(fd);
	data = buf;
}

void calibrate(IOController &controller) {
	a.type = frame_type::INIT;
	a.seq = num_pkts;
	*(int *)a.data = size;
	strcpy((char *) (a.data + 4), filename);
	controller.send(a, a);

	int counter = 0;
	time_t past[20];
	char text[20];
	memset(past, 0, sizeof(past));
	while (!ready) {
		Mat pic = controller.receive_sync(a, b);
		controller.set_pic(pic);
		if (a.type == frame_type::INIT && b.type == frame_type::INIT) {
			sprintf(text, "%.2f fps, %d", 20. / (clock() - past[counter]) * CLK_TCK, counter);
			controller.showmsg(text);
			past[counter] = clock();
			if (++counter == 20) {
				counter = 0;
			}
		}
	}
	controller.clear();
}

bool send(IOController &controller, uchar *data) {
	if (lst.empty()) return false;

	for (auto p = lst.begin(); p != lst.end(); ++p) {
		a.type = frame_type::DATA;
		a.seq = *p;
		memcpy(a.data, data + *p * MAX_DATA, MAX_DATA);

		if (++p == lst.end()) {
			b.type = frame_type::END;
			controller.send(a, b);
			waitKey(fps);
			return true;
		} else {
			b.type = frame_type::DATA;
			b.seq = *p;
			memcpy(b.data, data + *p * MAX_DATA, MAX_DATA);
			controller.send(a, b);
		}

		waitKey(fps);
	}
	
	a.type = frame_type::END;
	b.type = frame_type::END;
	controller.send(a, b);
	waitKey(fps);
	
	return true;
}

void setList(frame &f) {
	lst.clear();
	short len = f.seq;
	for (int i = 0; i != len; ++i) {
		lst.push_back(*((short *) f.data + i));
	}
}

int main(int argc, char* args[]) {
	if (argc != 4) return -1;
	filename = args[1];
	fps = 1000 / atoi(args[2]);
	compressed = atoi(args[3]);
	read_file();

	IOController controller(640, 480);
	setMouseCallback("w", onMouse);

process:
	controller.clear();
	ready = false;
	calibrate(controller);
	controller.showmsg("Sending");
	lst.clear();
	for (int i = 0; i != num_pkts; ++i)
		lst.push_back(i);

	while (true) {
		if (!send(controller, data)) break;
		while (true) {
			controller.receive(a, b);
			if (a.type == frame_type::ACK) {
				setList(a);
				break;
			} else if (b.type == frame_type::ACK) {
				setList(b);
				break;
			}
			if (waitKey(1) == 32) {
				goto process;
			}
		}
	}
}