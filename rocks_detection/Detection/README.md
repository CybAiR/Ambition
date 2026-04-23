# Rock Detection Pipeline

This repository contains a comprehensive pipeline for rock detection, ranging from classical computer vision algorithms to advanced deep learning models (YOLO, R-CNN).

## Project Structure

### YOLO Real-Time Detection
* **YOLOModel.py** - Handles the training phase of a custom YOLO model. It configures training parameters, automatically saves the best weights, and includes a validation routine for static test images.
* **YOLOTest.py** - Executes live inference using the trained YOLO model. It utilizes a custom multi-threaded webcam handler to eliminate frame lag and ensure smooth detection.
* **YOLOEdges.py** - Executes real-time YOLO detection combined with OAK-D stereo vision to calculate the physical dimensions (width, height, depth) of rocks in centimeters.
* **TymonRocks.py** - A streamlined YOLOv8 script designed for quick field testing and live detection.

### Mathematical Analysis
* **Mathematic.py** - Implements classical computer vision techniques based on the ROCKSTER algorithm. It features sky-ground segmentation via variance analysis, edge detection, gap filling, and morphological filtering to identify rocks without neural networks.

### Deep Learning (R-CNN)
* **ModelTest.py** - Trains an R-CNN model using VGG16 as the feature extractor and Selective Search for region proposals.
* **ModelRunTest.py** - Deploys the trained R-CNN model on a live camera feed. It also incorporates multi-threading to maintain a steady framerate during heavy Selective Search processing.

### Utilities
* **checkCUDA.py** - A diagnostic script to verify hardware acceleration for both TensorFlow and PyTorch environments.
* **CameraTest.py** - A simple utility to check standard webcam or OAK-D Lite connectivity using OpenCV and DepthAI.

# Current results

### Mathematical edge detection results
![Mathematical edge detection results](results/resultsOpp.jpg)

### Real-time object detection using YOLO26
![Real-time object detection using YOLO26](results/resultsYOLO1.jpg)

### Real-time object detection using YOLO26 with dimensioning
![Real-time object detection using YOLO26](results/resultsYOLO2.jpg)