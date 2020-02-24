/* 
 * Copyright (C) 2020 Christian Kröner, University of Warwick 
 *
 * This file is part of SeqFileProcessing2D.
 *
 * SeqFileProcessing2D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SeqFileProcessing2D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with SeqFileProcessing2D.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MOSQUITOPOSITION_hpp
#define MOSQUITOPOSITION_hpp
#include <string>
//opencv
#if CV_MAJOR_VERSION < 3
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <ctype.h>
#else
#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include <ctype.h>
#endif
//end opencv
using namespace std;
/**
  * \brief Class to store the Position of a single Mosquito at a single time
  */
class MosquitoPosition
{
public:
    MosquitoPosition(double ix, double iy, double iarea, double imaxInt,double icircularity, double iinertia, double iconvexity, double iradius, vector<cv::Point> ic = vector<cv::Point>());
    MosquitoPosition(const MosquitoPosition& a);
    ~MosquitoPosition() {};
    enum flagType
    {
     INVALID = -1,
     VALID = 1,
     KEEPME = 8,
    };
    double getX();
    double getY();
    double getA();
    double getI();
    double getCircularity();
    double getInertia();
    double getConvexity();
    vector<cv::Point> getContour();
    int getFlag();
    int setFlag(flagType i, bool b = true);
    int setFlag(int i,bool replace = true ); 
    int move(int ofx, int ofy);
    bool hasCont();
    bool MosquitoHasContour();
    bool setContour(vector<cv::Point> ic, bool ignoreError = false);
    bool calculateContourStatistics();

private:
    double x;
    double y;
    double area;
    double maxInt;
    int flag;
    string ID;
    double circularity;
    double inertia;
    double convexity; 
    double radius;
    vector<cv::Point> c;
    bool hasContour;
};
#endif //MOSQUITOPOSITION_hpp

