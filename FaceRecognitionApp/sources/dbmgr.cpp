#include "dbmgr.h"
#include "imageprocessor.h"

DBMgr::DBMgr() {
    this->path = R"(.\db\database.db)";
    this->photos_path = R"(.\photos)";
    int result = sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (result != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }
    else {
        std::cout << "Opened database successfully" << std::endl;
    }
}

DBMgr::~DBMgr() {
    sqlite3_close(db);
}

void DBMgr::addTable() {
    std::string sql = "CREATE TABLE IF NOT EXISTS photos ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "label INTEGER, "
        "name TEXT, "
        "photo BLOB)";
    char* errMsg = NULL;
    int rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    else {
        std::cout << "Table created successfully" << std::endl;
    }
}

void DBMgr::dropTable() {
    std::string sql = "DROP TABLE IF EXISTS photos";
    char* errMsg = NULL;
    int rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    else {
        std::cout << "Table dropped successfully" << std::endl;
    }
}

std::pair<std::vector<cv::Mat>, std::vector<std::pair<int, std::string>>> DBMgr::readPhotosFromDir() {
    if (!std::filesystem::exists(DBMgr::photos_path) || !std::filesystem::is_directory(DBMgr::photos_path)) {
        std::cerr << "Invalid path: doesn't exist or not a directory: " << DBMgr::photos_path << std::endl;
    }
    std::vector<std::pair<int, std::string>> names;
    std::vector<cv::Mat> images;
    int max_label = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(DBMgr::photos_path)) {
        const auto& file_path = entry.path();
        if (file_path.extension() == ".jpg" || file_path.extension() == ".png") {
            std::string name = file_path.parent_path().filename().string();
            cv::Mat image = cv::imread(file_path.string());
            images.push_back(image);
            int label = -1;
            for (const auto& n : names) {
                if (n.second == name) {
                    label = n.first;
                    break;
                }
            }
            if (label == -1) {
                label = max_label++;
                names.push_back(std::make_pair(label, name));
            }
            else {
                names.push_back(std::make_pair(label, name));
            }
        }
    }
    return std::make_pair(images, names);
}

int DBMgr::insertPhotosIntoDB(std::pair<std::vector<cv::Mat>, std::vector<std::pair<int, std::string>>> rows) {
    auto images = rows.first;
    auto names = rows.second;
    for (int i = 0; i < images.size(); i++) {
        std::vector<uchar> buffer;
        cv::imencode(".png", images[i], buffer);

        int dataSize = buffer.size();
        std::string sql = "INSERT INTO photos (label, name, photo) VALUES (?, ?, ?)";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
            return -1;
        }

        sqlite3_bind_int(stmt, 1, names[i].first);
        sqlite3_bind_text(stmt, 2, names[i].second.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_blob(stmt, 3, buffer.data(), dataSize, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Error during inserting data: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return -2;
        }

        sqlite3_finalize(stmt);
    }

    return 0;
}

std::vector<std::pair<int, std::string>> DBMgr::getNames() {
    std::string sql = "SELECT DISTINCT label, name FROM photos";
    std::vector<std::pair<int, std::string>> names;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Error during getting names: " << sqlite3_errmsg(db) << std::endl;
        return names;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int label = sqlite3_column_int(stmt, 0);
        const unsigned char* name_data = sqlite3_column_text(stmt, 1);
        std::string name(reinterpret_cast<const char*>(name_data));
        names.push_back(std::make_pair(label, name));
    }

    sqlite3_finalize(stmt);
    return names;
}

// in the end of the day, unnesessery, maybe in some update I will use it to something different than planned role
std::string DBMgr::getName(int label) {
    std::string sql = "SELECT DISTINCT name FROM photos WHERE label = ?";
    std::string name;
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error while preparing SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return name;
    }

    if (sqlite3_bind_int(stmt, 1, label) != SQLITE_OK) {
        std::cerr << "Error while binding value: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return name;
    }

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char* result = sqlite3_column_text(stmt, 0);
        if (result) {
            name = reinterpret_cast<const char*>(result);
        }
    }
    else if (rc == SQLITE_DONE) {
        std::cerr << "No results found." << std::endl;
    }
    else {
        std::cerr << "Error while retrieving data: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return name;
}

std::pair<std::vector<int>, std::vector<cv::Mat>> DBMgr::getImages() {
    std::string sql = "SELECT label, photo FROM photos";
    std::vector<int> labels;
    std::vector<cv::Mat> images;
    sqlite3_stmt* stmt;
    std::cout << "Getting images..." << std::endl;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Error while executing: " << sqlite3_errmsg(db) << std::endl;
        return std::make_pair(labels, images);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        int bytes = sqlite3_column_bytes(stmt, 1);
        std::cout << "Read label: " << id << ", size of photo blob: " << bytes << " bytes." << std::endl;

        if (bytes == 0) {
            std::cerr << "Warning: Empty photo blob for label: " << id << std::endl;
            continue;
        }

        const uchar* blob_data = reinterpret_cast<const uchar*>(sqlite3_column_blob(stmt, 1));
        std::vector<uchar> data(blob_data, blob_data + bytes);
        cv::Mat img = cv::imdecode(cv::Mat(data), cv::IMREAD_UNCHANGED);

        if (img.empty()) {
            std::cerr << "Failed to decode image for label: " << id << std::endl;
            continue;
        }

        images.push_back(img);
        labels.push_back(id);
    }

    std::cout << "Total images loaded: " << images.size() << std::endl;


    sqlite3_finalize(stmt);
    return std::make_pair(labels, images);
}

void DBMgr::initializeDB(ImageProcessor image_processor) {
    dropTable();
    addTable();
    auto read_photos = readPhotosFromDir();
    image_processor.preprocessImages(read_photos.first);
    insertPhotosIntoDB(read_photos);
    std::cout << "Database initialized successfully" << std::endl;
}
