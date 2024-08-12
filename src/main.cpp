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
  cv::Mat frame;

  // Measure the frame rate - initialise variables
  int frame_id = 0;
  timeval start, end;
  gettimeofday(&start, NULL);
  cv::Mat hsv_frame;

  // Create the thresholding window

  for (;;) {
    if (!cap.read(frame)) {
      printf("Could not read a frame.\n");
      break;
    }
    // Convert it into hsv format
    cv::cvtColor(frame, hsv_frame, cv::COLOR_BGR2HSV);

    /*
     * Do the operations on the frame
     * - Color Thresholding
     * - Enhace the threholded image
     * - Locate centre of mass, and print to terminal
     * - Convert to HSV colour space
     * - Create a control window
    */

    // show frame
    cv::imshow("Camera", frame);
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
