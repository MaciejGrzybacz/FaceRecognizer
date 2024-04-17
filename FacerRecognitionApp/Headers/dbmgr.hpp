#pragma once
#include "sqlite3.h"
#include <string>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <filesystem>

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
        - addPhotosToDB(): adds photos from the photos directory to the database 
        - getNames(): returns a vector of pairs of integers and strings representing the labels and names from the database
        - getName(int label): returns the name corresponding to the label from the database 
        - getImages(): returns a vector of pairs of integers and cv::Mat representing the labels and images from the database
        - insertPhotoIntoDb(std::string photoPath, int label, std::string name): inserts a photo into the database 
        - testDB(): test function for the database operations 
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
    int addPhotosToDB();
    std::vector<std::pair<int, std::string>> getNames();
    std::string getName(int label);
    std::vector<std::pair<int, cv::Mat>> getImages();
    int insertPhotoIntoDb(std::string photoPath, int label, std::string name);
    void testDB();
};
