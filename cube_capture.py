#!/usr/bin/python
import sys
import cv2
import time
import os
import argparse

def open_camera(camera_id=0, frame_h=600, frame_w=800, fps=30):
  """
  Opens and returns a video device with a default resolution of 800x600 at 30 FPS.

  :param camera_id: The video device system ID (default 0).
  :param frame_h: The frame height (default 600).
  :param frame_w: The frame width (default 800).
  :param fps: The Frames Per Second (FPS) of the capture (default 30).
  """
  vid_dev = cv2.VideoCapture(camera_id)

  # set the device capture parameters. for more information see this link:
  # http://docs.opencv.org/modules/highgui/doc/reading_and_writing_images_and_video.html#videocapture-set
  vid_dev.set(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT, frame_h);
  vid_dev.set(cv2.cv.CV_CAP_PROP_FRAME_WIDTH, frame_w);
  vid_dev.set(cv2.cv.CV_CAP_PROP_FPS, fps);

  return vid_dev

def capture_frame(vid_dev):
  """
  Captures the current frame from the supplied video device. If an error occurs
  during capture, this function will return None indicating that a frame was not
  captured.

  :param vid_dev: The open video device (see 'open_camera').
  """
  ret, img = vid_dev.read()
  if not ret:
    print sys.stderr, "Error Capturing from the Video Device"
    return None
  else:
    return img

def cleanup(camera_id=0):
    """
    Closes all OpenCV windows and releases the video device.

    :param camera_id: The video device system ID (default 0).
    """
    cv2.destroyAllWindows()
    cv2.VideoCapture(camera_id).release()

def rgb_to_gray(img):
    """
    Converts an RGB image to Grayscale. Each Grayscale pixel will be represented
    by the intensity of the pixels in the original image.

    :param img: An RGB image.
    """
    print img.shape
    return cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

def apply_threshold(img, threshold=170):
    """
    Converts an image into a binary image using the specified threshold. All
    pixels with a value less than the threshold will become a binary 0, while
    all other pixels will become a binary 1.

    :param img: The image the threshold should be applied to.
    :param threshold: The threshold value used for conversion (default 170).
    """
    (thresh, img_bw) = cv2.threshold(img, threshold, 255, cv2.THRESH_BINARY)
    return (thresh, img_bw)

# main entry point for this script
if __name__ == "__main__":

  # CLI configurations
  parser = argparse.ArgumentParser(description="""Captures video frames from a
                                                  video device using OpenCV.""")
  parser.add_argument("--id", action="store", type=int, default=0,
                      help="The system ID for the video device (0 by default)")
  parser.add_argument("--threshold", action="store", type=int, default=170,
                      help="The binary threshold value used in RGB->Binary conversion.")

  args = parser.parse_args()

  # camera ID
  camera_id = args.id
  bw_threshold = args.threshold

  # open the camera device with the supplied ID
  vid_dev = open_camera(camera_id)

  while True:
      # get a frame from the video device
      img_orig = capture_frame(vid_dev)

      # if a frame was succesfully captured, show it
      if img_orig is not None:
          # do some image manipulation for comparison
          img_gray = rgb_to_gray(img_orig)
          (thresh, img_bw) = apply_threshold(img_orig, bw_threshold)
          cv2.imshow('video_rgb', img_orig)
          #cv2.imshow('video_gray', img_gray)
          cv2.imshow('video_bw', img_bw)

      # otherwise the video capture has failed.
      else:
          break

      # terminate the loop on a KeyboardInterrupt
      if (cv2.waitKey(2) >= 0):
          break

  # close the video device and any active windows
  cleanup(camera_id)
