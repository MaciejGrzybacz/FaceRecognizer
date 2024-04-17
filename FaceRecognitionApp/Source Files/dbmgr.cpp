#include "dbmgr.hpp"

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


int DBMgr::addPhotosToDB() {
    if (!std::filesystem::exists(DBMgr::photos_path) || !std::filesystem::is_directory(DBMgr::photos_path)) {
        std::cerr << "Invalid path: doesn;t exist or not a directory: " << DBMgr::photos_path << std::endl;
        return 1;
    }
    std::vector<std::pair<int, std::string>> names;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(DBMgr::photos_path)) {
        const auto& file_path = entry.path();
        if (file_path.extension() == ".jpg" || file_path.extension() == ".png") {
            std::string name = file_path.parent_path().filename().string();
            cv::Mat image = cv::imread(file_path.string());
            int label = -1;
            for (const auto& n : names) {
                if (n.second == name) {
                    label = n.first;
                    break;
                }
            }
            if (label == -1) {
                label = names.size();
                names.push_back(std::make_pair(label, name));
            }
            if (insertPhotoIntoDb(file_path.string(), label, name) != 0) {
                std::cerr << "Error during inserting photo: " << file_path.string() << std::endl;
                return 2;
            }
        }
    }
    return 0;
}

int DBMgr::insertPhotoIntoDb(const std::string photoPath, int label, const std::string name) {
    cv::Mat image = cv::imread(photoPath);
    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(400, 400));
    std::vector<uchar> buffer;
    cv::imencode(".png", resizedImage, buffer);

    int dataSize = buffer.size();
    std::string sql = "INSERT INTO photos (label, name, photo) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Error: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    sqlite3_bind_int(stmt, 1, label);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 3, buffer.data(), dataSize, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Error during inserting data: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -2;
    }

    sqlite3_finalize(stmt);
    return 0;
}

void DBMgr::testDB()
{
    DBMgr db;
    //db.addTable();
    //db.addPhotosToDB();

    auto images = db.getImages();
    size_t current_image = 0;

    for (auto name : db.getNames()) {
        std::cout << name.first << " " << name.second << std::endl;
    }
    for (auto image : images) {
        std::string name = db.getName(image.first);
        cv::imshow(name, image.second);
		cv::waitKey(0);
    }
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

std::vector<std::pair<int, cv::Mat>> DBMgr::getImages() {
    std::string sql = "SELECT label, photo FROM photos";
    std::vector<std::pair<int, cv::Mat>> images;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Error while executing: " << sqlite3_errmsg(db) << std::endl;
        return images;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        int bytes = sqlite3_column_bytes(stmt, 1);
        const uchar* blob_data = reinterpret_cast<const uchar*>(sqlite3_column_blob(stmt, 1));

        std::vector<uchar> data(blob_data, blob_data + bytes);

        cv::Mat img = cv::imdecode(cv::Mat(data), cv::IMREAD_UNCHANGED);
        images.push_back(std::make_pair(id, img));
    }

    sqlite3_finalize(stmt);
    return images;
}
