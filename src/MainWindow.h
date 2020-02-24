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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "SegmentationWorker.hpp"
#include <QMainWindow>
#include <QThread>
#include <QProgressDialog>
#include <QTableWidget>
#include <QHeaderView>
#include <QSettings>
#include <vector>
#include <fstream>
#include "VideoPlayerWidget.h"
#include "SeqInfoWidget.h"
#include "HistogramWidget.h"
#include "GraphWidget.h"
#include "FrameSelectWidget.h"
#include "SegParameterWidget.h"
#include "TrackingParameterWidget.h"
#include "Viewer.h"
#include "ViewerWithMouse.h"
#include "DisplayAllPointsWidget.h"
#include "MosquitoesInFrame.hpp"
#include "MosquitoesInSeq.hpp"
#include "MosquitoPosition.hpp"
#include "iSeq.hpp"
#include "DisplaySettingsParameterWidget.h"
#include "ImageComposer.hpp"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include "MosquitoRegistry.hpp"
#include "TrackPoint.hpp"
#include "AnimationParameterWidget.h"
#include "CodecParameterWidget.h"
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

class VideoPlayerWidget;
class DisplayAllPointsWidget;
class HistogramWidget;
class GraphWidget;
class QMenuBar;
class QLabel;
class QTableWidget;
class QHeaderView;
class MosquitoesInFrame;
class MosquitoesInSeq;
class MosquitoPosition;
class Viewer;
class QFileDialog;

/**
  * \brief Main Window Class
  */
class MainWindow : public QMainWindow
{

    Q_OBJECT

public:
    //MainWindow
    MainWindow();
    ~MainWindow();
    bool setDebug(const QString &fileName);
    //MainWindow_actions
    bool switchSeq(int CSeq);
    //MainWidow_context
    //MainWindow_functions
    vector<cv::Point> polyBB(vector<cv::Point> ppp, cv::Point2f center, double scale = 1.0);
    vector<vector<double> > segmentROI(iSeq* seqPointer,cv::Rect roi, unsigned long workF, double &maxdiff);
    vector<QPointF> toVectorOfQPointF(vector<vector<double> > in);
    //MainWindow_io
    bool loadFile(const QString &fileName);
    bool loadProject(const QString &fileName, bool append=false);
    bool loadPositionsFile(const QString &fileName, bool dynamic = false);
    bool loadPositionsFileRange(const QString &fileName, bool dynamic = false);
    bool savePositionsFile(const QString &fileName, bool selection = false);
    bool loadTracksFile(const QString &fileName);
    bool saveTracksFile(const QString &fileName);
    bool savePGMFile(const QString &fileName);
    bool savePGMFileWithMetadata(const QString &fileName);
    bool savePGMSelectionFile(const QString &fileName);
    bool saveHistogramToFile(const QString &fileName);
    bool savePolyToFile(const QString &fileName, vector<cv::Point> poly);
    vector<cv::Point> loadPolyFromFile(const QString &fileName);
    void customSettingsFromFile(QString f);
    //Mainwindow_ui
    void setNative(bool b);
        
    /**
      * \brief Storage for Settings
      */
    struct Settings
    {
    double minA;
    double maxA;
    double fracN;
    double minThreshold;
    int useIth;
    unsigned long fromFrame;
    unsigned long toFrame;
    unsigned long FrameStep;
    unsigned long currFrame;
    int erode;
    int dilute;
    double cleanCluster;
    bool blackOnWhite;
    int displayEachN;
    };
    typedef struct Settings Settings;
    enum MouseModeType {
     NONE,
     ZOOM,
     PAN,
     ADDPOINT,
     REMOVEPOINT,
     SELECTOFFSET,
     POLYGONSELECTION,
     SELECTROI,
     SELECTMOSQUITO,
     REMOVEPOINTFROMTRACK,
     REMOVETRACK,
     ADDPOINTTOTRACK,
     JOINTRACKS,
     SPLITTRACK,
     POLYGONSELECTIONFORTRACKS,     
     INVERSEPOLYGONSELECTION,
     EDITPOLYGON,
     SELECTPOLYROI,
     SELECTTRACKTODISPLAY,
     SELECTTRACKTOESTIMATE,
     SELECTTRACKFROMESTIMATE,
     SELECTPOINTFORLOCALSEARCH,
     SELECTTRACKFORLOCALSEARCH,
     SELECTTRACKFORLOCALBACKWARDSSEARCH,
     SELECTTRACKFORLOCALINBETWEENSEARCH,
     DISPLAYPIXELVALUE,
     POLYGONSELECTIONTOTRACK
    };
    
private:
    //MainWindow_ui
    void createDockWindows();
    void createMenus();
    void createToolBar();
    //MainWindow_io
    void loadSettings(unsigned long i);
    void saveSettings(unsigned long i);
    void loadSettingsFromFile(string inputFileName);
    void saveSettingsToFile(string outputFileName);    
    void saveSegSettingsToAll();
    void saveSegSettings(unsigned long i);
    bool saveProject(const QString &fileName);
    //MainWindow_actions
    void processSeqI(unsigned long i);
    void on_actionCleanEdges_triggered(unsigned long wframe);
    void updateTableOfTracks();
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    bool save_animation(const QString &fileName);
    //MainWindow_functions
    bool backupPos(unsigned int n, bool force = false);
    bool backupTracks(unsigned int n);
    bool restorePos(unsigned int n);
    bool restoreTracks(unsigned int n);
    int seqPointerToSeqNum(iSeq* sp);
    void updatePointerOfTracks(int c, int r);
    QString strippedName(const QString &fullFileName);

