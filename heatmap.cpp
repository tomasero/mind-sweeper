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
int matchMethod;
int maxTrackbar = 5;
const char* wndname = "Heatmap";

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
	// Hardcoded Example
	int risk[16][16];
	int x = 5;
	int y = 8;

	risk[0][0] = 0;
	risk[0][1] = 0;
	risk[0][2] = 0;
	risk[0][3] = 0;
	risk[0][4] = 0;
	risk[0][5] = 50 + x*4;
	risk[0][6] = 50 + x*4;
	risk[0][7] = 0;
	risk[0][8] = 0;
	risk[0][9] = 0;
	risk[0][10] = 50 + x*7;
	risk[0][11] = 50 + x*5;
	risk[0][12] = 50 + x*3;
	risk[0][13] = 50;
	risk[0][14] = 50;
	risk[0][15] = 50;
	
	risk[1][0] = 0;
	risk[1][1] = 0;
	risk[1][2] = 0;
	risk[1][3] = 0;
	risk[1][4] = 0;
	risk[1][5] = 50 + x*4;
	risk[1][6] = 50 + x*3;
	risk[1][7] = 0;
	risk[1][8] = 0;
	risk[1][9] = 0;
	risk[1][10] = 0;
	risk[1][11] = 0;
	risk[1][12] = 50 + x*5;
	risk[1][13] = 50 + x;
	risk[1][14] = 50;
	risk[1][15] = 50;
	
	risk[2][0] = 0;
	risk[2][1] = 0;
	risk[2][2] = 0;
	risk[2][3] = 0;
	risk[2][4] = 0;
	risk[2][5] = 50 + x*3;
	risk[2][6] = 50 + x*5;
	risk[2][7] = 0;
	risk[2][8] = 0;
	risk[2][9] = 0;
	risk[2][10] = 0;
	risk[2][11] = 0;
	risk[2][12] = 0;
	risk[2][13] = 50 + x*2;
	risk[2][14] = 50;
	risk[2][15] = 50;
	
	risk[3][0] = 0;
	risk[3][1] = 0;
	risk[3][2] = 0;
	risk[3][3] = 0;
	risk[3][4] = 50 + x*5;
	risk[3][5] = 50 + x*2;
	risk[3][6] = 50 + x*4;
	risk[3][7] = 0;
	risk[3][8] = 0;
	risk[3][9] = 0;
	risk[3][10] = 0;
	risk[3][11] = 0;
	risk[3][12] = 0;
	risk[3][13] = 50 + x*3;
	risk[3][14] = 50;
	risk[3][15] = 50;
	
	risk[4][0] = 0;
	risk[4][1] = 0;
	risk[4][2] = 0;
	risk[4][3] = 0;
	risk[4][4] = 50 + x*3;
	risk[4][5] = 50;
	risk[4][6] = 50 + x*3;
	risk[4][7] = 50 + x*5;
	risk[4][8] = 50 + x*10;
	risk[4][9] = 0;
	risk[4][10] = 0;
	risk[4][11] = 0;
	risk[4][12] = 0;
	risk[4][13] = 50 + x*5;
	risk[4][14] = 50 + x*2;
	risk[4][15] = 50;

	risk[5][0] = 0;
	risk[5][1] = 0;
	risk[5][2] = 0;
	risk[5][3] = 0;
	risk[5][4] = 50 + x*3;
	risk[5][5] = 50 + x;
	risk[5][6] = 50 + x;
	risk[5][7] = 50;
	risk[5][8] = 50 + x*6;
	risk[5][9] = 0;
	risk[5][10] = 0;
	risk[5][11] = 0;
	risk[5][12] = 0;
	risk[5][13] = 0;
	risk[5][14] = 50 + x*3;
	risk[5][15] = 50;

	risk[6][0] = 0;
	risk[6][1] = 0;
	risk[6][2] = 50 + x*5;
	risk[6][3] = 50 + x*2;
	risk[6][4] = 50 + x*2;
	risk[6][5] = 0;
	risk[6][6] = 50 + x*3;
	risk[6][7] = 50 + x*2;
	risk[6][8] = 50  +x*4;
	risk[6][9] = 0;
	risk[6][10] = 0;
	risk[6][11] = 0;
	risk[6][12] = 0;
	risk[6][13] = 0;
	risk[6][14] = 50 + x*8;
	risk[6][15] = 50 + y*4;

	risk[7][0] = 50 + x*3;
	risk[7][1] = 50 + x*3;
	risk[7][2] = 50 + x*2;
	risk[7][3] = 50;
	risk[7][4] = 50 + x;
	risk[7][5] = 50 + x;
	risk[7][6] = 50 + x*3;
	risk[7][7] = 0;
	risk[7][8] = 50 + x*4;
	risk[7][9] = 50 + x*5;
	risk[7][10] = 0;
	risk[7][11] = 0;
	risk[7][12] = 0;
	risk[7][13] = 0;
	risk[7][14] = 0;
	risk[7][15] = 50 + y*7;

	risk[8][0] = 50;
	risk[8][1] = 50;
	risk[8][2] = 50;
	risk[8][3] = 50;
	risk[8][4] = 50;
	risk[8][5] = 50;
	risk[8][6] = 50 + x*2;
	risk[8][7] = 50 + x*2;
	risk[8][8] = 50 + x*4;
	risk[8][9] = 50 + x*5;
	risk[8][10] = 50 + x*7;
	risk[8][11] = 50 + x*8;
	risk[8][12] = 0;
	risk[8][13] = 0;
	risk[8][14] = 0;
	risk[8][15] = 50 + y*9;
			
	risk[9][0] = 50 + x*2;
	risk[9][1] = 50 + x*2;
	risk[9][2] = 50 + x*2;
	risk[9][3] = 50;
	risk[9][4] = 50;
	risk[9][5] = 50 + x;
	risk[9][6] = 50 + x;
	risk[9][7] = 50 + x;
	risk[9][8] = 50 + x*3;
	risk[9][9] = 0;
	risk[9][10] = 0;
	risk[9][11] = 0;
	risk[9][12] = 0;
	risk[9][13] = 0;
	risk[9][14] = 0;
	risk[9][15] = 50 + y*6;

	risk[10][0] = 50 + x*2;
	risk[10][1] = 0;
	risk[10][2] = 50 + x*2;
	risk[10][3] = 50 + x;
	risk[10][4] = 50 + x;
	risk[10][5] = 50 + x*2;
	risk[10][6] = 0;
	risk[10][7] = 50 + x;
	risk[10][8] = 50 + x*5;
	risk[10][9] = 0;
	risk[10][10] = 0;
	risk[10][11] = 0;
	risk[10][12] = 0;
	risk[10][13] = 0;
	risk[10][14] = 0;
	risk[10][15] = 50 + y*4;

	risk[11][0] = 50 + x*2;
	risk[11][1] = 50 + x*2;
	risk[11][2] = 50 + x*2;
	risk[11][3] = 50 + x;
	risk[11][4] = 0;
	risk[11][5] = 50 + x*2;
	risk[11][6] = 50 + x;
	risk[11][7] = 50 + x;
	risk[11][8] = 50 + x*4;
	risk[11][9] = 0;
	risk[11][10] = 0;
	risk[11][11] = 0;
	risk[11][12] = 0;
	risk[11][13] = 0;
	risk[11][14] = 0;
	risk[11][15] = 50 + y*2;

	risk[12][0] = 50;
	risk[12][1] = 50;
	risk[12][2] = 50;
	risk[12][3] = 50 + x;
	risk[12][4] = 50 + x;
	risk[12][5] = 50 + x;
	risk[12][6] = 50 + x;
	risk[12][7] = 50 + x*2;
	risk[12][8] = 50 + x*6;
	risk[12][9] = 0;
	risk[12][10] = 0;
	risk[12][11] = 0;
	risk[12][12] = 0;
	risk[12][13] = 0;
	risk[12][14] = 50 + x*4;
	risk[12][15] = 50 + y*1;

	risk[13][0] = 50;
	risk[13][1] = 50;
	risk[13][2] = 50;
	risk[13][3] = 50;
	risk[13][4] = 50;
	risk[13][5] = 50;
	risk[13][6] = 50 + x*2;
	risk[13][7] = 0;
	risk[13][8] = 0;
	risk[13][9] = 0;
	risk[13][10] = 0;
	risk[13][11] = 0;
	risk[13][12] = 0;
	risk[13][13] = 0;
	risk[13][14] = 50 + x*3;
	risk[13][15] = 50;

	risk[14][0] = 50;
	risk[14][1] = 50;
	risk[14][2] = 50;
	risk[14][3] = 50;
	risk[14][4] = 50;
	risk[14][5] = 50;
	risk[14][6] = 50 + x*3;
	risk[14][7] = 0;
	risk[14][8] = 0;
	risk[14][9] = 0;
	risk[14][10] = 0;
	risk[14][11] = 0;
	risk[14][12] = 0;
	risk[14][13] = 0;
	risk[14][14] = 50 + x*2;
	risk[14][15] = 50;

	risk[15][0] = 50;
	risk[15][1] = 50;
	risk[15][2] = 50;
	risk[15][3] = 50;
	risk[15][4] = 50;
	risk[15][5] = 50;
	risk[15][6] = 50 + x*2;
	risk[15][7] = 0;
	risk[15][8] = 0;
	risk[15][9] = 0;
	risk[15][10] = 0;
	risk[15][11] = 50 + x*5;
	risk[15][12] = 50 + x*4;
	risk[15][13] = 50 + x*3;
	risk[15][14] = 50 + x;
	risk[15][15] = 0;

    static const char* names[] = { "bigBlank.png", 0};
	// "blank.png", 0 };
    namedWindow( wndname, 1 );
    vector<vector<Point> > squares;

	four = imread( "four.png", 1);
	three = imread( "three.png", 1);
	two = imread( "two.png", 1);
	one = imread( "one.png", 1);
	clicked = imread( "clicked.png", 1);
	unclicked = imread( "unclicked.png", 1);

	namedWindow( "Helper", CV_WINDOW_AUTOSIZE );
	char* trackbar_label = "Method: \n 0: SQDIFF \n 1: SQDIFF NORMED \n 2: TM CCORR \n 3: TM CCORR NORMED \n 4: TM COEFF \n 5: TM COEFF NORMED";
  	createTrackbar(trackbar_label, "Helper", &matchMethod, maxTrackbar, MatchingMethod);

	
    for (int i = 0; names[i] != 0; i++) {
        Mat image = imread(names[i], 1);
        if (image.empty()) {
            cout << "Couldn't load " << names[i] << endl;
            continue;
        }

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
    }

	makeText(risk);

    return 0;
}
