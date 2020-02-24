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
#ifndef SEGMENTATIONWORKER_H
#define SEGMENTATIONWORKER_H
#include "Image.hpp"
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

#include <QWidget>

/**
  * \brief Worker Class for the segmentation
  */
class SegmentationWorker : public QObject
{
    Q_OBJECT

public:
    SegmentationWorker();
    ~SegmentationWorker();

public slots:
    void process(Image<unsigned char>* a, Image<unsigned char>* b, double minA, double maxA, double fracN, double minThreshold,int erode, int dilute, int blackOnWhite,int medianBlur1 = 0, int medianBlur2 = 0 , int gaussK1 = 0, double gaussS1 = 0, int gaussK2 = 0, double gaussS2 = 0, vector<cv::Point> poly = vector<cv::Point>(), int maskByThreshold = 0, bool doAbs = false);
    void process(Image<unsigned char>* a, Image<unsigned char>* b, double minA, double maxA, double fracN, double minThreshold,int erode, int dilute, int blackOnWhite,cv::Rect roi, int medianBlur1 = 0, int medianBlur2 = 0, int gaussK1 = 0, double gaussS1 = 0, int gaussK2 = 0, double gaussS2 = 0, vector<cv::Point> poly = vector<cv::Point>(), int maskByThreshold = 0, bool doAbs = false);

signals:
    void finished(vector<vector<double> >,double);

private:
    Image<short> *work;
    Image<double> *dwork;
};
#endif // SEGMENTATIONWORKER_H    
