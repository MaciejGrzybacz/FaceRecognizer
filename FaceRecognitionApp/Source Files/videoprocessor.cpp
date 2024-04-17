#include "videoprocessor.hpp"

VideoProcessor::VideoProcessor()
{
	std::string video_path = R"(.\demo.mp4)";
	cap = cv::VideoCapture(0);
	if (!cap.isOpened()) {
		std::cerr << "Error opening video file" << std::endl;
	}
	else {
		std::cout << "Opened video file successfully" << std::endl;
	}
	image_processor = ImageProcessor();
}

VideoProcessor::~VideoProcessor()
{
	cap.release();
	cv::destroyAllWindows();
	image_processor.~ImageProcessor();
}

int VideoProcessor::processVideo()
{
	cv::Mat frame;
	while (true) {
		cap >> frame;
		if (frame.empty()) {
			std::cerr << "Error reading frame" << std::endl;
			break;
		}
		image_processor.processFrame(frame);
		cv::imshow("Video", frame);
		if (cv::waitKey(10) >= 0) break;
	}
	return 0;
}
