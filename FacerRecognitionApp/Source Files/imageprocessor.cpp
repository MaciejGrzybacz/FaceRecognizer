#include "imageprocessor.hpp"

ImageProcessor::ImageProcessor()
{
	face_detector_path = R"(.\classifiers\haarcascade_frontalface_alt.xml)";
	face_recognizer_path = R"(.\classifiers\face_recognizer.yml)";
	scale_factor = 1;
	face_detector = cv::CascadeClassifier(face_detector_path);
	if (!face_detector.load(face_detector_path)) {
		std::cerr << "Failed to load face cascade from " << face_detector_path << std::endl;
		exit(1); 
	}
	face_recognizer = cv::face::LBPHFaceRecognizer::create();
	face_tracker = cv::TrackerKCF::create();
	current_faces = std::vector<cv::Rect>();
	recognized_faces = std::vector<std::pair<std::string, cv::Rect>>();
}

ImageProcessor::~ImageProcessor()
{
	;
}

cv::Mat ImageProcessor::processFrame(cv::Mat frame) {
	if (frame.empty()) {
		std::cerr << "Error: The frame is empty." << std::endl;
		return frame;
	}

	int new_width = static_cast<int>(frame.cols * scale_factor);
	int new_height = static_cast<int>(frame.rows * scale_factor);


	cv::Mat resized_frame;
	cv::resize(frame, resized_frame, cv::Size(new_width, new_height));

	detectFaces(resized_frame);

	drawFaces(frame);

	return frame;
}

std::vector<cv::Rect> ImageProcessor::getFaces()
{
	return current_faces;
}

std::vector<std::pair<std::string, cv::Rect>> ImageProcessor::getRecognizedFaces()
{
	return recognized_faces;
}

void ImageProcessor::trainRecognizer()
{
	// need to implement the training part
	//face_recognizer->save(face_recognizer_path);
}

void ImageProcessor::detectFaces(cv::Mat& frame) {
	cv::Mat gray;
	if (frame.empty()) {
		std::cerr << "Received empty frame." << std::endl;
		return;
	}
	cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
	cv::equalizeHist(gray, gray);

	std::vector<cv::Rect> faces;
	face_detector.detectMultiScale(gray, faces, 1.1, 3);
	if (faces.empty()) {
		std::cerr << "No faces detected." << std::endl;
		return;
	}
	current_faces = faces; 
}

void ImageProcessor::trackFaces(cv::Mat& frame) {
	if (face_tracker && !current_faces.empty()) {
		auto tracked_face = current_faces[0];
		if (!face_tracker->update(frame, tracked_face)) {
			std::cerr << "Tracking failed." << std::endl;
			current_faces.clear();
			face_tracker.release(); 
		}
		else {
			current_faces[0] = tracked_face;
		}
	}
	else {
		std::cerr << "Tracker not initialized or no faces to track." << std::endl;
	}
}


void ImageProcessor::recognizeFaces(cv::Mat& frame)
{
}

void ImageProcessor::drawFaces(cv::Mat& frame)
{
	if (current_faces.empty()) {
		return;
	}

	for (const auto& face : current_faces) {
		cv::Rect scaled_face(face.x / scale_factor,
			face.y / scale_factor,
			face.width / scale_factor,
			face.height / scale_factor);

		// Rysowanie prostokąta na klatce obrazu z nowymi współrzędnymi
		cv::rectangle(frame, scaled_face, cv::Scalar(255, 0, 0), 2);
	}
	current_faces.clear();
}

