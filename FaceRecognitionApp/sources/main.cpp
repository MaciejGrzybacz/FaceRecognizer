#include "dbmgr.h"
#include "imageprocessor.h"

int prepareImageProcessing(DBMgr& db, ImageProcessor& image_processor)
{
	//db.initializeDB(image_processor); //you can use it to initialize database(drup table, create new, add all photos)
	auto labels_and_images = db.getImages();
	auto images = labels_and_images.second;
	auto labels = labels_and_images.first;
	image_processor.setNames(db.getNames());
	image_processor.trainRecognizer(images, labels);
	return 0;
}
int main()
{
	DBMgr db;
	ImageProcessor image_processor;
	prepareImageProcessing(db,image_processor);
	std::string video_path = R"(.\demo.mp4)";
	cv::VideoCapture cap = cv::VideoCapture(0); //If you want to use video from path, you need to change 0 to path


	if (!cap.isOpened()) {
		std::cerr << "Error opening video file" << std::endl;
		exit(1);
	}
	else {
		std::cout << "Opened video file successfully" << std::endl;
	}

	cv::Mat frame;
	while (true) {
		cap >> frame;
		if (frame.empty()) {
			std::cerr << "Error reading frame" << std::endl;
			break;
		}
		image_processor.processFrame(frame);
		cv::imshow("Face Recogniser", frame);
		if (cv::waitKey(10) >= 0) break;
	}

	return 0;
}