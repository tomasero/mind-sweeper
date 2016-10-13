// The "Square Detector" program.
// It loads several images sequentially and tries to find squares in
// each image

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <math.h>
#include <string.h>
#include <fstream>
#include <stdio.h>

using namespace cv;
using namespace std;

// Image matrices for the pattern matching templates
Mat four, three, two, one, clicked, unclicked;

// Image matrices for helper images while template matching
Mat r1, r2, r3, r4, rClick, rUnclick;

// Image matrix for the cropped grid from the image
Mat cropped;

int ret_ind;
int matchMethod = 0;

/**
 * @function MatchingMethod
 * @brief Trackbar callback
 */
void MatchingMethod(int, void*) {
	Mat img_display;
	cropped.copyTo( img_display );

	/// Create the resulting matrices
	int result_cols_four =  -cropped.cols + four.cols + 1;
	int result_rows_four = -cropped.rows + four.rows + 1;
	int result_cols_three =  -cropped.cols + three.cols + 1;
	int result_rows_three = -cropped.rows + three.rows + 1;
	int result_cols_two =  -cropped.cols + two.cols + 1;
	int result_rows_two = -cropped.rows + two.rows + 1;
	int result_cols_one =  -cropped.cols + one.cols + 1;
	int result_rows_one = -cropped.rows + one.rows + 1;
	int result_cols_clicked =  -cropped.cols + clicked.cols + 1;
	int result_rows_clicked = -cropped.rows + clicked.rows + 1;
	int result_cols_unclicked =  -cropped.cols + unclicked.cols + 1;
	int result_rows_unclicked = -cropped.rows + unclicked.rows + 1;

 	r4.create( result_rows_four, result_cols_four, CV_32FC1 );
 	r3.create( result_rows_three, result_cols_three, CV_32FC1 );
 	r2.create( result_rows_two, result_cols_two, CV_32FC1 );
 	r1.create( result_rows_one, result_cols_one, CV_32FC1 );
 	rClick.create( result_rows_clicked, result_cols_clicked, CV_32FC1 );
 	rUnclick.create( result_rows_unclicked, result_cols_unclicked, CV_32FC1 );

	/// Do the Matching and Normalize
  	matchTemplate( cropped, four, r4, matchMethod);
  	matchTemplate( cropped, three, r3, matchMethod); 
	matchTemplate( cropped, two, r2, matchMethod); 
	matchTemplate( cropped, one, r1, matchMethod); 
	matchTemplate( cropped, clicked, rClick, matchMethod); 
	matchTemplate( cropped, unclicked, rUnclick, matchMethod); 

  	/// Localizing the best match with minMaxLoc
  	double minVal4; double maxVal4; Point minLoc4; Point maxLoc4;
  	double minVal3; double maxVal3; Point minLoc3; Point maxLoc3;
	double minVal2; double maxVal2; Point minLoc2; Point maxLoc2;
	double minVal1; double maxVal1; Point minLoc1; Point maxLoc1;
	double minValC; double maxValC; Point minLocC; Point maxLocC;
	double minValU; double maxValU; Point minLocU; Point maxLocU;

	// minVal* and maxVal* doubles are populate with similarity values
  	minMaxLoc( r4, &minVal4, &maxVal4, &minLoc4, &maxLoc4, Mat() );
    minMaxLoc( r3, &minVal3, &maxVal3, &minLoc3, &maxLoc3, Mat() ); 
    minMaxLoc( r2, &minVal2, &maxVal2, &minLoc2, &maxLoc2, Mat() ); 
    minMaxLoc( r1, &minVal1, &maxVal1, &minLoc1, &maxLoc1, Mat() ); 
    minMaxLoc( rClick, &minValC, &maxValC, &minLocC, &maxLocC, Mat() ); 
    minMaxLoc( rUnclick, &minValU, &maxValU, &minLocU, &maxLocU, Mat() ); 

	int ret_val;
	ret_ind = 0;
  	if (matchMethod  == CV_TM_SQDIFF || matchMethod == CV_TM_SQDIFF_NORMED) { 
		// Lowest value is best fit
		double minVals[6];
		ret_val = minValC;
		minVals[0] = minValC;
		minVals[1] = minVal1;
		minVals[2] = minVal2;
		minVals[3] = minVal3;
		minVals[4] = minVal4;
		minVals[5] = minValU;
		
		for (int d = 0; d < 6; d++) {
			if (ret_val > minVals[d]) {
				ret_val = minVals[d];
				ret_ind = d;
			}
		}

	} else {
		// Highest value is best fit
		double maxVals[6];
		ret_val = maxValC;
		maxVals[0] = maxValC;
		maxVals[1] = maxVal1;
		maxVals[2] = maxVal2;
		maxVals[3] = maxVal3;
		maxVals[4] = maxVal4;
		maxVals[5] = maxValU;
		
		for (int d = 0; d < 6; d++) {
			if (ret_val < maxVals[d]) {
				ret_val = maxVals[d];
				ret_ind = d;
			}
		}
	}
	return;
}

// Takes an array as input and writes to a file in the current directory
static void makeText(int heatmap[][16]) {
	int k, l;
	ofstream myfile;
	myfile.open("hardcode.txt");
	for (k = 0; k < 16; k++) {
		for (l = 0; l < 16; l++) {
			myfile << heatmap[k][l];
			if (!(k == 15 && l == 15)) {
				myfile << "\n";
			}
		}
	}

	myfile.close();
}

int main(int /*argc*/, char** /*argv*/) {
    static const char* names[] = { "grid.png", 0};
    vector<vector<Point> > squares;

	four = imread( "four.png", 1);
	three = imread( "three.png", 1);
	two = imread( "two.png", 1);
	one = imread( "one.png", 1);
	clicked = imread( "clicked.png", 1);
	unclicked = imread( "unclicked.png", 1);

	Mat image = imread("grid.png", 1);	

	Rect rectang = Rect(795, 333, 961, 961);
	Mat crop = image(rectang);
	int tileW = 60;
	int tileH = 60;
		
	// Drawing the Grid
	//for (int h = 0; h < 16; h++) {
	//	for (int w = 0; w < 16; w++) {
	//	Point start = Point(w*tileW, h*tileH);
	//	Point end = Point((w+1)*tileW, (h+1)*tileH);
	//	rectangle(crop, start, end, Scalar(234, 234, 0), 1, 8);
	//	}
	//}
		
	// Detect each Minesweeper tile as an ROI
	for (int w = 0; w < 16; w++) {
		for (int h = 0; h < 16; h++) {
			Rect roi(w*tileW, h*tileH, tileW, tileH);	
			cropped = crop(roi);
			string s = to_string(w) + to_string(h);
			imshow(s, cropped);
			MatchingMethod( 0, 0 );
			if (ret_ind == 0) {
				cout << "Tile is: Clicked" << endl;
			} else if (ret_ind == 5) {
				cout << "Tile is: Unclicked" << endl;
			} else {
				cout << "Tile is: " << ret_ind << endl;
			}
			int c = waitKey();
			if ( (char)c == 27) {
				continue;
			}
		}
	}
		
	//Mat cropped = crop(roi);
	//imshow(wndname, cropped);

    //int c = waitKey();
    //if( (char)c == 27 )
    //   break;

	//makeText(risk);

    return 0;
}
