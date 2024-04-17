#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <opencv2/tracking.hpp>
#include "dbmgr.hpp"
/*
	Class description:
	Class is responsible for processing the image frame.

	Attributes:
		- face_detector_path: path to the face detector xml file
		- face_recognizer_path: path to the face recognizer yml file
		- scale_factor: factor used for resizing the frame
		- current_faces: vector of rectangles representing the faces in the current frame
		- recognized_faces: vector of pairs of strings and rectangles representing the recognized faces in the current frame
		- face_detector: object of the CascadeClassifier class, used for face detection
		- face_tracker: pointer to the face tracker object
		- face_recognizer: pointer to the face recognizer object
		- db: object of the DBMgr class, used for database operations

	Methods:
		- ImageProcessor(): constructor
		- ~ImageProcessor(): destructor
		- processFrame(cv::Mat frame): processes the frame, runs all private methods and returns the processed frame
		- getFaces(): returns the vector of rectangles representing the faces in the current frame, mainly used for testing
		- getRecognizedFaces(): returns the vector of pairs of strings and rectangles representing the recognized faces in the current frame, mainly used for testing
		- detectFaces(cv::Mat& frame): detects faces in the frame and stores them in the current_faces vector
		- trackFaces(cv::Mat& frame): tracks faces in the frame using the face_tracker object 
		- recognizeFaces(cv::Mat& frame): recognizes faces in the frame using the face_recognizer object 
		- drawFaces(cv::Mat& frame): draws rectangles and names around the faces in the frame
*/
class ImageProcessor {
private:

	std::string	face_detector_path;
	std::string face_recognizer_path;
	double scale_factor;

	std::vector<cv::Rect> current_faces;
	std::vector<std::pair<std::string, cv::Rect>> recognized_faces;
	cv::CascadeClassifier face_detector;
	cv::Ptr<cv::Tracker> face_tracker;
	cv::Ptr<cv::face::FaceRecognizer> face_recognizer;
	DBMgr db;

public:
	ImageProcessor();
	~ImageProcessor();
	cv::Mat processFrame(cv::Mat frame);
	std::vector<cv::Rect> getFaces();
	std::vector<std::pair<std::string,cv::Rect>> getRecognizedFaces();

private:
	void trainRecognizer();
	void detectFaces(cv::Mat& frame);
	void trackFaces(cv::Mat& frame);
	void recognizeFaces(cv::Mat& frame);
	void drawFaces(cv::Mat& frame);
};