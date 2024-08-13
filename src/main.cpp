#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <sys/time.h>

#define IS_RASPI false

int main() {
  // Open the video camera.
  std::string pipeline =
      "libcamerasrc"
      " ! video/x-raw, width=800, height=600" // camera needs to capture at a
                                              // higher resolution
      " ! videoconvert"
      " ! videoscale"
      " ! video/x-raw, width=400, height=300" // can downsample the image after
                                              // capturing
      " ! videoflip method=rotate-180" // remove this line if the image is
                                       // upside-down
      " ! appsink drop=true max_buffers=2";
  // cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);
  cv::VideoCapture cap(1);
  if (!cap.isOpened()) {
    printf("Could not open camera.\n");
    return 1;
  }

  // Create the OpenCV window
  cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);
  cv::namedWindow("Output", cv::WINDOW_AUTOSIZE);
  cv::Mat frame;

  // Measure the frame rate - initialise variables
  int frame_id = 0;
  timeval start, end;
  gettimeofday(&start, NULL);
  cv::Mat hsv_frame;

  // Create the thresholding window
  cv::namedWindow("Control", cv::WINDOW_AUTOSIZE);
  int iLowH = 0;
  int iHighH = 179;

  int iLowS = 0;
  int iHighS = 255;

  int iLowV = 0;
  int iHighV = 255;
  int morphSize = 3;

  // Create trackbars in "Control" window
  cv::createTrackbar("LowH", "Control", &iLowH, 179); // Hue (0 - 179)
  cv::createTrackbar("HighH", "Control", &iHighH, 179);

  cv::createTrackbar("LowS", "Control", &iLowS, 255); // Saturation (0 - 255)
  cv::createTrackbar("HighS", "Control", &iHighS, 255);

  cv::createTrackbar("LowV", "Control", &iLowV, 255); // Value (0 - 255)
  cv::createTrackbar("HighV", "Control", &iHighV, 255);

  cv::createTrackbar("MorphSize", "Control", &morphSize, 10);

  for (;;) {
    if (!cap.read(frame)) {
      printf("Could not read a frame.\n");
      break;
    }

    /*
     * Do the operations on the frame
     * - Color Thresholding
     * - Enhace the threholded image
     * - Locate centre of mass, and print to terminal
     * - Convert to HSV colour space
     * - Create a control window
     */

    // Convert it into hsv format
    cv::cvtColor(frame, hsv_frame, cv::COLOR_BGR2HSV);

    // Threshold the image
    cv::Mat thresh_img;
    cv::inRange(hsv_frame, cv::Scalar(iLowH, iLowS, iLowV),
                cv::Scalar(iHighH, iHighS, iHighV), thresh_img);

    // Perform the morphological operations
    cv::morphologyEx(thresh_img, thresh_img, cv::MORPH_CLOSE,
                     cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
    cv::morphologyEx(thresh_img, thresh_img, cv::MORPH_OPEN,
                     cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

    // Find the center of mass
    cv::Moments m = cv::moments(thresh_img, true);
    if (m.m00 > 0) {
      double x = m.m10 / m.m00;
      double y = m.m01 / m.m00;
      printf("Centre of mass was (%f, %f)\n", x, y);
    }

    // show frame
    cv::imshow("Camera", frame);
    cv::imshow("Output", thresh_img);
    cv::waitKey(1);

    // Measure the frame rate
    frame_id++;
    if (frame_id >= 30) {
      gettimeofday(&end, NULL);
      double diff =
          end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0;
      printf("30 frames in %f seconds = %f FPS\n", diff, 30 / diff);
      frame_id = 0;
      gettimeofday(&start, NULL);
    }
  }

  // Free the camera
  cap.release();
  return 0;
}
