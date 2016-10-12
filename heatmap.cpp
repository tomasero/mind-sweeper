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

using namespace cv;
using namespace std;


static void help() {
    cout <<
    "\nA program using pyramid scaling, Canny, contours, contour simpification and\n"
    "memory storage (it's got it all folks) to find\n"
    "squares in a list of images pic1-6.png\n"
    "Returns sequence of squares detected on the image.\n"
    "the sequence is stored in the specified memory storage\n"
    "Call:\n"
    "./squares\n"
    "Using OpenCV version %s\n" << CV_VERSION << "\n" << endl;
}


int thresh = 50, N = 11;
const char* wndname = "Square Detection Demo";

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static double angle( Point pt1, Point pt2, Point pt0 ) {
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
static void findSquares(const Mat& image, vector<vector<Point> >& squares) {
    squares.clear();

	// NEW PART (mid-line comment slashes added)
    Mat pyr, timg, gray0(image.size(), CV_8U), gray;

    // down-scale and upscale the image to filter out the noise
    pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
    pyrUp(pyr, timg, image.size());


	// NEW PART:
	//Mat blurred(timg);
	//medianBlur(timg, blurred, 9);
	//Mat gray0(blurred.size(), CV_8U), gray;
	for (int y = 0; y < image.rows; y++) { 
		for (int x = 0; x < image.cols; x++) { 
			for (int c = 0; c < 3; c++) {
      			timg.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( 0.4*( timg.at<Vec3b>(y,x)[c] ));
             }
    	}
    }


    vector<vector<Point> > contours;

    // find squares in every color plane of the image
    for(int c = 0; c < 3; c++) {
        int ch[] = {c, 0};
        mixChannels(&timg, 1, &gray0, 1, ch, 1);

        // try several threshold levels
        for(int l = 0; l < N; l++) {
            // hack: use Canny instead of zero threshold level.
            // Canny helps to catch squares with gradient shading
            if(l == 0) {
                // apply Canny. Take the upper threshold from slider
                // and set the lower to 0 (which forces edges merging)
                Canny(gray0, gray, 0, thresh, 5);
                // dilate canny output to remove potential
                // holes between edge segments
                dilate(gray, gray, Mat(), Point(-1,-1));
            } else {
                // apply threshold if l!=0:
                //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
                gray = gray0 >= (l+1)*255/N;
            }

            // find contours and store them all as a list
            findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

            vector<Point> approx;

            // test each contour
            for (size_t i = 0; i < contours.size(); i++) {
                // approximate contour with accuracy proportional
                // to the contour perimeter
                approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

                // square contours should have 4 vertices after approximation
                // relatively large area (to filter out noisy contours)
                // and be convex.
                // Note: absolute value of an area is used because
                // area may be positive or negative - in accordance with the
                // contour orientation
                if (approx.size() == 4 &&
                    fabs(contourArea(Mat(approx))) > 1000 &&
                    isContourConvex(Mat(approx))) {
                    double maxCosine = 0;

                    for (int j = 2; j < 5; j++) {
                        // find the maximum cosine of the angle between joint edges
                        double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                        maxCosine = MAX(maxCosine, cosine);
                    }

                    // if cosines of all angles are small
                    // (all angles are ~90 degree) then write quandrange
                    // vertices to resultant sequence
                    if( maxCosine < 0.3 )
                        squares.push_back(approx);
                }
            }
        }
    }
}


// the function draws all the squares in the image
static void drawSquares(Mat& image, const vector<vector<Point> >& squares) {
    for (size_t i = 0; i < squares.size(); i++) {
        const Point* p = &squares[i][0];
        int n = (int)squares[i].size();
        polylines(image, &p, &n, 1, true, Scalar(0,0,0), 3, CV_AA);
    }
    imshow(wndname, image);
}

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

// NEWEST PART (took away const identifier of squares)
void getLargestSquare(vector<vector<Point> >& squares, vector<Point>& biggest_square) {
    if (!squares.size()) {
        // no squares detected
        return;
    }

    int max_width = 0;
    int max_height = 0;
    int max_square_idx = 0;
    const int n_points = 4;

    for (size_t i = 0; i < squares.size(); i++) {
        // Convert a set of 4 unordered Points into a meaningful cv::Rect structure.
        Rect rectangle = boundingRect(Mat(squares[i]));

    //  cout << "find_largest_square: #" << i << " rectangle x:" << rectangle.x << " y:" << rectangle.y << " " << rectangle.width << "x" << rectangle.height << endl;

        // Store the index position of the biggest square found
        if ((rectangle.width >= max_width) && (rectangle.height >= max_height)) {
            max_width = rectangle.width;
            max_height = rectangle.height;
            max_square_idx = i;
        }
    }
	// NEWEST PART (second line)
    biggest_square = squares[max_square_idx];
	squares.erase(squares.begin() + max_square_idx);
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
    // "pic2.png", "pic3.png", "pic4.png", "pic5.png", "pic6.png", 0 };
    help();
    namedWindow( wndname, 1 );
    vector<vector<Point> > squares;

    for (int i = 0; names[i] != 0; i++) {
        Mat image = imread(names[i], 1);
        if (image.empty()) {
            cout << "Couldn't load " << names[i] << endl;
            continue;
        }

		Mat img;
    	findSquares(image, squares);
		//Rect rectang;
		//for (size_t i = 0; i < squares.size(); i++) {
    	//	rectang = boundingRect(Mat(squares[i]));
		//	imshow(wndname, rectang);
		//}
		
		// NEW PARTS (both lines)
		vector<Point> largest;
		for (int w = 0; w < 27; w++) {
			getLargestSquare(squares, largest);
		}
	
		// NEWESTEST PART
		Rect rectang = boundingRect(Mat(largest));
		Mat cropped = image(rectang);
		cout << "x: " << rectang.x << ", y: " << rectang.y << ", width: " << rectang.width << ", height: " << rectang.height << endl;
		
		vector<Point> large;
		findSquares(cropped, squares);
		for (int u = 0; u < 1; u++) {
			getLargestSquare(squares, large);
		}
		Rect recta = boundingRect(Mat(large));
		cout << "x: " << recta.x << ", y: " << recta.y << ", width: " << recta.width << ", height: " << recta.height << endl;
		Rect grid = Rect(recta.x + 10, recta.y + 10, recta.width - 21, recta.height - 20);
		Mat crop = cropped(grid);
		
		findSquares(crop, squares);		
	
		// NEW PART (next 2 lines);
		//vector<vector<Point> > helper;
		//helper.push_back(largest);
		//drawSquares(image, helper);

		for (int u = 0; u < 15; u++) {
			getLargestSquare(squares, large);
		}
		//imshow(wndname, crop);
		drawSquares(crop, squares);

        int c = waitKey();
        if( (char)c == 27 )
            break;
    }

	makeText(risk);

    return 0;
}