    //Variables
    VideoPlayerWidget *m_player;
    SeqInfoWidget *seqInfo;
    FrameSelectWidget *frameSelect;
    SegParameterWidget *segParam;
    TrackingParameterWidget *trackParam;
    AnimationParameterWidget *aniParam;
    CodecParameterWidget *codecParam;
    DisplaySettingsParameterWidget *displayParam;
    HistogramWidget *Histogram;
    GraphWidget *Graph;
    ViewerWithMouse *ImgViewer;
    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;
    QMenu *debugMenu;
    QMenu *editMenu;
    QAction *openAct;
    QAction *loadPositionsAct;
    QAction *loadPositionsRangeAct;
    QAction *loadDynamicPositionsAct;
    QAction *savePositionsAct;
    QAction *loadProjectAct;
    QAction *saveProjectAct;
    QAction *savePositions4Act;
    QAction *loadTracksAct;
    QAction *saveTracksAct;
    QAction *closeAct;
    QDockWidget *HistoDock;
    QDockWidget *GraphDock;
    QDockWidget *ImgViewerDock;
    QAction *accept;
    std::vector<iSeq> seq;
    std::vector<Settings> settings;
    Settings defSettings;
    int currSeqNum;
    int blackOnWhite;
    iSeq* currSeq;
    bool isOpen;
    double minA;
    double maxA;
    double fracN;
    double minThreshold;
    int useIth;
    int erode;
    int dilute;
    double cleanCluster;
    int fc;
    int displayEachN;
    double TrackMinA;
    double TrackMaxA;
    double TrackMinD;
    double TrackMaxD;
    int TrackMaxG;
    unsigned int TrackMaxN;
    int TrackMaxCL;
    unsigned int TrackMinTD;
    int appendTracks;
    QThread* thread;
    SegmentationWorker* worker;
    int ith_changed;
    std::vector<MosquitoesInSeq> MosqPos;
    std::vector<std::vector<MosquitoesInSeq> > MosqPosHist;
    int lastMosqPosHist;
    std::vector<MosquitoRegistry> Reg;
    std::vector<std::vector<MosquitoRegistry> > RegHist;
    int lastRegHist;
    unsigned long maxHistSize;
    MosquitoesInSeq *currMosqPos;
    MosquitoRegistry *currReg;
    unsigned long workFrame;
    unsigned long fromFrame;
    unsigned long toFrame;
    unsigned long FrameStep;
    int timerID2;
    QProgressDialog *DisplayProgress;
    bool seqFileChanged;
    int process_all;
    QTableWidget* m_pTableWidget;
    QStringList m_TableHeader;
    QTableWidget* m_tTableWidget;
    QStringList m_tTableHeader;
    QTableWidget* m_eTableWidget;
    QStringList m_eTableHeader;
    DisplayAllPointsWidget* displayAllPointsWidget;  
    int debug;
    ofstream debugFile;
    int MaxRecentFiles;
    vector<QAction *> recentFileActs;
    unsigned int currMeta;
    int syncOnMeta;
    double meta_t0;
    double meta_fps;    
    ImageComposer composer;
    MouseModeType mouseMode;
    bool nativeFD;    
    vector<QPointF> polygon;
    vector<QPointF> polyROI;
    bool showAllPoints;
    int selectedTrack;
    int selectedVert;
    int thingToAccept;
    int currentRow;
    int currentCol;
    double currentSlope;
    double currentIntersection;
    bool disablePosBackup;
    vector<cv::Point2f> tmpPointStorage;
    vector<vector<unsigned long> > tmpFrameNumberStorage;

signals:
    void readyToRunNext();
    
protected:
    //MainWindow_ui
    void timerEvent(QTimerEvent *event);
    void keyPressEvent(QKeyEvent *e);
    //MainWindow
    void closeEvent(QCloseEvent *event);
       
private slots:
    //MainWindow_actions
    void on_readyToRunNext_triggered();
    void on_actionOpen_triggered();
    void on_about_triggered();
    void on_actionClose_triggered();
    void on_actionOpenPositions_triggered();
    void on_actionOpenPositionsRange_triggered();
    void on_actionOpenDynamicPositions_triggered();
    void on_actionSavePositions_triggered();
    void on_actionSavePositionsSelection_triggered();
    void on_actionOpenProject_triggered();
    void on_actionSaveProject_triggered();
    void on_actionOpenTracks_triggered();
    void on_actionSaveTracks_triggered();
    void on_actionSaveHistOfImage_triggered();
    void on_actionFrom_triggered();
    void on_actionCopySettings_triggered();
    void on_actionProcessAll_triggered();
    void on_actionCleanCluster_triggered();
    void on_actionCleanAllCluster_triggered();
    void on_actionResetPositions_triggered();
    void on_actionResetTracks_triggered();
    void on_blackOnWhite_checked(int i);
    void on_displayShowPointsClicked(int i);
    void on_disablePosBackup(int i);
    void on_displayShowTracksClicked(int i); 
    void on_displayContoursChanged(int i);
    void on_displayPointSizeChanged(double v);
    void on_displayTrackWidthChanged(double v);
    void on_displaySyncOnMetaClicked(int i);
    void on_displaySelectOnMetaClicked(int i);
    void on_displayArrayWidthChanged(int i);
    void on_displayArrayHeightChanged(int i);
    void on_displayMinGChanged(int i);
    void on_displayMaxGChanged(int i);
    void on_varPointRadiusChanged(double v);
    void on_fromButton_clicked(int i);
    void on_toButton_clicked(int i);
    void on_selectCurrent(int i);
    void on_actionShowDiffImage_triggered();
    void on_actionShowTable_triggered();
    void on_actionShowTableOfAll_triggered();
    void on_actionShowTableOfTracks_triggered();
    void on_actionShowHist_triggered();
    void on_actionShowHistOfImage_triggered();
    void on_actionShowPoints_triggered();
    void on_actionTo_triggered();
    void on_actionSavePGM_triggered();
    void on_actionSavePGMMetadata_triggered();
    void on_actionSavePGMSelection_triggered();
    void on_testParams_clicked();
    void on_startParams_clicked();
    void on_TrackStartParams_clicked();
    void update_minA(double v);
    void update_maxA(double v);
    void update_fracN(double v);
    void update_minThreshold(double v);
    void update_useIth(int i);
    void update_erode(int i);
    void update_dilute(int i);
    void update_cleanCluster(double v);
    void update_displayEachNFrames(int i);
    void update_TrackMinA(double v);
    void update_TrackMaxA(double v);
    void update_TrackMinD(double v);
    void update_TrackMaxD(double v);
    void update_TrackMaxG(int i);
    void update_TrackMaxN(int i);
    void update_TrackMaxCL(int i);
    void update_TrackMinTD(int i);
    void update_TrackAppendOrOverwrite(int i);
    void update_fromFrame(int i);
    void update_toFrame(int i);
    void update_FrameStep(int i);
    void removePointClicked(double x, double y, int c, int r);
    void addPointClicked(double x, double y);
    void on_pTable_ContextMenuRequest(const QPoint & P);
    void on_tTable_ContextMenuRequest(const QPoint & P);
    void on_mouseMove(QPoint m, int c, int r,int dx, int dy, int ddx, int ddy);
    void on_mouseRelease(QPoint m, int c, int r,int dx, int dy);
    void on_mouseClicked(QPoint m, int c, int r);
    void on_m_player_keyPressEvent(QKeyEvent* key);
    void on_CreateAnimationParams_clicked();
    void clearCustomSettings();
    void pTableClicked(int row,int col);
    void tTableClicked(int row,int col);
    void tTableSelectionChanged();
    void tTableCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void setDefaultSegmentationParameter();
    void displaySeqIn(int i, int c, int r);
    //MainWindow_ui
    void displayTable();
    void initTable(int n);
    void fillTable(unsigned long n);
    void displayTableOfAll();
    void initTableOfAll(int n);
    void fillTableOfAll();
    void displayTableOfTracks();
    void initTableOfTracks(int n);
    void fillTableOfTracks();
    //MainWindow_context
    void showContextMenuClicked(QPoint m, int c, int r);
    //MainWindow_io
    void save_results(vector<vector<double> > centers, double maxDiff);
    void saveSettingsAsDefault();
    void saveCustomGeometrySettingsToFile(QString f);
    void loadCustomSettings();
    void loadCustomSettingsFromFile(QString f);
    void saveCustomSettings();
    void saveCustomSettingsToFile(QString f);
    void loadGeometryFromFile(QString f);
    void customSettingsFromFile();
    void customSettingsToFile();
    void openRecentFile();
};
#endif //MAINWINDOW_H
