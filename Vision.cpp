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
#include "opencv2\imgproc\imgproc.hpp"
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
int frame_extract(string inp_path, std::vector<Mat> &out);
cv::Mat deskew(Mat &img);
cv::Mat deskew(Mat &img, double angle);
cv::Mat fourier_deskew(Mat &img);
cv::Mat histogram_deskew(Mat &img);
double skew_angle(cv::Mat &img);
void removeSmallBlobs(cv::Mat& im, double size);

void display_image(Mat img)
{
	namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("Display window", img);
	waitKey(0);
	cv::destroyWindow("Display window");
}

const char* keys =
{
	"{help h|| Display this message}"
	"{img || Deskew an image}"//dev purpose
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
	string inp_dir = "";
	stringstream inp_path;
	if (parser.has("@inp"))
		inp_path << inp_dir << parser.get<string>("@inp");
	else
	{
		help();
		return 0;
	}
	if (parser.has("img"))
	{
		Mat img = imread(inp_path.str(), 0);
		cout << "Reading " << inp_path.str() << endl;
		if (img.empty())
		{
			cout << "Error!";
			getchar();
			return 0;
		}
		//cv::resize(img, img, Size(), 0.3, 0.3);
		imshow("Deskewed", deskew(img));
		waitKey(0);
		return 0;
	}
	cout << "\nMeasuring " << inp_path.str() << endl;
	vector<Mat> buffer;
	frame_extract(inp_path.str(), buffer);
	return 0;
}

int frame_extract(string inp_path, std::vector<Mat> &out)
{
	VideoCapture cap;
	cap.open(inp_path); // open the media file

	if (!cap.isOpened())  // check if file is opened
	{
		cout << "\nload failed";
		getchar();
		return -1;
	}

	Mat frame;
	std::vector<Mat> frames; // vector of last 10 frames
	std::vector<double> sharpness;
	int i = 0;
	Mat temp;
	//namedWindow("edges", 1); // for debug only
	while (cap.read(frame))
	{
		i++;
		//cv::medianBlur(frame, frame, 5);
		cv::resize(frame, frame, Size(), 0.3, 0.3);
		frames.push_back(frame); // add frame to vector
		sharpness.push_back(frame_contrast_measure(frame)); // calculate contrast
		if (i % 96 == 0)
		{
			cout << max_value(sharpness) << ", " << max_valued_key(sharpness) << "\n";
			temp = get_sharpest_frame(sharpness, frames);
			//display_image(temp);
			Mat _temp;
			double min, max;
			cv::minMaxLoc(temp, &min, &max);
			cv::cvtColor(temp, temp, cv::COLOR_RGB2GRAY);
			//temp = Scalar::all(255) - temp;
			//cv::adaptiveThreshold(temp, temp, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 55, 2);
			//cv::GaussianBlur(temp, temp, cv::Size(5, 5), 0, 0);
			cv::threshold(temp, temp, (min+max)/2, 255, cv::THRESH_BINARY_INV);
			//dilate(temp, temp, cv::getStructuringElement(cv::MORPH_CROSS, Size(3, 3), Point(-1, -1)));
			/*display_image(temp);
			display_image(fourier_deskew(temp));*/
			/*removeSmallBlobs(temp, 50);*/
			//cv::morphologyEx(temp, _temp, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11)));
			//cv::morphologyEx(_temp, _temp, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11)));
			//display_image(temp);
			//display_image(fourier_deskew(temp));
			//display_image(deskew(temp));
			stringstream _out;
			_out << "Output_Doan2\\Global\\"<<i/96<<".jpg";
			vector<int> params;
			params.push_back(CV_IMWRITE_JPEG_QUALITY);
			params.push_back(95);
			cv::cvtColor(deskew(temp), temp, cv::COLOR_GRAY2RGB);
			imwrite(_out.str(), temp, params);
			out.push_back(temp);
			frames.clear();
			sharpness.clear();
		}
		//imshow("edges", frame); // for debug purpose
	};
	cout << max_value(sharpness) << ", " << max_valued_key(sharpness) << "\n";
	temp = get_sharpest_frame(sharpness, frames);
	out.push_back(temp);
	frames.clear();
	sharpness.clear();
	//printf("Max Contrast: %f\nMean Contrast: %f\nMedian Contrast: %f", max_value(sharpness), mean(sharpness), medium(sharpness));
	cout << "Completed";
	return 0;
}

