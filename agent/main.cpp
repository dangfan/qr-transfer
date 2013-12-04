#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <ctime>
#include "coder.h"
#include "io.h"
#include "zlib.h"

#pragma comment(lib, "../core/lib/zdll.lib")

using namespace std;
using namespace cv;

vector<int> lst;
bool ready = false;
int size, num_pkts, fps;
char filename[255];
frame a, b;

void onMouse(int event, int x, int y, int flags, void*) {
	if (event != EVENT_LBUTTONDOWN) return;
	ready = true;
}

void calibrate(IOController &controller) {
	a.type = frame_type::INIT;
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
			num_pkts = a.seq;
			size = *(int *)a.data;
			strcpy(filename, (char *)(a.data + 4));
		}
	}
	controller.clear();
}

void calibrate_with_meta(IOController &controller) {
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

bool finished(bool *r) {
	for (int i = 0; i != num_pkts; ++i)
		if (!r[i]) return false;
	return true;
}

void ack(IOController &controller, bool *r) {
	short *lst = new short[num_pkts];
	short counter = 0;
	for (int i = 0; i != num_pkts; ++i) {
		if (!r[i]) {
			lst[counter++] = i;
		}
	}
	if (counter > MAX_DATA / 2) counter = MAX_DATA / 2;
	a.type = frame_type::ACK;
	a.seq = counter;
	memcpy(a.data, lst, counter * sizeof(short));
	controller.send(a, a);
	delete[] lst;
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

	fps = 1000 / atoi(args[2]);
	int compressed = atoi(args[3]);

	IOController controller(640, 480);
	setMouseCallback("w", onMouse);
	calibrate(controller);
	uchar *data = new uchar[size];
	bool *received = new bool[num_pkts];
	memset(received, 0, num_pkts * sizeof(bool));
	controller.set_info(num_pkts, size, data, received);

	controller.showmsg("Receiving");

	while (!finished(received)) {
		controller.stop = false;
		while (!controller.stop) {
			controller.receive(a, b);
		}
		ack(controller, received);
	}

	uLong bufLen = size * 10;
	uchar *buf = new uchar[bufLen];
	if (compressed) {
		uncompress(buf, &bufLen, data, size);
	} else {
		buf = data;
		bufLen = size;
	}

	char new_name[100];
	strcpy(new_name, args[1]);
	strcat(new_name, filename);
	FILE *file = fopen(new_name, "wb");
	fwrite(buf, sizeof(uchar), bufLen, file);
	fclose(file);

	controller.clear();
	ready = false;
	calibrate_with_meta(controller);
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
				return 0;
			}
		}
	}
}