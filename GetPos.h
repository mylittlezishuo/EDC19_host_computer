#pragma once
#include<opencv2\highgui.hpp>
#include<opencv2\core.hpp>
#include<opencv2\imgproc.hpp>
#include<Windows.h>
#include<conio.h>
#include<iostream>
#include<thread>
#include<MemoryBuffer.h>
#include<filesystem>
#include"Record.h"

enum ColorType
{
	BLACK = 0, GRAY = 1, WHITE = 2, RED = 3, ORANGE = 4, YELLOW = 5, GREEN = 6, CYAN = 7, BLUE = 8, VIOLET = 9, IDLENESS = 10, INTEGRAL = 11, RELISH = 12, PRIME = 13, HUHUHU = 14
};

#define RECTANGLE 1
#define CIRCLE 0;

using namespace std;
using namespace cv;
using namespace ::Windows::Foundation;

//constants and variables
const uint Channel = 1;
const uint WaitTime = 20;
vector<cv::Point> points;

//Data pack
struct pos
{
	Point2d cpos1;
	Point2d cpos2;
	Point2d bpos;
	pos(Point cpos1, Point cpos2, Point bpos, uint rows, uint cols)
	{
		this->cpos1.x = cpos1.x / cols;
		this->cpos1.y = cpos1.y / rows;
		this->cpos2.x = cpos2.x / cols;
		this->cpos2.y = cpos2.y / rows;
		this->bpos.x = bpos.x / cols;
		this->bpos.y = bpos.y / rows;
	}
};

//Perspective Transformation
//[passed]
void Adjust(Mat src, Mat &dst, Point2f onw, Point2f one, Point2f osw, Point2f ose)
{
	Point2f nw(0, 0);
	Point2f sw(0, src.rows);
	Point2f ne(src.cols, 0);
	Point2f se(src.cols, src.rows);
	Mat m = getPerspectiveTransform(new Point2f[4]{ onw,one,osw,ose }, new Point2f[4]{ nw,ne,sw,se });
	warpPerspective(src, dst, m, dst.size(), WARP_FILL_OUTLIERS);
}

//Judge if a color satisfies the HSV values of certain colors.
//[testing]
//For better result, use actual values of certain colors in question.
bool isColorHSV(int colortype, uint hvalue, uint svalue, uint vvalue)
{
	switch (colortype)
	{
	case BLACK:
		return ((hvalue > 0 && hvalue < 180) && (svalue > 0 && svalue < 255) && (vvalue > 0 && vvalue < 46));
	case GRAY:
		return ((hvalue > 0 && hvalue < 180) && (svalue > 0 && svalue < 100) && (vvalue > 46 && vvalue < 220));
	case WHITE:
		return ((hvalue > 0 && hvalue < 180) && (svalue > 0 && svalue < 30) && (vvalue > 221 && vvalue < 255));
	case RED:
		return (((hvalue > 0 && hvalue < 10) || (hvalue > 156 && hvalue < 180)) && (svalue > 43 && svalue < 255) && (vvalue > 46 && vvalue < 255));
	case GREEN:
		return ((hvalue > 35 && hvalue < 77) && (svalue > 100 && svalue < 255) && (vvalue > 46 && vvalue < 255));
	case ORANGE:
		return ((hvalue > 11 && hvalue < 25) && (svalue > 100 && svalue < 255) && (vvalue > 46 && vvalue < 255));
	case YELLOW:
		return ((hvalue > 26 && hvalue < 34) && (svalue > 100 && svalue < 255) && (vvalue > 46 && vvalue < 255));
	case CYAN:
		return ((hvalue > 78 && hvalue < 99) && (svalue > 100 && svalue < 255) && (vvalue > 46 && vvalue < 255));
	case BLUE:
		return ((hvalue > 100 && hvalue < 124) && (svalue > 100 && svalue < 255) && (vvalue > 46 && vvalue < 255));
	case VIOLET:
		return ((hvalue > 125 && hvalue < 155) && (svalue > 100 && svalue < 255) && (vvalue > 46 && vvalue < 255));
	case IDLENESS:
		//write your own judging code here;
		return false;
	case INTEGRAL:
		//write your own judging code here;
		return false;
	case RELISH:
		//write your own judging code here;
		return false;
	case PRIME:
		//write your own judging code here;
		return false;
	case HUHUHU:
		//write your own judging code here;
		return false;
	}
}

//Get the block of a certain color.
//[passed]
void GetImageBlock(Mat& src, Mat& dst, uint colortype)//画出特定颜色区块的函数，该函数有待提高效率
{
	int row = src.rows;
	int channel = src.channels();
	int col = src.cols;
	cvtColor(src, dst, COLOR_BGR2HSV);
	Mat_<Vec3b>::iterator beg = dst.begin<Vec3b>();
	Mat_<Vec3b>::iterator end = dst.end<Vec3b>();
	for (; beg != end; beg++)
	{
		if (
			//((*beg)[0] < 10 || ((*beg)[0]>156)&&(*beg)[0]<180||(*beg)[0]>350) && (((*beg)[1]>60)&&((*beg)[1]<255))&&(((*beg)[2]>60) && ((*beg)[2]<255))
			isColorHSV(colortype, (*beg)[0], (*beg)[1], (*beg)[2]))
		{
			(*beg)[0] = 255;
			(*beg)[1] = 255;
			(*beg)[2] = 255;
		}
		else
		{
			(*beg)[0] = 0;
			(*beg)[1] = 0;
			(*beg)[2] = 0;
		}
	}
}