cv::Mat deskew(Mat & img)
{
	return deskew(img, skew_angle(img));
}

// Image quality measure
double frame_contrast_measure(const Mat&img)
{
	Mat dx, dy, temp;
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
	getchar();
}
Mat get_sharpest_frame(std::vector<double> sharpness, std::vector<Mat> frames)
{
	return frames.at(max_valued_key(sharpness));
}

double skew_angle(cv::Mat & img)
{
	std::vector<cv::Vec4i> lines;
	cv::Mat _temp;
	Size size = img.size();
	cv::Canny(img, _temp, 50, 200, 3);
	cv::HoughLinesP(_temp, lines, 1, CV_PI / 180, 10, size.width / 2.f, 100);
	cv::Mat disp_lines(size, CV_8UC1, cv::Scalar(0, 0, 0));
	double angle = 0.;
	unsigned nb_lines = lines.size();
	for (unsigned i = 0; i < nb_lines; ++i)
	{
		cv::line(disp_lines, cv::Point(lines[i][0], lines[i][1]),
			cv::Point(lines[i][2], lines[i][3]), cv::Scalar(255, 0, 0));
		angle += atan2((double)lines[i][3] - lines[i][1],
			(double)lines[i][2] - lines[i][0]);
	}
	cout << "No of lines: " << nb_lines << endl;
	angle /= nb_lines; // mean angle, in radians.
	angle *= 180 / CV_PI;
	if (angle > 45)
		angle = 90 - angle;
	cout << "Skew angle: " << angle << endl;
	//display_image(disp_lines);
	return angle;
}

void removeSmallBlobs(cv::Mat & im, double size)
{
	{
		// Only accept CV_8UC1
		if (im.channels() != 1 || im.type() != CV_8U)
			return;

		// Find all contours
		std::vector<std::vector<cv::Point> > contours;
		cv::findContours(im.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		for (int i = 0; i < contours.size(); i++)
		{
			// Calculate contour area
			double area = cv::contourArea(contours[i]);

			// Remove small objects by drawing the contour with black color
			if (area <= size)
				cv::drawContours(im, contours, i, CV_RGB(0, 0, 0), -1);
		}
	}
}

cv::Mat deskew(Mat &img, double angle)
{
	std::vector<cv::Point> points;
	cv::Mat_<uchar>::iterator it = img.begin<uchar>();
	cv::Mat_<uchar>::iterator end = img.end<uchar>();
	for (; it != end; ++it)
		if (*it)
			points.push_back(it.pos());

	cv::RotatedRect box = cv::minAreaRect(cv::Mat(points));
	cv::Mat rot_mat = cv::getRotationMatrix2D(box.center, angle, 1);
	cv::Mat rotated;
	cv::warpAffine(img, rotated, rot_mat, img.size(), cv::INTER_CUBIC);
	return rotated;
}

cv::Mat fourier_deskew(Mat & img)
{
	// OpenCV Fourier Trasnform example
	Mat padded;  //expand input image to optimal size
	int m = getOptimalDFTSize(img.rows);
	int n = getOptimalDFTSize(img.cols); // on the border add zero values
	copyMakeBorder(img, padded, 0, m - img.rows, 0, n - img.cols, BORDER_CONSTANT, Scalar::all(0));

	Mat planes[] = { Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F) };
	Mat complexI;
	merge(planes, 2, complexI);

	dft(complexI, complexI);

	split(complexI, planes);
	magnitude(planes[0], planes[1], planes[0]);
	Mat magI = planes[0];

	magI += Scalar::all(1);
	log(magI, magI);

	magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

	int cx = magI.cols / 2;
	int cy = magI.rows / 2;

	Mat q0(magI, Rect(0, 0, cx, cy));
	Mat q1(magI, Rect(cx, 0, cx, cy));
	Mat q2(magI, Rect(0, cy, cx, cy));
	Mat q3(magI, Rect(cx, cy, cx, cy));

	Mat tmp;
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);

	normalize(magI, magI, 0, 1, CV_MINMAX); // Transform the matrix with float values into a viewable image form (float between values 0 and 1).
	

	return magI;
}

cv::Mat histogram_deskew(Mat & img)
{
	cv::Mat hist;

	return hist
;
}


