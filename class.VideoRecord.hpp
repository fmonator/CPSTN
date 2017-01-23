#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "entities.hpp"
#include <queue>

using namespace cv;
using namespace std;

class VideoRecord {

private:
	// http://docs.opencv.org/modules/highgui/doc/reading_and_writing_images_and_video.html#videocapture
	VideoCapture* cap;
	string filename;

	void load() {
		if(filename.empty()) {
			cap = new VideoCapture(0);
		} else {
			cap = new VideoCapture(filename);
		}
		if(!cap->isOpened()) {
			throw runtime_error("[ModulKamera] I cannot open device / file.");
		}
	}

	void unload() {
		if(cap != NULL) {
			if(cap->isOpened()) {
				cap->release();
			}
			delete cap;
			cap = NULL;
		}
	}

public:

	class EndOfStream : public std::exception {
	public:
		EndOfStream() : std::exception("EndOfStream") {}
	};

	VideoRecord(const string filename) {
		this->filename = filename;
		load();
	}

	~VideoRecord() {
		unload();
	}

	// Ziskaj aktualnu snimku z kamery (You will get the current frame from the camera)
	Frame* readNext() {
		Frame* frame = new Frame();
		if(!cap->read(frame->data)) {
			throw EndOfStream();
		}
		frame->pos_msec = cap->get(CV_CAP_PROP_POS_MSEC) / 40;
		return frame;
	}

	void doReset() {
		unload();
		load();
	}
};