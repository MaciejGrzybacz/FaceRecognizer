#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include "imageprocessor.hpp"
#include "videoprocessor.hpp"
#include "dbmgr.hpp" //only for test

int main() {
    DBMgr db;
    db.testDB();
    VideoProcessor vp;
    //vp.processVideo();
}
