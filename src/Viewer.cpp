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
#include "Viewer.h"
#include <iostream>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <vector>

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

Viewer::Viewer(QWidget *parent) :
    QOpenGLWidget(parent)
{
    mSceneChanged = false;
    mBgColor = QColor::fromRgb(128, 128, 128);

    row = 0;
    col = 0;
    mOutH = 0;
    mOutW = 0;
    mImgRatio = 4.0f/3.0f;
    iH = 0;
    iW = 0;

    mPosX = 0;
    mPosY = 0;
    
    cropping=0;
    selection.setRect(0,0,0,0);	
    crop.setRect(0,0,0,0);	
    cropNeedsUpdate=1;
    s1x=0;
    s1y=0;
    s2x=0;
    s2y=0;
    drawSelection=false;
    drawLine=false;
    drawPolygon=false;
    drawPolygons=false;
    drawContours=false;
    minTrackL=2;
    pointMeta=std::vector<double>();
    trackMeta=std::vector<double>();
    havePointMeta=false;
    haveTrackMeta=false;
    highlight=false;
    highlightTrack=false;
    highlightX=0;
    highlightY=0;
    roi=false;
    roi_x0=0;
    roi_y0=0;
    roi_x1=0;
    roi_y1=0;
    oldMinG=0;
    oldMaxG=255;
}

void Viewer::initializeGL()
{
    initializeOpenGLFunctions();
    float r = ((float)mBgColor.darker().red())/255.0f;
    float g = ((float)mBgColor.darker().green())/255.0f;
    float b = ((float)mBgColor.darker().blue())/255.0f;
    glClearColor(r,g,b,1.0f);
}

QSize Viewer::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize Viewer::sizeHint() const
{
    return QSize(400, 200);
}

void Viewer::resizeGL(int width, int height)
{
    int oldMOutH=mOutH;
    int oldMOutW=mOutW;
    int oldMPosX=mPosX;
    int oldMPosY=mPosY;
    width*=devicePixelRatioF();
    height*=devicePixelRatioF();
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    if(sH!=height || sW!=width) cropNeedsUpdate=1;
    sH=height;
    sW=width;
    mOutH = width/mImgRatio;
    mOutW = width;
    if(mOutH>height)
    {
        mOutW = height*mImgRatio;
        mOutH = height;
    }
    emit imageSizeChanged( mOutW, mOutH );
    mPosX = (width-mOutW)/2;
    mPosY = (height-mOutH)/2;
    if((mPosX!=oldMPosX && mPosY!=oldMPosY && mOutH!=oldMOutH && mOutW!=oldMOutW ) || mSceneChanged)
    {
       mSceneChanged = true;
       updateScene();
    }
}

void Viewer::updateScene()
{
    if( mSceneChanged && this->isVisible() )
    {
       update();
       mSceneChanged=false;
    }
}

void Viewer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(this->size().width()!=mRenderQtImg.width())
        resizeGL(this->size().width(),this->size().height());
    renderImage();
    mSceneChanged = false;
}