//Get the centroid.
//[passed]
Point getCentroid(vector<Point> list)
{
	Point result_point(0, 0);
	//Construct a buffer used by the pca analysis  
	int sz = static_cast<int>(list.size());
	Mat data_pts(sz, 2, CV_64FC1);
	for (int i = 0; i < data_pts.rows; ++i)
	{
		data_pts.at<double>(i, 0) = list[i].x;
		data_pts.at<double>(i, 1) = list[i].y;
	}

	//Perform PCA analysis  
	PCA pca_analysis(data_pts, Mat(), CV_PCA_DATA_AS_ROW);

	//Store the center of the object  
	Point cntr = Point(static_cast<int>(pca_analysis.mean.at<double>(0, 0)),
		static_cast<int>(pca_analysis.mean.at<double>(0, 1)));
	return cntr;
}

//Deletes the discrete points and gives a bi-color matrix of contours. Also checks out centroids.
//[passed]
vector<Point> deleteDiscretePoints(Mat &src, bool type = true, bool ifDrawCent = false, uint CentSize = 2, Mat &dest = Mat())
{
	Mat mid1, mid2;
	vector<Vec4i> hierarchy;
	vector<Point> rett;
	if (src.channels() != 1)cvtColor(src, mid1, COLOR_BGR2GRAY, 0);
	threshold(mid1, mid2, 127, 255, 0);
	vector<vector<Point>> contours;
	findContours(mid2, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	Mat dst(src.size(), CV_8UC1);
	//drawContours(dst, contours, -1, Scalar(0));
	vector<vector<Point>> simplifiedctrs(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		if (type)approxPolyDP(Mat(contours[i]), simplifiedctrs[i], 10, true);//该函数的第三个参数应根据实际合排时方块的大小做判断
		else simplifiedctrs[i] = contours[i];
	}
	for (int i = 0; i < simplifiedctrs.size(); i++)
	{
		drawContours(dst, simplifiedctrs, static_cast<int>(i), Scalar(0), 1, 8, hierarchy, 0);
		if (ifDrawCent)
		{
			Point center = getCentroid(contours[i]);
			rett.push_back(center);
			circle(dst, center, CentSize, Scalar(0, 255, 255), -1, 8);
		}
	}
	/*vector<vector<Point>> contours2;
	findContours(dst, contours2, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);*/
	//drawContours(empty, contours2, -1, Scalar(0));*/

	/*{
	Mat elevate(src.size(), CV_8UC1);
	for (int i = 0; i < simplifiedctrs.size(); i++)
	{
	RotatedRect rr = minAreaRect(simplifiedctrs[i]);
	Point2f vertices[4];
	rr.points(vertices);
	for (int j = 0; j < 4; j++)
	{
	line(elevate, vertices[j], vertices[(j + 1) % 4], Scalar());
	}
	}
	return elevate;
	}*/
	dest = dst;
	return rett;
}

//Finds the center of the color block on the car
//[passed]
Point GetCentroidofCar(Mat &src, uint colortype)
{
	Mat mid1, mid2, mid3, mid4;
	blur(src, mid1, Size(5, 5));
	threshold(mid1, mid2, 128, 255, THRESH_BINARY);
	GetImageBlock(mid2, mid3, colortype);
	vector<Point> v;
	v = deleteDiscretePoints(mid3, true, false, 2u, mid4);
	if(!v.empty())return v[0];
	else return Point(0, 0);
	return Point(0, 0);
}

//Finds the center of the ball.
//[passed]
Point GetCentroidofBall(Mat &src, uint colortype)
{
	Mat mid1, mid2, mid3, mid4;
	blur(src, mid1, Size(10, 10));
	GetImageBlock(mid1, mid2, colortype);
	vector<Vec3f> v;
	cvtColor(mid2, mid2, COLOR_BGR2GRAY);
	HoughCircles(mid2, v, HOUGH_GRADIENT, 10, 50, 100, 20, 0, 20);
	for (int i = 0; i < v.size(); i++)
	{
		circle(src, Point(v[0][0], v[0][1]), 5, Scalar(255, 255, 0), -1, 8, 0);
	}
	if (!v.empty())return Point(v[0][0], v[0][1]);
	else return Point(0, 0);
}

//Callback function of mouse event
void callback(int event, int x, int y, int flags, void* userdata)
{
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		points.emplace_back(x, y);
		cout << x << " " << y << endl;
	}
}

//Initializes the camera.
//[writing]
void Camera_Init(const string &Winname, Point2f &nw, Point2f &ne, Point2f &sw, Point2f &se, MouseCallback onmouse = callback, uint channel = 0)
{
	namedWindow(Winname);
	VideoCapture vdc(channel);
	Mat src;
	vdc >> src;
	imshow(Winname, src);
	setMouseCallback(Winname, onmouse);
reinput:
	waitKey();
	if (points.size() != 4)
	{
		points.clear();
		goto reinput;
	}
	nw = points[0];
	ne = points[1];
	sw = points[2];
	se = points[3];
	vdc.release();
	destroyWindow(Winname);
}

//For each turn, the UI calls this function to get the coordinates of the cars and the ball.
//[testing]
void Camera_Loop(Mat &src, Point &cpos1, uint color1, Point &cpos2, uint color2, Point &bpos, int color3, Point2f nw, Point2f ne, Point2f sw, Point2f se)
{
	//Adjust(src, src, nw, ne, sw, se);
	cpos1 = GetCentroidofCar(src, color1);
	cpos2 = GetCentroidofCar(src, color2);
	bpos = GetCentroidofBall(src, color3);
}

//Show a window.
//[passed]
void Camera_Show(Mat &src, Point cpos1, Point cpos2, Point bpos, const string &Winname)
{
	circle(src, cpos1, 5, Scalar(255, 255, 0), -1, 8, 0);
	circle(src, cpos2, 5, Scalar(255, 0, 255), -1, 8, 0);
	circle(src, bpos, 5, Scalar(255, 0, 0), -1, 8, 0);
	imshow(Winname, src);
}

