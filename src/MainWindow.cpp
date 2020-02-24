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
#include <functional>
#include "MainWindow.h"
#include "VideoPlayerWidget.h"
#include "SeqInfoWidget.h"
#include "TrackingParameterWidget.h"
#include "FrameSelectWidget.h"
#include "HistogramWidget.h"
#include "GraphWidget.h"
#include "Viewer.h"
#include "ViewerWithMouse.h"
#include "DisplayAllPointsWidget.h"
#include "SegmentationWorker.hpp"
#include "MosquitoesInFrame.hpp"
#include "MosquitoesInSeq.hpp"
#include "MosquitoPosition.hpp"
#include "iSeq.hpp"
#include "DisplaySettingsParameterWidget.h"
#include "ImageComposer.hpp"
#include <QApplication>
#include <QDialog>
#include <QInputDialog>
#include <QFileDialog>
#include <QMenuBar>
#include <QThread>
#include <QSettings>
#include <QDate>
#include <fstream>
#include <QTableWidget>
#include <QKeyEvent>
#include <QLabel>
#include <QDockWidget>
#include <QProgressDialog>
#include <QMessageBox>
#include <vector>
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
#include <unistd.h>
#include <time.h>
extern "C" {
#include <libavcodec/avcodec.h>
}
#include <time.h>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace cv;

