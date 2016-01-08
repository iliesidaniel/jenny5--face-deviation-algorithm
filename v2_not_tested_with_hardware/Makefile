LIB = -lopencv_calib3d -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videoio -lopencv_video -lopencv_videostab
BIN = run.out
SRC = facedetect.cpp
INCLUDE = -I/usr/local/include/opencv2

all:
	g++ -o $(BIN) $(SRC) $(LIB) $(INCLUDE)
