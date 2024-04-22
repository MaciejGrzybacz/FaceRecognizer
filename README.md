# FaceRecognizer

The **Face Recognition System with SQLite Database** is a C++ application designed to detect and recognize faces in images using OpenCV and store relevant information in a SQLite database. The project consists of two main components: the image processing module and the database management module.

## Image Processing Module:
The image processing module, implemented in the `ImageProcessor` class, is responsible for detecting and recognizing faces in images. It utilizes OpenCV's computer vision library to perform tasks such as face detection, face recognition, and image preprocessing. The key functionalities of this module include:

- **Face detection:** Utilizes a pre-trained Haar cascade classifier to detect faces in images.
- **Face recognition:** Employs the LBPH (Local Binary Patterns Histograms) face recognizer to recognize faces based on previously trained models.
- **Image preprocessing:** Converts images to grayscale, equalizes histogram, and preprocesses images to improve face recognition accuracy.
- **Integration with the OpenCV library:** Utilizes OpenCV functions for image manipulation, feature extraction, and face tracking.

## Database Management Module:
The database management module, implemented in the `DBMgr` class, facilitates the creation, management, and querying of a SQLite database to store face-related information. The key functionalities of this module include:

- **Database initialization:** Creates a SQLite database file and initializes the required tables for storing face data.
- **Table creation and deletion:** Dynamically creates and drops tables in the database to manage face data efficiently.
- **Insertion and retrieval of data:** Inserts face images along with associated labels and names into the database and retrieves stored face data for recognition purposes.
- **Integration with SQLite:** Utilizes the SQLite C/C++ interface to execute SQL queries, handle database transactions, and manage database connections.

## Project Workflow:
1. **Image Processing:** Input images are processed using the image processing module to detect and recognize faces.
2. **Database Interaction:** Detected faces and associated metadata are stored in the SQLite database using the database management module.
3. **Face Recognition:** Upon detecting faces in images, the system matches them against stored face data in the database to recognize individuals.
4. **Feedback and Output:** Recognized faces are annotated with labels/names and visualized in the output images, providing feedback to the user.

## Project Components:
- **ImageProcessor:** Handles image processing tasks including face detection, recognition, and preprocessing.
- **DBMgr:** Manages the SQLite database, including table creation, data insertion, and retrieval.

## Future Enhancements:
- **User interface:** Develop a user-friendly interface for interacting with the system, allowing users to manage face data and view recognition results.
- **Performance optimization:** Optimize image processing algorithms and database operations for faster and more efficient processing. 
