#pragma once
#include <opencv2/opencv.hpp>
#include "imageprocessor.hpp"

/*
	Class description:
	Class to process the video, read the frames and run the image processing methods
	Attributes:
		- video_path: path to the video file
		- cap: video capture object
		- image_processor: object of the ImageProcessor class, used for image processing

	Methods:
		- VideoProcessor(): constructor
		- ~VideoProcessor(): destructor
		- processVideo(): processes the video, reads the frames and runs the image processing methods
*/
class VideoProcessor {
private:
	std::string video_path;
	cv::VideoCapture cap;
	ImageProcessor image_processor;

public:
	VideoProcessor();
	~VideoProcessor();
	int processVideo();
};