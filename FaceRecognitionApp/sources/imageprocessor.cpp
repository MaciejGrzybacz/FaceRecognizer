#include "imageprocessor.h"

ImageProcessor::ImageProcessor()
{
	face_detector_path = R"(.\classifiers\haarcascade_frontalface_alt.xml)";
	face_recognizer_path = R"(.\classifiers\face_recognizer.yml)";
	scale_factor = 0.5;
	face_detector = cv::CascadeClassifier(face_detector_path);
	if (!face_detector.load(face_detector_path)) {
		std::cerr << "Failed to load face cascade from " << face_detector_path << std::endl;
		exit(1);
	}
	face_recognizer = cv::face::LBPHFaceRecognizer::create();
	face_tracker = cv::TrackerKCF::create();
	current_faces = std::vector<cv::Rect>();
	recognized_faces = std::vector<std::pair<std::string, cv::Rect>>();
	known_names = std::vector<std::pair<int, std::string>>();
}


void ImageProcessor::setNames(std::vector<std::pair<int, std::string>> names)
{
	known_names = names;
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
	recognizeFaces(resized_frame);
	drawFaces(frame);

	return frame;
}

int ImageProcessor::preprocessImages(std::vector<cv::Mat>& images)
{
	for (int i = 0; i < images.size(); i++) {
		cv::Mat gray;
		cv::cvtColor(images[i], gray, cv::COLOR_BGR2GRAY);
		cv::equalizeHist(gray, gray);
		std::vector<cv::Rect> faces;
		face_detector.detectMultiScale(gray, faces, 1.1, 3);
		if (faces.empty()) {
			std::cerr << "No faces detected." << std::endl;
			return 1;
		}
		cv::Mat cropped = images[i](faces[0]);
		images[i] = cropped;
	}
	return 0;
}

void ImageProcessor::trainRecognizer(std::vector<cv::Mat> images, std::vector<int> labels) {
	std::cout << "Training recognizer..." << std::endl;
	std::ifstream recognizer_file(face_recognizer_path);
	if (recognizer_file.good()) {
		face_recognizer->read(face_recognizer_path);
		recognizer_file.close();
	}
	else {
		if (images.empty() || labels.empty() || images.size() != labels.size()) {
			std::cerr << "Invalid training data." << std::endl;
			return;
		}

		std::vector<cv::Mat> grayscale_images;
		for (const auto& image : images) {
			cv::Mat gray_image;
			cv::cvtColor(image, gray_image, cv::COLOR_BGR2GRAY);
			grayscale_images.push_back(gray_image);
		}

		face_recognizer->train(grayscale_images, labels);
		face_recognizer->save(face_recognizer_path);
		//face_recognizer->read(face_recognizer_path);
		trainRecognizer(images, labels);
	}
	std::cout << "Recognizer trained." << std::endl;
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
	face_detector.detectMultiScale(gray, faces, 1.5, 4);
	if (faces.empty()) {
		return;
	}
	current_faces = faces;
}

// I wanted to use tracking on video, but it's sooo unefective to track faces with it
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


void ImageProcessor::recognizeFaces(cv::Mat& frame) {
	cv::Mat gray;
	if (frame.empty()) {
		std::cerr << "Received empty frame." << std::endl;
		return;
	}
	cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
	cv::equalizeHist(gray, gray);

	for (auto& face : current_faces) {
		cv::Mat faceROI = gray(face);

		int label;
		double confidence;
		face_recognizer->predict(faceROI, label, confidence);

		if (confidence < 150.0) {
			int i = 0;
			while (known_names[i].first != label) {
				i++;
			}
			recognized_faces.push_back({ known_names[i].second, face});
		}
		else {
			recognized_faces.push_back({ "Unknown", face });
		}
	}
}

void ImageProcessor::drawFaces(cv::Mat& frame) {
	if (recognized_faces.empty()) {
		return;
	}

	for (const auto& rec_face : recognized_faces) {
		cv::Rect scaled_face = cv::Rect(rec_face.second.x / scale_factor, rec_face.second.y / scale_factor,
			rec_face.second.width / scale_factor, rec_face.second.height / scale_factor);

		cv::rectangle(frame, scaled_face, cv::Scalar(0, 255, 0), 2);
		cv::putText(frame, rec_face.first, cv::Point(scaled_face.x, scaled_face.y - 10),
			cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 255, 0), 2);
	}
	recognized_faces.clear();
}


