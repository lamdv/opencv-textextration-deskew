// Standard C++ lib
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <numeric>
#include <string>
#include <sstream>
#include <functional>
#include <queue>

using namespace std;

// OpenCV lib
#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

using namespace cv;

static void help();

double frame_contrast_measure(const Mat&img);
double max_value(std::vector<double> v);
double mean(std::vector<double> v);
double medium(std::vector<double> v);
int max_valued_key(vector<double> v);
Mat get_sharpest_frame(std::vector<double> sharpness, std::vector<Mat> frames);

const char* keys =
{
	"{help h|| Display this message}"
	"{@inp | doan1.mp4 | Example file name}"
};

int main(int argc, const char** argv)
{
	// Commandline handling
	CommandLineParser parser(argc, argv, keys);
	if (parser.has("help"))
	{
		help();
		return 0;
	}
	string inp_dir = "..\\Input_Sample\\";
	stringstream inp_path;
	if (parser.has("@inp"))
		inp_path << inp_dir << parser.get<string>("@inp");
	else
	{
		help();
		return 0;
	}
	cout << "Measuring " << inp_path.str() << endl;

	VideoCapture cap;
	cap.open(inp_path.str()); // open the media file

	if (!cap.isOpened())  // check if file is opened
		return -1;

	Mat frame;
	std::vector<Mat> frames; // vector of last 10 frames
	std::vector<double> sharpness;
	int i = 0;	
	//namedWindow("edges", 1); // for debug only
	while (cap.read(frame))
	{
		i++;
		//cap >> frame; // fetch a new frame from cap
		frames.push_back(frame); // add frame to vector
		sharpness.push_back(frame_contrast_measure(frame)); // calculate contrast
		if (i % 24 == 0)
		{
			//imshow("Test",get_sharpest_frame(sharpness, frames));
			printf("%f, %d\n", max_value(sharpness), max_valued_key(sharpness));
			frames.clear();
			sharpness.clear();
		}
		//imshow("edges", frame); // for debug purpose
	};
	printf("Max Contrast: %f\nMean Contrast: %f\nMedian Contrast: %f", max_value(sharpness), mean(sharpness), medium(sharpness));
	getchar();
	return 0;
}

// Image quality measure
double frame_contrast_measure(const Mat&img)
{
	Mat dx, dy;
	Sobel(img, dx, CV_32F, 1, 0, 3);
	Sobel(img, dy, CV_32F, 0, 1, 3);
	magnitude(dx, dy, dx);
	return sum(dx)[0];
}

// vector functions
double max_value(vector<double> v)
{
	double max = v.at(0);
	for (int i = 1; i < v.size(); i++)
	{
		if (max < v.at(i))
		{
			max = v.at(i);
		}
	}
	return max;
}
int max_valued_key(vector<double> v)
{
	double max = v.at(0);
	int max_pos = 0;
	for (int i = 1; i < v.size(); i++)
	{
		if (max < v.at(i))
		{
			max = v.at(i);
			max_pos = i;
		}
	}
	return max_pos;
}
double mean(std::vector<double> v)
{
	return std::accumulate(v.begin(), v.end(), 0.0)/v.size();
}
double medium(std::vector<double> v) {
	std::vector<double> temp;
	temp.assign(v.begin(), v.end());
	std::sort(temp.begin(), temp.end());
	return temp.at(temp.size()/2);
}

static void help()
{

	printf("\nThis sample calculate blurness of the image\n"
		"Call:\n"
		"    /.edge [image_name -- Default is ../data/fruits.jpg]\n\n");
}
Mat get_sharpest_frame(std::vector<double> sharpness, std::vector<Mat> frames)
{
	return frames.at(max_valued_key(sharpness));
}