MainWindow::MainWindow()
{
    QSettings qsettings;
    restoreGeometry(qsettings.value("mainWindowGeometry").toByteArray());
    MaxRecentFiles = 5;
    defSettings = { 8.0,600.,0.15,6.0,1, 0,0,0,0,5,5,0,1,1};
    displayAllPointsWidget = new DisplayAllPointsWidget();
    m_pTableWidget = NULL;
    m_tTableWidget = NULL;
    m_eTableWidget = NULL;
    seqFileChanged=false;
    blackOnWhite=1;
    ith_changed=1;
    debug=0;
    process_all=0;
    maxHistSize=2;
    lastMosqPosHist=-1;
    lastRegHist=-1;
    disablePosBackup=true;
    m_player=new VideoPlayerWidget();
    setCentralWidget( m_player );
    createMenus();
    createDockWindows();
    setWindowTitle( "SeqFileProcessing2D" );
    thread = new QThread;
    worker = new SegmentationWorker();
    worker->moveToThread(thread);
    thread->start();
    connect(segParam, SIGNAL(minAreaValueChanged(double)), this, SLOT(update_minA(double)));
    connect(segParam, SIGNAL(maxAreaValueChanged(double)), this, SLOT(update_maxA(double)));
    connect(segParam, SIGNAL(thresholdValueChanged(double)), this, SLOT(update_fracN(double)));
    connect(segParam, SIGNAL(minThresholdValueChanged(double)), this, SLOT(update_minThreshold(double)));
    connect(segParam, SIGNAL(useIthValueChanged(int)), this, SLOT(update_useIth(int)));
    connect(segParam, SIGNAL(diluteValueChanged(int)), this, SLOT(update_dilute(int)));
    connect(segParam, SIGNAL(erodeValueChanged(int)), this, SLOT(update_erode(int)));
    connect(segParam, SIGNAL(cleanClusterValueChanged(double)), this, SLOT(update_cleanCluster(double)));
    connect(segParam, SIGNAL(displayEachNFramesValueChanged(int)), this, SLOT(update_displayEachNFrames(int)));
    connect(segParam, SIGNAL(onTestParamsClicked()), this, SLOT(on_testParams_clicked()));
    connect(segParam, SIGNAL(onStartParamsClicked()), this, SLOT(on_startParams_clicked()));
    connect(segParam, SIGNAL(checkBlackOnWhiteValueChanged(int)), this, SLOT(on_blackOnWhite_checked(int)));

    connect(trackParam, SIGNAL(minAValueChanged(double)), this, SLOT(update_TrackMinA(double)));
    connect(trackParam, SIGNAL(maxAValueChanged(double)), this, SLOT(update_TrackMaxA(double)));
    connect(trackParam, SIGNAL(minDValueChanged(double)), this, SLOT(update_TrackMinD(double)));
    connect(trackParam, SIGNAL(maxDValueChanged(double)), this, SLOT(update_TrackMaxD(double)));
    connect(trackParam, SIGNAL(maxGValueChanged(int)), this, SLOT(update_TrackMaxG(int)));
    connect(trackParam, SIGNAL(maxNValueChanged(int)), this, SLOT(update_TrackMaxN(int)));
    connect(trackParam, SIGNAL(maxCLValueChanged(int)), this, SLOT(update_TrackMaxCL(int)));
    connect(trackParam, SIGNAL(minTDValueChanged(int)), this, SLOT(update_TrackMinTD(int)));
    connect(trackParam, SIGNAL(onStartParamsClicked()), this, SLOT(on_TrackStartParams_clicked()));
    connect(trackParam, SIGNAL(minTrackDisplayLengthValueChanged(int)), m_player, SLOT(set_MinTrackDisplayLength(int)));
    connect(trackParam, SIGNAL(trackDisplayLengthValueChanged(int)), m_player, SLOT(set_trackDisplayLength(int)));
    connect(trackParam, SIGNAL(appendOrOverwriteValueChanged(int)), this, SLOT(update_TrackAppendOrOverwrite(int)));

    connect(aniParam, SIGNAL(onCreateAnimationClicked()), this, SLOT(on_CreateAnimationParams_clicked()));

    minA=segParam->getMinA();
    maxA=segParam->getMaxA();
    fracN=segParam->getThreshold();
    minThreshold=segParam->getMinThreshold();
    useIth=segParam->getIth();
    erode=segParam->getErode();
    dilute=segParam->getDilute();
    cleanCluster=segParam->getCleanCluster();
    displayEachN=segParam->getDiplayEachNFrames();
    fc=0;

    TrackMinA=trackParam->getMinA();
    TrackMaxA=trackParam->getMaxA();
    TrackMinD=trackParam->getMinD();
    TrackMaxD=trackParam->getMaxD();
    TrackMaxG=trackParam->getMaxG();
    TrackMaxN=trackParam->getMaxN();
    TrackMaxCL=trackParam->getMaxCL();
    TrackMinTD=trackParam->getMinTD();
    appendTracks=trackParam->getAppendOrOverwrite();

    fracN=segParam->getThreshold();
    minThreshold=segParam->getMinThreshold();
    useIth=segParam->getIth();
    erode=segParam->getErode();
    dilute=segParam->getDilute();
    cleanCluster=segParam->getCleanCluster();
    displayEachN=segParam->getDiplayEachNFrames();

    connect(worker, SIGNAL(finished(vector<vector<double> >, double)), this, SLOT(save_results(vector<vector<double> >, double)));

    fromFrame=frameSelect->getFrom();
    toFrame=frameSelect->getTo();
    FrameStep=frameSelect->getStep();
    connect(frameSelect, SIGNAL(toValueChanged(int)),this, SLOT(update_toFrame(int)));
    connect(frameSelect, SIGNAL(fromValueChanged(int)),this, SLOT(update_fromFrame(int)));
    connect(frameSelect, SIGNAL(stepValueChanged(int)),this, SLOT(update_FrameStep(int)));
    connect(seqInfo, SIGNAL(seqFileSelected(int)), this, SLOT(on_selectCurrent(int)));
    connect(this, SIGNAL(readyToRunNext()), this, SLOT(on_readyToRunNext_triggered()));
    connect(m_player, SIGNAL(set_from_value(int)), this, SLOT(on_fromButton_clicked(int)));
    connect(m_player, SIGNAL(set_to_value(int)), this, SLOT(on_toButton_clicked(int)));
    connect(m_player, SIGNAL(addPoint(double,double)), this, SLOT(addPointClicked(double,double)));
    connect(m_player, SIGNAL(removePoint(double,double,int,int)), this, SLOT(removePointClicked(double,double,int,int)));
    connect(m_player, SIGNAL(contextMenu(QPoint,int,int)), this, SLOT(showContextMenuClicked(QPoint,int,int)));
    connect(m_player, SIGNAL(mouseClick(QPoint,int,int)), this, SLOT(on_mouseClicked(QPoint,int,int)));
    connect(m_player, SIGNAL(mouseMove(QPoint,int,int,int,int,int,int)), this, SLOT(on_mouseMove(QPoint,int,int,int,int,int,int)));
    connect(m_player, SIGNAL(mouseRelease(QPoint,int,int,int,int)), this, SLOT(on_mouseRelease(QPoint,int,int,int,int)));
    connect(m_player, SIGNAL(EmitKeyPressEvent(QKeyEvent*)), this, SLOT(on_m_player_keyPressEvent(QKeyEvent*)));
    connect(displayParam, SIGNAL(showPointsChanged(int)), this, SLOT(on_displayShowPointsClicked(int)));
    connect(displayParam, SIGNAL(showTracksChanged(int)), this, SLOT(on_displayShowTracksClicked(int)));
    connect(displayParam, SIGNAL(pointSizeChanged(double)), this, SLOT(on_displayPointSizeChanged(double)));
    connect(displayParam, SIGNAL(trackWidthChanged(double)), this, SLOT(on_displayTrackWidthChanged(double)));
    connect(displayParam, SIGNAL(syncOnMetaChanged(int)), this, SLOT(on_displaySyncOnMetaClicked(int)));
    connect(displayParam, SIGNAL(selectOnMetaChanged(int)), this, SLOT(on_displaySelectOnMetaClicked(int)));
    connect(displayParam, SIGNAL(ArrayWidthChanged(int)), this, SLOT(on_displayArrayWidthChanged(int)));
    connect(displayParam, SIGNAL(ArrayHeightChanged(int)), this, SLOT(on_displayArrayHeightChanged(int)));
    connect(displayParam, SIGNAL(displayContoursChanged(int)), this, SLOT(on_displayContoursChanged(int)));
    connect(displayParam, SIGNAL(minGChanged(int)), this, SLOT(on_displayMinGChanged(int)));
    connect(displayParam, SIGNAL(maxGChanged(int)), this, SLOT(on_displayMaxGChanged(int)));
    connect(displayParam, SIGNAL(disablePosBackupChanged(int)), this, SLOT(on_disablePosBackup(int)));
    connect(displayParam, SIGNAL(varPointRadiusChanged(double)), this, SLOT(on_varPointRadiusChanged(double)));

    restoreState(qsettings.value("mainWindowState").toByteArray());
    loadCustomSettings();
    setDefaultSegmentationParameter();
    updateRecentFileActions();
    currMeta=0;
    syncOnMeta=1;
    meta_fps=50;
    meta_t0=0;
    mouseMode=ZOOM;
    m_player->setCursorForWidgets(5);
    QPixmap icon = QPixmap(":SeqFileProcessing2D.png");
    QIcon windowIcon = QIcon(icon);
    setWindowIcon(windowIcon);
    nativeFD=false;
    currSeqNum=-1;
    currSeq=nullptr;
    timerID2=-1;    
    showAllPoints=false;
    selectedTrack=-1;
    selectedVert=-1;
    thingToAccept=0;
    currentRow=-1;
    currentCol=-1;
}

MainWindow::~MainWindow()
{
 saveCustomSettings();
}


bool MainWindow::setDebug(const QString &fileName)
{
    string fileN=fileName.toUtf8().constData();
    debugFile.open(fileN.c_str(),ios::binary);
    if(debugFile.is_open())
    {
        debug=1;
    }
    return true;
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    saveCustomSettings();
    event->accept();
}