void Viewer::renderImage()
{
    glClear(GL_COLOR_BUFFER_BIT);
    if (!mRenderQtImg.isNull())
    {
        glLoadIdentity();
        rescaleLevels();
        QImage image;
        if(!cropping)
        {
        glPushMatrix();
        {
            int imW = mRenderQtImg.width();
            int imH = mRenderQtImg.height();
            iH=imH;
            iW=imW;
            if( imW != this->size().width() &&
                    imH != this->size().height() )
            {
                image = mRenderQtImg.scaled(
                            QSize(mOutW,mOutH),
                            Qt::KeepAspectRatio,
                            Qt::SmoothTransformation
                        );
            }
            else
                image = mRenderQtImg;
            glRasterPos2i( mPosX, mPosY );
            imW = image.width();
            imH = image.height();
            glDrawPixels( imW, imH, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
            if(highlightTrack)
            {
              if(trackToHighlight.size()>2)
              {
                glLineWidth(15.0);
                glBegin( GL_LINE_STRIP );
                double r=1;
                double g=1;
                double b=0;
                glColor3f( r,g,b);
                for(int i=0; i< trackToHighlight.size(); ++i)
                {
                    glVertex2f(trackToHighlight[i].x()/iW*imW+mPosX,(iH-trackToHighlight[i].y())/iH*imH+mPosY);
                }
                glEnd();
              }
            }
            if(dSettings.showPoints)
            {
            if(dSettings.showPointsVar>0)
            {
            double o1=dSettings.showPointsVar*0.92387953;
            double o2=dSettings.showPointsVar*0.38268343;
            double r=1;
            double g=1;
            double b=1;
            glColor3f( r,g,b);
            for(int i=0; i< points.size(); ++i)
            {
                if(havePointMeta){
                    calcColor(pointMeta[i],r,g,b);
                    glColor3f(r,g,b);
                }
                glLineWidth(1);
                glBegin( GL_LINE_LOOP );
                double x0=points.at(i).x();
                double y0=points.at(i).y();
                double p0x=x0+o1;
                double p0y=y0+o2;
                double p1x=x0+o2;
                double p1y=y0+o1;
                double p2x=x0-o2;
                double p2y=y0+o1;
                double p3x=x0-o1;
                double p3y=y0+o2;
                double p4x=x0-o1;
                double p4y=y0-o2;
                double p5x=x0-o2;
                double p5y=y0-o1;
                double p6x=x0+o2;
                double p6y=y0-o1;
                double p7x=x0+o1;
                double p7y=y0-o2;

                glVertex2f(p0x/iW*imW+mPosX,(iH-p0y)/iH*imH+mPosY);
                glVertex2f(p1x/iW*imW+mPosX,(iH-p1y)/iH*imH+mPosY);
                glVertex2f(p2x/iW*imW+mPosX,(iH-p2y)/iH*imH+mPosY);
                glVertex2f(p3x/iW*imW+mPosX,(iH-p3y)/iH*imH+mPosY);
                glVertex2f(p4x/iW*imW+mPosX,(iH-p4y)/iH*imH+mPosY);
                glVertex2f(p5x/iW*imW+mPosX,(iH-p5y)/iH*imH+mPosY);
                glVertex2f(p6x/iW*imW+mPosX,(iH-p6y)/iH*imH+mPosY);
                glVertex2f(p7x/iW*imW+mPosX,(iH-p7y)/iH*imH+mPosY);
                glEnd();
            }
            }
            else
            {
            glPointSize(dSettings.pointSize);
            glBegin( GL_POINTS );
            double r=0;
            double g=1.;
            double b=0;
            glColor3f(r,g,b);
            for(int i=0; i< points.size(); ++i)
            {
                if(havePointMeta){
                    calcColor(pointMeta[i],r,g,b);
                    glColor3f(r,g,b);
                }
                glVertex2f(points.at(i).x()/iW*imW+mPosX,(iH-points.at(i).y())/iH*imH+mPosY);
            }
            glEnd();
            }
            }
            if(dSettings.showTracks)
            {
            for(int lineno=0; lineno<tracks.size(); lineno++)
            {
              if(tracks[lineno].size()>=minTrackL)
              {
                glLineWidth(dSettings.lineWidth);
                glBegin( GL_LINE_STRIP );
                double r=1;
                double g=0;
                double b=0;
                if(haveTrackMeta){
                    calcColor(trackMeta[lineno],r,g,b);
                }
                glColor3f( r,g,b);
                
                for(int i=0; i< tracks[lineno].size(); ++i)
                {
                    glVertex2f(tracks[lineno][i].x()/iW*imW+mPosX,(iH-tracks[lineno][i].y())/iH*imH+mPosY);
                }
                glEnd();
              }
            }
            }

            if(drawLine)
            {
            glLineWidth(1);
            glBegin( GL_LINE_STRIP  );
            double r=1;
            double g=1;
            double b=1;
            glColor3f( r,g,b);
            glVertex2f(s1x/iW*imW+mPosX,(iH-s1y)/iH*imH+mPosY);
            glVertex2f(s2x/iW*imW+mPosX,(iH-s2y)/iH*imH+mPosY);
            glEnd();            
            }

            if(drawSelection)
            {
            glLineWidth(1);
            glBegin( GL_LINE_LOOP  );
            double r=1;
            double g=1;
            double b=1;
            glColor3f( r,g,b);
            glVertex2f(s1x/iW*imW+mPosX,(iH-s1y)/iH*imH+mPosY);
            glVertex2f(s1x/iW*imW+mPosX,(iH-s2y)/iH*imH+mPosY);
            glVertex2f(s2x/iW*imW+mPosX,(iH-s2y)/iH*imH+mPosY);
            glVertex2f(s2x/iW*imW+mPosX,(iH-s1y)/iH*imH+mPosY);
            glEnd();            
            }
            if(drawPolygon)
            {
                glLineWidth(dSettings.lineWidth);
                glBegin( GL_LINE_STRIP );
                double r=1;
                double g=1;
                double b=1;
                glColor3f( r,g,b);
                for(int i=0; i< polygon.size(); ++i)
                {
                   glVertex2f(polygon[i].x()/iW*imW+mPosX,(iH-polygon[i].y())/iH*imH+mPosY);
                }
                glEnd();
            }

            if(drawContours && dSettings.showContours)
            {
                if(contoursForPoints.size()>0)
                {
                  for(int p=0; p<contoursForPoints.size(); p++)
                  {
                    glLineWidth(1);
                    glBegin( GL_LINE_STRIP );
                    double r=1;
                    double g=1;
                    double b=1;
                    glColor3f( r,g,b);
                    for(int i=0; i< contoursForPoints[p].size(); ++i)
                    {
                       glVertex2f(contoursForPoints[p][i].x()/iW*imW+mPosX,(iH-contoursForPoints[p][i].y())/iH*imH+mPosY);
                    }
                    glEnd();                  
                  }
                }
            }
            if(highlight)
            {
                glLineWidth(2);
                glBegin( GL_LINE_STRIP );
                glColor3f( 1,0,0);
                double x=highlightX;
                double y=highlightY;
                glVertex2f((x-15)/iW*imW+mPosX,(iH-y)/iH*imH+mPosY);
                glVertex2f((x+15)/iW*imW+mPosX,(iH-y)/iH*imH+mPosY);
                glEnd();
                glBegin( GL_LINE_STRIP );
                glColor3f( 1,0,0);
                glVertex2f((x)/iW*imW+mPosX,(iH-(y-15))/iH*imH+mPosY);
                glVertex2f((x)/iW*imW+mPosX,(iH-(y+15))/iH*imH+mPosY);
                glEnd();
            }
            if(roi)
            {
                glLineWidth(10);
                glBegin( GL_LINE_LOOP );
                glColor3f( 0,0,1);
                glVertex2f(roi_x0/double(iW)*imW+mPosX,(iH-roi_y0)/double(iH)*imH+mPosY);
                glVertex2f(roi_x0/double(iW)*imW+mPosX,(iH-roi_y1)/double(iH)*imH+mPosY);
                glVertex2f(roi_x1/double(iW)*imW+mPosX,(iH-roi_y1)/double(iH)*imH+mPosY);
                glVertex2f(roi_x1/double(iW)*imW+mPosX,(iH-roi_y0)/double(iH)*imH+mPosY);
                glEnd();            
            }
        }
        glPopMatrix();
        }
        else{
        glPushMatrix();
        {
            if(cropNeedsUpdate)
             calculate_crop();
            QImage c=mRenderQtImg.copy(crop);
            int imW = c.width();
            int imH = c.height();
            iH=mRenderQtImg.height();
            iW=mRenderQtImg.width();
            if( imW != this->size().width() &&
                    imH != this->size().height() )
            {
                image = c.scaled(
                            QSize(sW,sH),
                            Qt::IgnoreAspectRatio,
                            Qt::SmoothTransformation
                        );
            }
            else
                image = c;
            glRasterPos2i( 0, 0 );
            imW = image.width();
            imH = image.height();

            glDrawPixels( imW, imH, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

           if(highlightTrack)
            {
              if(trackToHighlight.size()>2)
              {
                glLineWidth(15.0);
                glBegin( GL_LINE_STRIP );
                double r=1;
                double g=1;
                double b=0;
                glColor3f( r,g,b);
                for(int i=0; i< trackToHighlight.size(); ++i)
                {
                    glVertex2f((trackToHighlight[i].x()-crop.x())/(float)crop.width()*imW,
                    (crop.height()-(trackToHighlight[i].y()-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                }
                glEnd();
              }
            }
 
            if(dSettings.showPoints)
            {
            if(dSettings.showPointsVar>0)
            {
            double o1=dSettings.showPointsVar*0.92387953;
            double o2=dSettings.showPointsVar*0.38268343;

            double r=1;
            double g=1;
            double b=1;
            glColor3f( r,g,b);
            for(int i=0; i< points.size(); ++i)
            {
                if(havePointMeta){
                    calcColor(pointMeta[i],r,g,b);
                    glColor3f(r,g,b);
                }

                glLineWidth(1);
                glBegin( GL_LINE_LOOP );
                double x0=points.at(i).x();
                double y0=points.at(i).y();
                double p0x=x0+o1;
                double p0y=y0+o2;
                double p1x=x0+o2;
                double p1y=y0+o1;
                double p2x=x0-o2;
                double p2y=y0+o1;
                double p3x=x0-o1;
                double p3y=y0+o2;
                double p4x=x0-o1;
                double p4y=y0-o2;
                double p5x=x0-o2;
                double p5y=y0-o1;
                double p6x=x0+o2;
                double p6y=y0-o1;
                double p7x=x0+o1;
                double p7y=y0-o2;
            
                glVertex2f((p0x-crop.x())/(float)crop.width()*imW,
                  (crop.height()-(p0y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glVertex2f((p1x-crop.x())/(float)crop.width()*imW,
                  (crop.height()-(p1y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glVertex2f((p2x-crop.x())/(float)crop.width()*imW,
                  (crop.height()-(p2y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glVertex2f((p3x-crop.x())/(float)crop.width()*imW,
                  (crop.height()-(p3y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glVertex2f((p4x-crop.x())/(float)crop.width()*imW,
                  (crop.height()-(p4y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glVertex2f((p5x-crop.x())/(float)crop.width()*imW,
                  (crop.height()-(p5y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glVertex2f((p6x-crop.x())/(float)crop.width()*imW,
                  (crop.height()-(p6y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glVertex2f((p7x-crop.x())/(float)crop.width()*imW,
                  (crop.height()-(p7y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glEnd();
            }
            }
            else
            {
            glPointSize(dSettings.pointSize);
            glBegin( GL_POINTS );
            double r=0;
            double g=1.;
            double b=0;
            glColor3f(r,g,b);
            
            for(int i=0; i< points.size(); ++i)
            {
                if(havePointMeta){
                    calcColor(pointMeta[i],r,g,b);
                    glColor3f(r,g,b);
                }
                glVertex2f((points.at(i).x()-crop.x())/(float)crop.width()*imW,
                           (crop.height()-(points.at(i).y()-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
            }
            glEnd();
            }
            }
            if(dSettings.showTracks)
            {
            for(int lineno=0; lineno<tracks.size(); lineno++)
            {
              if(tracks[lineno].size()>=minTrackL)
              {
                glLineWidth(dSettings.lineWidth);
                glBegin( GL_LINE_STRIP );
                double r=1;
                double g=0;
                double b=0;
                if(haveTrackMeta){
                    calcColor(trackMeta[lineno],r,g,b);
                }
                glColor3f( r,g,b);
                for(int i=0; i< tracks[lineno].size(); ++i)
                {
                    glVertex2f((tracks[lineno][i].x()-crop.x())/(float)crop.width()*imW,
                    (crop.height()-(tracks[lineno][i].y()-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                }
                glEnd(); 
              }           
            }
            }
            if(drawSelection)
            {
            glLineWidth(1);
            glBegin( GL_LINE_LOOP  );
            double r=1;
            double g=1;
            double b=1;
            glColor3f( r,g,b);
            glVertex2f((s1x-crop.x())/float(crop.width())*imW, (crop.height()-(s1y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
            glVertex2f((s1x-crop.x())/float(crop.width())*imW, (crop.height()-(s2y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
            glVertex2f((s2x-crop.x())/float(crop.width())*imW, (crop.height()-(s2y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
            glVertex2f((s2x-crop.x())/float(crop.width())*imW, (crop.height()-(s1y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
            glEnd();            
            }
            if(drawLine)
            {
            glLineWidth(1);
            glBegin( GL_LINE_STRIP  );
            double r=1;
            double g=1;
            double b=1;
            glColor3f( r,g,b);
            glVertex2f((s1x-crop.x())/float(crop.width())*imW, (crop.height()-(s1y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
            glVertex2f((s2x-crop.x())/float(crop.width())*imW, (crop.height()-(s2y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
            glEnd();            
            }
            if(drawPolygon)
            {
                glLineWidth(dSettings.lineWidth);
                glBegin( GL_LINE_STRIP );
                double r=1;
                double g=1;
                double b=1;
                glColor3f( r,g,b);
                for(int i=0; i< polygon.size(); ++i)
                {
                     glVertex2f((polygon[i].x()-crop.x())/(float)crop.width()*imW,
                      (crop.height()-(polygon[i].y()-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                }
                glEnd();
            }

            if(drawContours && dSettings.showContours)
            {
                if(contoursForPoints.size()>0)
                {
                  for(int p=0; p<contoursForPoints.size(); p++)
                  {
                    glLineWidth(1);
                    glBegin( GL_LINE_STRIP );
                    double r=1;
                    double g=1;
                    double b=1;
                    glColor3f( r,g,b);
                    for(int i=0; i< contoursForPoints[p].size(); ++i)
                    {
                         glVertex2f((contoursForPoints[p][i].x()-crop.x())/(float)crop.width()*imW,
                            (crop.height()-(contoursForPoints[p][i].y()-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                    }
                    glEnd();                  
                  }
                }
            }

            if(highlight)
            {
                glLineWidth(2);
                glBegin( GL_LINE_STRIP );
                glColor3f( 1,0,0);
                double x=highlightX;
                double y=highlightY;
                glVertex2f(((x+15)-crop.x())/(float)crop.width()*imW,
                    (crop.height()-(y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glVertex2f(((x-15)-crop.x())/(float)crop.width()*imW,
                    (crop.height()-(y-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glEnd();            
                glBegin( GL_LINE_STRIP );
                glColor3f( 1,0,0);
                glVertex2f(((x)-crop.x())/(float)crop.width()*imW,
                    (crop.height()-((y+15)-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glVertex2f(((x)-crop.x())/(float)crop.width()*imW,
                    (crop.height()-((y-15)-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glEnd();            
            }

            if(roi)
            {
                glLineWidth(10);
                glBegin( GL_LINE_STRIP  );
                glColor3f( 0,0,1);
                glVertex2f((roi_x0-crop.x())/float(crop.width())*imW, (crop.height()-(roi_y0-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glVertex2f((roi_x0-crop.x())/float(crop.width())*imW, (crop.height()-(roi_y1-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glVertex2f((roi_x1-crop.x())/float(crop.width())*imW, (crop.height()-(roi_y1-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glVertex2f((roi_x1-crop.x())/float(crop.width())*imW, (crop.height()-(roi_y0-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glVertex2f((roi_x0-crop.x())/float(crop.width())*imW, (crop.height()-(roi_y0-(iH-(crop.y()+crop.height()))))/(float)crop.height()*imH);
                glEnd(); 
            }           
            }
        glPopMatrix();   
        }
        glFlush();
    }
}

bool Viewer::showImage(cv::Mat image)
{
    highlight=false;
    highlightTrack=false;
    points.clear();
    tracks.clear();
    oldMinG=0;
    oldMaxG=255;
    image.copyTo(mOrigImage);
    image.copyTo(mOrigImageBackup);
    cv::flip(mOrigImage,mOrigImage,0);
    mImgRatio = (float)image.cols/(float)image.rows;
    if( mOrigImage.channels() == 3)
        mRenderQtImg = QImage((const unsigned char*)(mOrigImage.data),
                              mOrigImage.cols, mOrigImage.rows,
                              mOrigImage.step, QImage::Format_RGB888);
    else if( mOrigImage.channels() == 1)
        mRenderQtImg = QImage((const unsigned char*)(mOrigImage.data),
                              mOrigImage.cols, mOrigImage.rows,
                              mOrigImage.step, QImage::Format_Indexed8);
    else
        return false;
    mSceneChanged = true;
    updateScene();
    return true;
}

bool Viewer::showImageWithPoints(cv::Mat image, std::vector<cv::Point2f> cvpoints,std::vector<double> pm/*=std::vector<double>()*/,
                                            std::vector<std::vector<cv::Point> > contours/*=std::vector<std::vector<cv::Point> >()*/)
{
    highlight=false;
    highlightTrack=false;
    oldMinG=0;
    oldMaxG=255;
    
    if(dSettings.showContours && contours.size()>0)
    {
     cv::drawContours(image, contours, -1, cv::Scalar(255), 1);
    }
    image.copyTo(mOrigImage);
    image.copyTo(mOrigImageBackup);
    tracks.clear();
    cv::flip(mOrigImage,mOrigImage,0);
    mImgRatio = (float)image.cols/(float)image.rows;

    if( mOrigImage.channels() == 3)
        mRenderQtImg = QImage((const unsigned char*)(mOrigImage.data),
                              mOrigImage.cols, mOrigImage.rows,
                              mOrigImage.step, QImage::Format_RGB888);
    else if( mOrigImage.channels() == 1)
        mRenderQtImg = QImage((const unsigned char*)(mOrigImage.data),
                              mOrigImage.cols, mOrigImage.rows,
                              mOrigImage.step, QImage::Format_Indexed8);
    else
        return false;
    points.clear();
    for(unsigned long i=0; i<cvpoints.size(); i++)
    {
        QPointF p(cvpoints[i].x,cvpoints[i].y);
        points.push_back(p);
    }
    if(pm.size()>0 && pm.size()==cvpoints.size())
    {
     pointMeta.clear();
     for(int i=0; i<pm.size(); i++)
      pointMeta.push_back(pm[i]);
     havePointMeta=false;
    }
    else
    {
     pointMeta.clear();
     havePointMeta=false;
    }
    mSceneChanged = true;
    updateScene();
    return true;
}

bool Viewer::showImageWithPointsAndTracks(cv::Mat image, std::vector<cv::Point2f> cvpoints, std::vector<std::vector<cv::Point2f> > cvtracks, 
                                                     std::vector<double> pm/*=std::vector<double>()*/, std::vector<double> tm /*=std::vector<double>()*/,
                                                     std::vector<std::vector<cv::Point> > contours/*=std::vector<std::vector<cv::Point> >()*/)
{
    highlight=false;
    highlightTrack=false;
    oldMinG=0;
    oldMaxG=255;
    if(dSettings.showContours && contours.size()>0)
    {
     cv::drawContours(image, contours, -1, cv::Scalar(255), 1);
    }
    image.copyTo(mOrigImage);
    image.copyTo(mOrigImageBackup);
    cv::flip(mOrigImage,mOrigImage,0);
    mImgRatio = (float)image.cols/(float)image.rows;

    if( mOrigImage.channels() == 3)
        mRenderQtImg = QImage((const unsigned char*)(mOrigImage.data),
                              mOrigImage.cols, mOrigImage.rows,
                              mOrigImage.step, QImage::Format_RGB888);
    else if( mOrigImage.channels() == 1)
        mRenderQtImg = QImage((const unsigned char*)(mOrigImage.data),
                              mOrigImage.cols, mOrigImage.rows,
                              mOrigImage.step, QImage::Format_Indexed8);
    else
        return false;
    points.clear();
    for(unsigned long i=0; i<cvpoints.size(); i++)
    {
        QPointF p(cvpoints[i].x,cvpoints[i].y);
        points.push_back(p);
    }
    tracks.clear();
    for(unsigned long lineno=0; lineno<cvtracks.size(); lineno++)
    {
        std::vector<QPointF> tmp;
        for(unsigned long i=0; i<cvtracks[lineno].size(); i++)
        {
            QPointF p(cvtracks[lineno][i].x,cvtracks[lineno][i].y);
            tmp.push_back(p);
        }
        tracks.push_back(tmp);
    }
    if(pm.size()>0 && pm.size()==cvpoints.size())
    {
     pointMeta.clear();
     for(int i=0; i<pm.size(); i++)
      pointMeta.push_back(pm[i]);
     havePointMeta=true;
    }
    else
    {
     pointMeta.clear();
     havePointMeta=false;
    }
    
    if(tm.size()>0 && tm.size()==cvtracks.size())
    {
     trackMeta.clear();
     for(int i=0; i<tm.size(); i++)
      trackMeta.push_back(tm[i]);
     haveTrackMeta=true;
    }
    else
    {
     trackMeta.clear();
     haveTrackMeta=false;
    }
    mSceneChanged = true;
    updateScene();
    return true;
}

bool Viewer::redraw()
{
    updateScene();
    return true;
}

Viewer::DisplaySettings Viewer::getDisplaySettings()
{
  return dSettings;
}

bool Viewer::setDisplaySettings(DisplaySettings set)
{
  bool ret=false;
  dSettings=set;
  redraw();
  return ret;
}

bool Viewer::setColRow(int x, int y)
{
 col=x;
 row=y;
 return true;
}

int Viewer::getCol()
{
 return col;
}

int Viewer::getRow()
{
 return row;
}

QPointF Viewer::posToImgCoords(QPoint mpos)
{
    const QPoint m = this->mapFromGlobal( mpos );
    QPointF tmp;
    double x=-1;
    double y=-1;
    if(!cropping)
    {
     x=double(m.x()*devicePixelRatioF()-mPosX)/double(sW-2.*mPosX)*iW;
     y=double(m.y()*devicePixelRatioF()-mPosY)/double(sH-2.*mPosY)*iH;
    }
    else
    {
     x=crop.x()+double(m.x()*devicePixelRatioF())/double(sW)*crop.width();
     y=iH-(crop.y()+crop.height())+double(m.y()*devicePixelRatioF())/double(sH)*crop.height();
    }
    tmp.setX(x);
    tmp.setY(y);
    return tmp;
}

bool Viewer::setCrop(int x0, int y0, int w, int h)
{
 if(x0==0 && y0==0 && w==0 && h==0)
 {
   cropping=0;
   selection.setRect(x0,y0,w,h);	
 }
 else
 {
   cropping=1;
   selection.setRect(x0,y0,w,h);
   cropNeedsUpdate=1;
 }
 return true;
}

bool Viewer::calculate_crop()
{
 if((float)selection.width()/(float)selection.height()>(float)sW/(float)sH)
 {
 int newH=selection.width()/(float)sW*(float)sH;
 int offset=(selection.height()-newH)/2;
 crop.setRect(selection.x(),mRenderQtImg.height()-((selection.y()+offset)+newH),selection.width(),newH);
 }
 else
 {
 int newW=selection.height()/(float)sH*(float)sW;
 int offset=(selection.width()-newW)/2;
 crop.setRect(selection.x()+offset,mRenderQtImg.height()-(selection.y()+selection.height()),newW, selection.height());
 }
 
 if(crop.width()==0 || crop.height()==0)
 {
   crop.setRect(0,0,mRenderQtImg.width(),mRenderQtImg.height());
   cropping=0;
 }
 cropNeedsUpdate=0;
 mSceneChanged = true;
 updateScene();
 return true;
}

bool Viewer::moveCrop(int x0, int y0)
{
 selection.translate(x0,y0);
 cropNeedsUpdate=1;
 mSceneChanged = true;
 updateScene();
 return true;
}

bool Viewer::setSelection(double p1x, double p1y, double p2x, double p2y)
{
s1x=p1x;
s1y=p1y;
s2x=p2x;
s2y=p2y;
drawSelection=true;
mSceneChanged = true;
updateScene();
return true;
}

bool Viewer::setLine(double p1x, double p1y, double p2x, double p2y)
{
s1x=p1x;
s1y=p1y;
s2x=p2x;
s2y=p2y;
drawLine=true;
mSceneChanged = true;
updateScene();
return true;
}

bool Viewer::addToPolygon(double p1x, double p1y)
{
QPointF p(p1x,p1y);
polygon.push_back(p);
if(polygon.size()>1) {
 drawPolygon=true;
 mSceneChanged = true;
 updateScene();
}
return true;
}

bool Viewer::clearSelection()
{
s1x=0;
s1y=0;
s2x=0;
s2y=0;
drawSelection=false;
mSceneChanged = true;
updateScene();
return true;
}

bool Viewer::clearLine()
{
s1x=0;
s1y=0;
s2x=0;
s2y=0;
drawLine=false;
mSceneChanged = true;
updateScene();
return true;
}

bool Viewer::clearPolygon()
{
polygon.clear();
drawPolygon = false;
mSceneChanged = true;
updateScene();
return true;
}

void Viewer::setMinTrackL(int i)
{
    minTrackL=i;
}

void Viewer::setPoints(std::vector<cv::Point2f> cvpoints, std::vector<double> pm /*=std::vector<double>() */,std::vector<std::vector<cv::Point> > contours/*=std::vector<std::vector<cv::Point> >()*/)
{
    points.clear();
    for(unsigned long i=0; i<cvpoints.size(); i++)
    {
        QPointF p(cvpoints[i].x,cvpoints[i].y);
        points.push_back(p);
    }
    if(pm.size()>0 && pm.size()==cvpoints.size())
    {
     pointMeta.clear();
     for(int i=0; i<pm.size(); i++)
      pointMeta.push_back(pm[i]);
     havePointMeta=true;
    }
    else
    {
     pointMeta.clear();
     havePointMeta=false;
    }
    drawPolygons=false;
    std::vector<std::vector<QPointF> >().swap(polygons);
    std::vector<std::vector<double> >().swap(polygonsColorAndLineSize);
    if(contours.size()>0)
    {
     std::vector<std::vector<QPointF> >().swap(contoursForPoints);
     drawContours=true;
     for(unsigned long i=0; i<contours.size(); i++)
      if(contours[i].size()>1)
      {
       std::vector<QPointF> tmp;
       for(unsigned long j=0; j<contours[i].size(); j++)
       {
        QPointF p1(contours[i][j].x,contours[i][j].y); 
        tmp.push_back(p1);
       }
      contoursForPoints.push_back(tmp);
     }
    }
    else
    {
     drawContours=false;
     std::vector<std::vector<QPointF> >().swap(contoursForPoints);
    }
    mSceneChanged = true;
    updateScene();
}

void Viewer::setTracks(std::vector<std::vector<cv::Point2f> > cvtracks, std::vector<double> tm /*=std::vector<double>() */)
{	
    tracks.clear();
    for(unsigned long lineno=0; lineno<cvtracks.size(); lineno++)
    {
        std::vector<QPointF> tmp;
        for(unsigned long i=0; i<cvtracks[lineno].size(); i++)
        {
            QPointF p(cvtracks[lineno][i].x,cvtracks[lineno][i].y);
            tmp.push_back(p);
        }
        tracks.push_back(tmp);
    }
    if(tm.size()>0 && tm.size()==cvtracks.size())
    {
     trackMeta.clear();
     for(int i=0; i<tm.size(); i++)
      trackMeta.push_back(tm[i]);
     haveTrackMeta=true;
    }
    else
    {
     trackMeta.clear();
     haveTrackMeta=false;
    }
    mSceneChanged = true;
    updateScene();
}

bool Viewer::calcColor(double c, double& r,double& g,double& b)
{
 if(c<0 || c>1)
 {
  r=0;
  b=0;
  g=0;
  return false;
 }
 else
 {
 double cx=c*8.;
 if(cx<1)
 {
     r=0;
     g=0;
     b=(128+cx*128)/255.;
 }
 else if(cx<3)
 {
     r=0;
     g=(256*(cx-1.)/2.)/255.;
     b=1;
 }
 else if(cx<5)
 {
     r=(256*(cx-3.)/2.)/255.;
     g=1;
     b=(255-256*(cx-3.)/2.)/255.;
 }
 else if(cx<7)
 {
     r=1;
     g=(255-256*(cx-5.)/2.)/255.;
     b=0;
 }
 else
 {
     r=(255-256*(cx-7.)/2.)/255.;
     g=0;
     b=0;
 }
 }
 return true;
}

bool Viewer::setHighlight(double p1x, double p1y)
{
    highlight=true;
    highlightX=p1x;
    highlightY=p1y;
    mSceneChanged = true;    
    updateScene();
    return true;
}

bool Viewer::setTrackToHighlight(std::vector<cv::Point2f> cvtrack)
{
  trackToHighlight.clear();
  highlightTrack=false;
  if(cvtrack.size()>1)
  {
    highlightTrack=true;
    for(unsigned long i=0; i<cvtrack.size(); i++)
    {
        QPointF p(cvtrack[i].x-5.0,cvtrack[i].y-5.0);
        trackToHighlight.push_back(p);
    }
    for(unsigned long i=cvtrack.size()-1; i>0; i--)
    {
        QPointF p(cvtrack[i].x+5.0,cvtrack[i].y+5.0);
        trackToHighlight.push_back(p);
    }
    for(unsigned long i=0; i<cvtrack.size(); i++)
    {
        QPointF p(cvtrack[i].x,cvtrack[i].y);
        trackToHighlight.push_back(p);
    }
    mSceneChanged = true;    
    updateScene();
    return true;
   }
   else
   {
    mSceneChanged = true;    
    updateScene();
    return false;
   }
}

bool Viewer::setROI(int p1x, int p1y, int p2x, int p2y)
{
    roi=true;
    roi_x0=p1x;
    roi_y0=p1y;
    roi_x1=p2x;
    roi_y1=p2y;
    std::cerr<<"roi set.\n";
    std::cerr<<"("<<roi_x0<<"|"<<roi_y0<<") to ("<<roi_x1<<"|"<<roi_y1<<")\n";
    mSceneChanged = true;    
    updateScene();
    return true;
}

bool Viewer::clearROI()
{
    roi=false;
    roi_x0=0;
    roi_y0=0;
    roi_x1=0;
    roi_y1=0;
    std::cerr<<"roi cleared.\n";
    mSceneChanged = true;    
    updateScene();
    return true;
}

int Viewer::addToPolygons(std::vector<std::vector<double> > inPoly,int r/*=255*/, int g/*=255*/, int b/*=255*/, double lineWidth/*=1*/)
{
  int ret=-1;
  std::vector< QPointF> thisPoly;
  for(unsigned int i=0; i<inPoly.size();i++)
  {
    if(inPoly[i].size()==2)
    {
     QPointF p(inPoly[i][0],inPoly[i][1]);
     thisPoly.push_back(p);
    }
  }
  if(thisPoly.size()>0)
  {
   ret=polygons.size();
   polygons.push_back(thisPoly);
   std::vector<double> cal;
   cal.push_back(double(r)/255.);
   cal.push_back(double(g)/255.);
   cal.push_back(double(b)/255.);
   cal.push_back((double)lineWidth);
   polygonsColorAndLineSize.push_back(cal);
   
   if(polygons.size()>0 && drawPolygons==false)
   {
   drawPolygons=true;
   }
   mSceneChanged = true;
   updateScene();
  }
  return ret;
}

void Viewer::removePolygons()
{
   drawPolygons=false;
   std::vector<std::vector<QPointF> >().swap(polygons);
   std::vector<std::vector<double> >().swap(polygonsColorAndLineSize);  
   mSceneChanged = true;
   updateScene();
}

bool Viewer::clearTrackToHighlight()
{
  trackToHighlight.clear();
  highlightTrack=false;
  return true;
}

void Viewer::zoom(double v, QPointF o)
{
   if(iW>0 && iH>0)
   {
   double iw=selection.width();
   double ih=selection.height();
   double ix0=selection.left();
   double iy0=selection.top();
   if(cropping==0)
   {
    iw=iW; ih=iH; ix0=0; iy0=0;
   }
   double newW=iw+v*iw;
   double newH=ih+v*ih;
   double newx0=ix0+iw/2.-newW/2.;
   double newy0=iy0+ih/2.-newH/2.;
   double clickx=(o.x()-ix0)/iw;
   double clicky=(o.y()-iy0)/ih;
   double newClickx=(o.x()-newx0)/newW;
   double newClicky=(o.y()-newy0)/newH;
   double xshift= (newClickx-clickx)*newW;
   double yshift= (newClicky-clicky)*newH;
   if(newW>1 && newH>1 && newW<2*iW && newH<2*iH)
   {
   selection.setRect(newx0+xshift,newy0+yshift,newW,newH);
   cropping=1;
   cropNeedsUpdate=1;
   renderImage();  
   }
   else
   {
    std::cerr<<"Maximum zoom reached...\n";
   }
   }
   else
   {
    std::cerr<<"I think I don't have an image yet...\n";   
   }
}

void Viewer::rescaleLevels()
{
 if(oldMinG!=dSettings.minG || oldMaxG!=dSettings.maxG)
 {
 if(!(dSettings.minG==0 && dSettings.maxG==255) ) 
 {
    mOrigImageBackup.copyTo(mOrigImage);
    cv::subtract(mOrigImage, dSettings.minG, mOrigImage);
    double scale=255./double(dSettings.maxG-dSettings.minG);
    oldMinG=dSettings.minG;
    oldMaxG=dSettings.maxG;
    mOrigImage=scale*mOrigImage;
    cv::flip(mOrigImage,mOrigImage,0);
    if( mOrigImage.channels() == 3)
       mRenderQtImg = QImage((const unsigned char*)(mOrigImage.data),
                             mOrigImage.cols, mOrigImage.rows,
                             mOrigImage.step, QImage::Format_RGB888);
    else if( mOrigImage.channels() == 1)
       mRenderQtImg = QImage((const unsigned char*)(mOrigImage.data),
                             mOrigImage.cols, mOrigImage.rows,
                             mOrigImage.step, QImage::Format_Indexed8);

    mSceneChanged = true;
    updateScene();
 }
 }
}
