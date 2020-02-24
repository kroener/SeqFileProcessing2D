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
#ifndef DISPLAYALLPOINTSWIDGET_H
#define DISPLAYALLPOINTSWIDGET_H

#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QString>
#include <QLabel>
#include <QFileDialog>
#include "MosquitoesInSeq.hpp"
#include "MosquitoRegistry.hpp"
#include "iSeq.hpp"
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
#include "Viewer.h"
#include "ViewerWithMouse.h"
//end opencv
using namespace std;

class Viewer;
class QPushButton;
class QSpinBox;
class QDoubleSpinBox;
class QFileDialog;
class QCheckBox;

class DisplayAllPointsWidget : public QWidget
{

    Q_OBJECT

public:
    DisplayAllPointsWidget();
    int initialize(MosquitoesInSeq* ms, cv::Mat img, int fr, int t, int st,iSeq* currSeq, bool nFD, bool uT = false , int minT = 0 , int maxT  = 0 );
    void setRadius(double v);
    void giveRegistryPointer(MosquitoRegistry* iReg);

public slots:
    void on_updateButton_clicked();
    void on_saveImageButton_clicked();
    int getRadius();
     
private:
    void updateImage();
    cv::Scalar JetColorMap(double x);
    bool saveImage(const QString &fileName);
    ViewerWithMouse *OpenCVViewer;
    QPushButton *saveImageButton, *updateButton;
    QSpinBox *radius, *trackWidth, *from, *to, *step;
    QDoubleSpinBox *fontS;
    QCheckBox *displayTracks;
    QLabel *selectMode;
    cv::Mat iImg;
    cv::Mat oImg;
    MosquitoesInSeq *MosqInSeq;
    bool haveTracks;
    MosquitoRegistry *Registry;
    iSeq* seq;
    bool haveSeq;
    bool isOpen;
    bool nativeFD;
    bool useTrigger;
    int minTrigger;
    int maxTrigger;
    bool canCalcTime;
    double dt;
    double t0; 
};
#endif // DISPLAYALLPOINTSWIDGET_H 
