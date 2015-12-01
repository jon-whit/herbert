#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstring>

#define DEBUG 0

using namespace cv;
using namespace std;

/**
 * Stores the BGR average for each of the facelets into memory pointed to by
 * the argument 'averages'.
 *
 * The facelets are numbered in the following way:
 *
 * 1. If the point file is defining the mask for the up camera then the faces should
 *    ordered: B, L, U. If the mask is for the bottom camera then the faces should be
 *    ordered: F, D, R
 * 2. The ordering of each facelet should correspond to the numbering in the Kociemba's
 *    algorithm. For example: B1 will be at index 0 and B2 at index 2.
 *
 * This function:
 * 1. Applies a polygon mask to each of facelets.
 * 2. Calculates the mean BGR value of each facelet using the mask and stores this
 *    in an array. The array contains the mean BGR values of the facelet corresponding
 *    to that index.
 *
 * For example, if 'arr' is the returned array, then 'arr[0]' will contain the BGR
 * values for the facelet at index 0, 'arr[1]' for the facelet at index 1, etc..
 */
void store_BGRavg(Mat *image, string filename, Scalar *averages) {
    /*
     * Read in the text file containing the polygon points.
     */
     ifstream infile(filename.c_str());

    /*
     * Define the polygons that will be used as a mask.
     */
    int numPolygons = 27;
    Point pts[27][4];
    for ( int i = 0; i < numPolygons; i++ ) {

      // Read point pairs out of the text file
      int x1, y1, x2, y2, x3, y3, x4, y4;
      infile >> x1 >> y1;
      infile >> x2 >> y2;
      infile >> x3 >> y3;
      infile >> x4 >> y4;

      if ( DEBUG ) {
        printf("P%d: [(%d, %d), (%d, %d), (%d, %d), (%d, %d)]\n", i,
                x1, y1, x2, y2, x3, y3, x4, y4);
      }

      pts[i][0] = Point(x1, y1);
      pts[i][1] = Point(x2, y2);
      pts[i][2] = Point(x3, y3);
      pts[i][3] = Point(x4, y4);
    }

    /*
     * Generate a mask for each of the polygons and calculate the BGR mean using
     * that mask.
     */
    for ( int i = 0; i < numPolygons; i++ ) {

      const Point* points[1] = {pts[i]};
      int npoints = 4;

      // Create the mask
      Mat1b mask(image->rows, image->cols, uchar(0));
      fillPoly(mask, points, &npoints, 1, Scalar(255));

      // Compute the mean with the computed mask
      Scalar average = mean(*image, mask);
      averages[i] = average;
    }

    if ( DEBUG ) {
      for (int i = 0; i < numPolygons; i++)
        cout << "P" << i << "_AVG: " << averages[i] << endl;
    }
}

void categorize_facelets(Scalar *averages, char* facelets)
{
  // Should be made const later
  const int centerFIndexes[] = {4, 13, 22, 31, 40, 49};
  const char catToColorChar[] = {'W', 'B', 'R', 'G', 'Y', 'O'}; // Arbitrary right now

  // Extract all the colors from the center pieces
  Scalar primColors[6];
  for(int i = 0; i < 6; i++)
  {
    primColors[i] = averages[centerFIndexes[i]];
  }

  // Catagorize the remaining based on distance
  for(int i = 0; i < 54; i++)
  {
    // Calculate distance between averages[i] and
    // each of the primColors, and assign facelets[i]
    // to the smallest difference

    int min = 3 * 255 * 255 + 1;
    int minCat = 0;
    for(int cat = 0; cat < 6; cat++)
    {
      Scalar x = primColors[cat];
      Scalar y = averages[i];
      int b = x[0] - y[0];
      int g = x[1] - y[1];
      int r = x[2] - y[2];
      int distSquared = b*b + g*g + r*r;
      if(distSquared < min)
      {
	        min = distSquared;
	        minCat = cat; // assign the minimum to the appropriate category (0-5)
      }
    }

    facelets[i] = catToColorChar[minCat];
  }
}

void output_orientation(char* facelets)
{
  const char faces[] = {'B', 'L', 'U', 'F', 'D', 'R'};

  for(int i = 0; i < 54; i++)
  {
    if(i % 9 == 0)
    {
      // Don't print a space on the first section
      if(i != 0)
      {
	        cout << " ";
      }

      cout << faces[i / 9] << ":";
    }

    cout << facelets[i];
  }

  cout << endl;
}

int main(int argc, char** argv) {

    Mat upper_mat, down_mat;

    if ( !strcmp(argv[3], "-d") ) {
      upper_mat = imread(argv[1], CV_LOAD_IMAGE_COLOR);
      down_mat = imread(argv[2], CV_LOAD_IMAGE_COLOR);

      if (!upper_mat.data || !down_mat.data) {
          cout << "Could not open or find the image '" << argv[1] << "' '" << argv[2] << "'" << endl;
          return -1;
      }
    } else {

       VideoCapture camU(*argv[1] - 'a');
       VideoCapture camD(*argv[2] - 'b');

       if ( !camU.isOpened() || !camD.isOpened() ) {
           printf("Could not open the camera devices at '%s' and '%s'", argv[1], argv[2]);
           return -1;
       }

      camU >> upper_mat;
      camD >> down_mat;

    }

    // Store the BGR average for each of the facelets.
    Scalar averages[54];
    store_BGRavg(&upper_mat, "front_points2.txt", &averages[0]);
    store_BGRavg(&down_mat, "back_points2.txt", &averages[27]);

    // Categorize all 54 face colors
    char facelets[54];
    categorize_facelets(averages, facelets);
    output_orientation(facelets);

    /* Create a nice output display if debugging is enabled. This will create
     * output formatted like follows:
     *
     * F0  F1  F2
     * F3  F4  F5
     * F6  F7  F8
     *
     * F9  F10 F11
     * F12 F13 F14
     * F15 F16 F17
     *
     * F18 F19 F20
     * F21 F22 F23
     * F24 F25 F26
     * .
     * .
     * .
     * So on so forth..
     */
    if ( DEBUG ) {

      int cnt = 1;
      for (int i = 0; i < 54; i++) {
        printf("%d ", facelets[i]);
        cnt++;

        if ( cnt % 3 == 0 )
            printf("\n");

        if ( cnt == 9 || cnt == 18 || cnt == 27)
            printf("\n");
      }
    }

    return 0;
}
