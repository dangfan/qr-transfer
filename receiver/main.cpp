#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <ctime>
#include "coder.h"
#include "io.h"

using namespace std;
using namespace cv;

bool ready = false;
int size, num_pkts;

void onMouse(int event, int x, int y, int flags, void*) {
	if (event != EVENT_LBUTTONDOWN) return;
	ready = true;
}

void connect(IOController &controller) {
	frame frame_a, frame_b;
	memset(&frame_a, 0, sizeof(frame));
	frame_a.type = frame_type::INIT;
	controller.send(frame_a, frame_a);

	int counter = 0;
	time_t past[20], now;
	char text[20];
	memset(past, 0, sizeof(past));
	while (!ready) {
		controller.receive(frame_a, frame_b);
		if (frame_a.type == frame_type::INIT && frame_b.type == frame_type::INIT) {
			time(&now);
			sprintf(text, "%.2f fps, %d", 20 / difftime(now, past[counter]), counter);
			controller.showmsg(text);
			past[counter] = now;
			if (++counter == 20) {
				counter = 0;
			}
			num_pkts = frame_a.seq;
			size = *(int *)frame_a.data;
		}
		waitKey(10);
	}
}

void fill_data(frame &f, uchar *data, bool *received) {
	if (f.seq == num_pkts - 1) { // last frame
		memcpy(data + f.seq * MAX_DATA, f.data, size - f.seq * MAX_DATA);
	} else { // normal frame
		memcpy(data + f.seq * MAX_DATA, f.data, MAX_DATA);
	}
	received[f.seq] = true;
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
	frame frame_ack;
	frame_ack.type = frame_type::ACK;
	frame_ack.seq = counter;
	memcpy(frame_ack.data, lst, counter * sizeof(short));
	controller.send(frame_ack, frame_ack);
	delete[] lst;
}

int main() {
	IOController controller(640, 480);
	setMouseCallback("w", onMouse);
	connect(controller);

	controller.showmsg("Ready to receive");

	uchar *data = new uchar[size];
	bool *received = new bool[num_pkts];
	memset(received, 0, num_pkts * sizeof(bool));

	frame frame_a, frame_b;

	while (!finished(received)) {
		while (true) {
			controller.receive(frame_a, frame_b);
			if (frame_a.type == frame_type::DATA) {
				fill_data(frame_a, data, received);
			} else if (frame_a.type == frame_type::END) {
				break;
			}
			if (frame_b.type == frame_type::DATA) {
				fill_data(frame_b, data, received);
			} else if (frame_b.type == frame_type::END) {
				break;
			}
		}
		ack(controller, received);
	}

	FILE *file = fopen("test.ico", "wb");
	fwrite(data, sizeof(uchar), size, file);
	fclose(file);

	delete[] data;
	delete[] received;
	return 0;
}