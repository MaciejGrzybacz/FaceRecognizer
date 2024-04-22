#pragma once
#include "sqlite3.h"
#include <string>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <filesystem>
#include "imageprocessor.h"
/*
    Class description:
    This class is responsible for managing the database operations.

    Attributes:
        - db: pointer to the sqlite3 object
        - path: path to the database file
        - photos_path: path to the photos directory

    Methods:
        - DBMgr(): constructor
        - ~DBMgr(): destructor
        - addTable(): creates the photos table in the database
        - dropTable(): drops the photos table from the database
        - readPhotosFromDir(): reads photos from the photos directory
        - getNames(): returns the names of the people in the database
        - getName(int label): returns the name of the person with the given label
        - getImages(): returns the labels and images from the database
        - insertPhotosIntoDB(std::pair<std::vector<cv::Mat>, std::vector<std::pair<int, std::string>>>): inserts photos into the database
*/
class DBMgr {
    sqlite3* db;
    std::string path;
    std::string photos_path;

public:
    DBMgr();
    ~DBMgr();
    void addTable();
    void dropTable();
    std::pair<std::vector<cv::Mat>, std::vector<std::pair<int, std::string>>> readPhotosFromDir();
    std::vector<std::pair<int, std::string>> getNames();
    std::string getName(int label);
    std::pair<std::vector<int>, std::vector<cv::Mat>> getImages();
    int insertPhotosIntoDB(std::pair<std::vector<cv::Mat>, std::vector<std::pair<int, std::string>>>);
    void initializeDB(ImageProcessor image_processor);
};
