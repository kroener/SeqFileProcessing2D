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
#ifndef VIDEOPLAYERWIDGET_H
#define VIDEOPLAYERWIDGET_H

#include <QWidget>
#include <QTimerEvent>
#include "iSeq.hpp"
#include "MosquitoesInSeq.hpp"
#include "MosquitoRegistry.hpp"
#include "Viewer.h"
#include <QGridLayout>
#include <QMouseEvent>
class Viewer;
class QPushButton;
class QLabel;
class QSlider;
class QSpinBox;
class QTimerEvent;
/**
  * \brief Display Video Player Widget with control elements
  */
class VideoPlayerWidget : public QWidget
{

    Q_OBJECT

public:
    VideoPlayerWidget();
    unsigned long getCurrentFrameNo();
    void setCurrentFrameNo(unsigned long i);
    void savePGM(string filename);
    void setHavePositions();
    void setHaveNoPositions();
    void setHaveTracks();
    void setHaveNoTracks();
    Viewer::DisplaySettings getDisplaySettings();
    bool setDisplaySettings(Viewer::DisplaySettings set);
    iSeq* getSeqPointer(int c, int r);
    MosquitoesInSeq* getMosqPointer(int c, int r);
    MosquitoRegistry* getTrackPointer(int c, int r);
    int metaMax();
    int metaMin();
    bool haveImage(int t);
    bool useMetaMode();
    void removeMosquito(QPoint m, int c, int r, unsigned long nn=0, unsigned long mm=0, double searchRadius=8, bool onlyOne = false);
    void addMosquito(QPoint m, int c, int r);
    void setCursorForWidgets(int n);
    QPointF imageCoordinates(QPoint m, int c, int r);
    void drawAllPoints(int c, int r, int i, int j, bool random = false, bool colorByFlag = false);
    void drawSelectedTracks(int c, int r, vector<int> TrackIDs, bool random = false , bool colorByFlag = false);
    void clearAllPoints(int c, int r);
    int getColumns();
    int getRows();
    bool clearPolygons(int c=0, int r=0);
    void clearTrackToHighlight(int c=0, int r=0);
    bool findCurrSeq(iSeq* currSeq,int &c, int &r);
    void setFromFrame(unsigned long n);
    void setToFrame(unsigned long n);
signals:
    void set_from_value(int);
    void set_to_value(int);
    void removePoint(double,double,int,int);    
    void addPoint(double,double);
    void contextMenu(QPoint,int,int);
    void mouseClick(QPoint, int, int);
    void mouseMove(QPoint, int, int, int, int, int, int);
    void mouseRelease(QPoint, int, int, int, int);
    void EmitKeyPressEvent(QKeyEvent *event);  
public slots:
    void on_FirstButton_clicked();
    void on_LastButton_clicked();
    void on_NextButton_clicked();
    void on_PrevButton_clicked();
    void on_PlayButton_clicked();
    void on_LoopButton_clicked();
    void on_StopButton_clicked();
    void on_fromButton_clicked();
    void on_toButton_clicked();
    void set_tdisplay(int i);
    void set_trackDisplayLength(int i);
    void set_MinTrackDisplayLength(int i);
    void set_msdisplay(short ms);
    void set_usdisplay(short us);
    void update_time();
    void set_mdisplay(int mosqs);
    void set_mddisplay(int num);
    void hide_mddisplay();
    void if_pslider_changed(int value);
    void if_fselect_changed(int value);
    void if_fskipselect_changed(int value);
    void updateFrame(unsigned long value);
    void ForceUpdateFrame(unsigned long value, bool forceImageReload=false);
    void LoadSeq(iSeq *iseq);
    void reLoadSeq(iSeq *iseq);
    void reLoadSeq(iSeq *iseq,int c, int r);
    void LoadMosqPos(MosquitoesInSeq *MosqPos);
    void LoadMosqPos(MosquitoesInSeq *MosqPos,int c, int r);
    void LoadReg(MosquitoRegistry *Reg);
    void LoadReg(MosquitoRegistry *Reg,int c, int r);
    void redraw();
    void showImageWithPoints(unsigned long i,bool random = false, bool colorByFlag = false);
    void showImage(cv::Mat A, unsigned long i, int c, int r);
    void removePointClicked(double x, double y);
    void addPointClicked(double x, double y);
    void setColumns(int x);
    void setRows(int y);
    void activateMetaMode(bool b);
    void setZoom(QPoint m, int c, int r, int dx, int dy);
    void setZoom(int c, int r, int ix, int iy, int iw, int ih);
    void setROI(QPoint m, int c, int r, int dx, int dy);
    void setROI(cv::Rect iRoi, int c, int r);
    void clearROI(int c, int r);
    void moveZoom(QPoint m, int c, int r, int dx, int dy);
    void setSelection(QPoint m, int c, int r, int dx, int dy);
    void drawLine(QPoint m, int c, int r, int dx, int dy);
    void clearPolygon(int c, int r);
    void addToPolygon(QPoint m, int c, int r);
    void addToPolygon(double x, double y, int c, int r);
    bool setHighlight(double px, double py);
    bool setHighlight(double px, double py, int c, int r);
    bool setTrackToHighlight(vector<cv::Point2f> cvtrack);
    bool setTrackToHighlight(vector<cv::Point2f> cvtrack, int c, int r);

protected:
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* event) override;  
    void keyPressEvent(QKeyEvent *event);

private:
    int switchMeta();
    void showFrame(int c, int r, bool random = false, bool colorByFlag = false);
    int getPreviousTrigger();
    int getNextTrigger();
    QPushButton *firstButton, *lastButton, *prevButton, *nextButton, *playButton, *loopButton, *stopButton;
    QSlider *pslider;
    QSpinBox *fselect;
    QSpinBox *fskipselect;
    QLabel *tdisplay,*mdisplay, *msdisplay, *usdisplay;
    QLabel *sm_d,*mu_d;
    QLabel *dmdisplay;
    QPushButton *fromButton, *toButton;
    QLabel *dmddisplay;
    QLabel *mddisplay;
    Viewer *OpenCVViewer[4][4];
    QGridLayout *ocvv;
    iSeq *seq[4][4];
    MosquitoesInSeq *MosqInSeq[4][4];
    MosquitoRegistry *currReg[4][4];
    bool have_positions[4][4];
    bool have_tracks[4][4];
    int trackL;
    int minTrackL;
    bool isOpen[4][4];
    unsigned long frame[4][4];
    int timerID;
    int playing;
    unsigned long pskip;
    unsigned long originalEachN;
    int TSec;
    short TMSec;
    short TUSec;
    int vh;
    int vw;
    int syncByMeta;
    int forceLoad;
    bool metaMode;
    int trigger;
    QPoint m;
    QPoint mm;
    bool loopMode;
    unsigned long fromFrame;
    unsigned long toFrame;
};
#endif // VIDEOPLAYERWIDGET_H 
