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

/*
 * This class is based on QtOpenCVViewerGl from https://github.com/Myzhar/QtOpenCVViewerGl
 */
#ifndef VIEWER_H
#define VIEWER_H
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <opencv2/core/core.hpp>
#include <vector>
#include <QVector>
#include <QPointF>

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

/** \brief Class to display OpenCV Images using OpenGL
  * This Class will draw OpenCV Images using OpenGL.
  * Tracks and Points are drawn with OpenGL on top of the OpenCV Image.
  * This class is based on QtOpenCVViewerGl from https://github.com/Myzhar/QtOpenCVViewerGl
  */ 
class Viewer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit Viewer(QWidget *parent = 0);
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    struct DisplaySettings{
     int showPoints = 1;
     int showTracks = 1;
     float pointSize = 10.0;
     float lineWidth = 10;
     int showContours = 1;
     int minG=0;
     int maxG=255;
     float showPointsVar=0;
    };
    DisplaySettings getDisplaySettings();
    QPointF posToImgCoords(QPoint mpos);
    int addToPolygons(std::vector<std::vector<double> > inPoly,int r=255, int g=255, int b=255, double lineWidth=1.);
    void removePolygons();
    void reRenderImage(){renderImage();};
    void zoom(double v, QPointF o);

signals:
    void    imageSizeChanged( int outW, int outH );

public slots:
    bool    showImage(cv::Mat image);
    bool    showImageWithPoints(cv::Mat image, std::vector<cv::Point2f> cvpoints,std::vector<double> pm=std::vector<double>(),
                                std::vector<std::vector<cv::Point> > contours=std::vector<std::vector<cv::Point> >());
    bool    showImageWithPointsAndTracks(cv::Mat image, std::vector<cv::Point2f> cvpoints, std::vector<std::vector<cv::Point2f> > cvtracks,
                                         std::vector<double> pm=std::vector<double>(), std::vector<double> tm =std::vector<double>(),
                                         std::vector<std::vector<cv::Point> > contours=std::vector<std::vector<cv::Point> >());
    void    setPoints(std::vector<cv::Point2f> cvpoints, std::vector<double> = std::vector<double>(),
                      std::vector<std::vector<cv::Point> > contours=std::vector<std::vector<cv::Point> >());
    void    setTracks(std::vector<std::vector<cv::Point2f> > cvtracks, std::vector<double> = std::vector<double>());
    bool    redraw();
    bool    setDisplaySettings(DisplaySettings set);
    bool    setColRow(int x, int y);
    int     getCol();
    int     getRow();
    bool    setCrop(int x0, int y0, int w, int h);
    bool    moveCrop(int x0, int y0);
    bool    setSelection(double p1x, double p1y, double p2x, double p2y);
    bool    clearSelection();
    bool    setLine(double p1x, double p1y, double p2x, double p2y);
    bool    clearLine();
    void    setMinTrackL(int i);
    bool    addToPolygon(double p1x, double p1y);
    bool    clearPolygon();
    bool    setHighlight(double p1x, double p1y);
    bool    setTrackToHighlight(std::vector<cv::Point2f> cvtrack);
    bool    setROI(int p1x, int p1y, int p2x, int p2y );
    bool    clearROI();
    bool    clearTrackToHighlight();
    
protected:
    void 	initializeGL() override;
    void 	paintGL() override;
    void 	resizeGL(int width, int height) override;
    void        updateScene();
    void        renderImage();

private:
    bool calculate_crop();
    bool calcColor(double c, double& r,double& g,double& b);
    void rescaleLevels();
    bool mSceneChanged;
    QImage mRenderQtImg;
    cv::Mat mOrigImage; 
    cv::Mat mOrigImageBackup;
    QColor mBgColor;
    int mOutH;
    int mOutW;
    float mImgRatio;
    int mPosX;
    int mPosY;
    QPoint m_lastPos;
    int iH;
    int iW;
    int sH;
    int sW;
    QList<QPointF> points;
    std::vector<std::vector< QPointF> > tracks;
    QList<QPointF> polygon;
    std::vector<std::vector< QPointF> > polygons;
    std::vector<std::vector< QPointF> > contoursForPoints;
    std::vector<std::vector<double> > polygonsColorAndLineSize;
    std::vector<double> pointMeta;
    std::vector<double> trackMeta;
    bool havePointMeta;
    bool haveTrackMeta;
    DisplaySettings dSettings;
    int row;
    int col;
    QRect selection;
    QRect crop;
    int cropping;
    int cropNeedsUpdate;
    double s1x;
    double s1y;
    double s2x;
    double s2y;
    bool drawSelection;
    bool drawLine;
    bool drawPolygon;
    bool drawPolygons;
    bool drawContours;
    int minTrackL;
    bool highlight;
    double highlightX;
    double highlightY;
    bool roi;
    int roi_x0;
    int roi_y0;
    int roi_x1;
    int roi_y1;
    bool highlightTrack;
    std::vector< QPointF> trackToHighlight;
    int oldMinG;
    int oldMaxG;
};

#endif // VIEWER_H
 