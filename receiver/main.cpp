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

bool ready = false;
int size, num_pkts;
char filename[255];
frame frame_a, frame_b;

void onMouse(int event, int x, int y, int flags, void*) {
	if (event != EVENT_LBUTTONDOWN) return;
	ready = true;
}

void calibrate(IOController &controller) {
	frame_a.type = frame_type::INIT;
	controller.send(frame_a, frame_a);

	int counter = 0;
	time_t past[20];
	char text[20];
	memset(past, 0, sizeof(past));
	while (!ready) {
		Mat pic = controller.receive_sync(frame_a, frame_b);
		controller.set_pic(pic);
		if (frame_a.type == frame_type::INIT && frame_b.type == frame_type::INIT) {
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
	frame frame_ack;
	frame_ack.type = frame_type::ACK;
	frame_ack.seq = counter;
	memcpy(frame_ack.data, lst, counter * sizeof(short));
	controller.send(frame_ack, frame_ack);
	delete[] lst;
}

int main(int argc, char* args[]) {
	if (argc != 3) return -1;

	int compressed = atoi(args[2]);

	IOController controller(640, 480);
	setMouseCallback("w", onMouse);
	calibrate(controller);

	controller.showmsg("Ready to receive");

	while (true) {
		controller.receive(frame_a, frame_b);
		if (frame_a.type == frame_type::META) {
			num_pkts = frame_a.seq;
			size = *(int *) frame_a.data;
			strcpy(filename, (char *) (frame_a.data + 4));
			break;
		} else if (frame_b.type == frame_type::META) {
			num_pkts = frame_b.seq;
			size = *(int *) frame_b.data;
			strcpy(filename, (char *) (frame_b.data + 4));
			break;
		}
	}

	controller.showmsg("Receiving");
	time_t start = clock();

	frame_a.type = frame_type::METAACK;
	controller.send(frame_a, frame_a);

	uchar *data = new uchar[size];
	bool *received = new bool[num_pkts];
	memset(received, 0, num_pkts * sizeof(bool));
	controller.set_info(num_pkts, size, data, received);

	while (!finished(received)) {
		controller.stop = false;
		while (!controller.stop) {
			controller.receive(frame_a, frame_b);
		}
		ack(controller, received);
	}

	uLong bufLen = size * 10;
	uchar *buf = new uchar[bufLen];
	if (compressed) {
		uncompress(buf, &bufLen, data, size);
		uchar *t = buf; buf = data; data = t;
		size = bufLen;
	}

	char new_name[100];
	strcpy(new_name, args[1]);
	strcat(new_name, filename);
	FILE *file = fopen(new_name, "wb");
	fwrite(data, sizeof(uchar), size, file);
	fclose(file);

	double time = (double) (clock() - start) / CLK_TCK;
	controller.showtime(time);
	char buffer[20];
	sprintf(buffer, "%.2f KB/s", size / time / 1024);
	controller.showmsg(buffer);
	waitKey();

	delete[] data;
	delete[] received;
	delete[] buf;
	return 0;
}