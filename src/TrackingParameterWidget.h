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
#ifndef TRACKINGPARAMETERWIDGET_H
#define TRACKINGPARAMETERWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
class QLabel;
class QPushButton;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
/** 
  * \brief Widget to select Tracking parameter
  */
class TrackingParameterWidget : public QWidget
{

    Q_OBJECT

public:
    TrackingParameterWidget();
    void setMinA(double v);
    void setMaxA(double v);
    void setMinD(double v);
    void setMaxD(double v);
    void setMaxG(int i);
    void setMaxN(int i);
    void setMaxCL(int i);
    void setMinTD(int i);
    void setTrackDisplayLength(int i);
    void setMinTrackDisplayLength(int i);
    void setAppendOrOverwrite(int i);

signals:
    void minAValueChanged(double);
    void maxAValueChanged(double);
    void minDValueChanged(double);
    void maxDValueChanged(double);
    void maxDAutoJoinValueChanged(double);
    void maxGValueChanged(int);
    void maxGAutoJoinValueChanged(int);
    void maxNValueChanged(int);
    void maxCLValueChanged(int);
    void minTDValueChanged(int);
    void onStartParamsClicked();
    void minTrackDisplayLengthValueChanged(int);
    void trackDisplayLengthValueChanged(int);
    void appendOrOverwriteValueChanged(int);
    

public slots:
    void minDChanged(double v);
    void maxGChanged(int i);
    void maxGAutoJoinChanged(int i);
    void maxDChanged(double v);
    void maxDAutoJoinChanged(double v);
    void minAChanged(double v);
    void maxAChanged(double v);
    void maxNChanged(int i);
    void maxCLChanged(int i);
    void minTDChanged(int i);
    void minTrackDisplayLengthChanged(int i);
    void trackDisplayLengthChanged(int i);
    void appendOrOverwriteChanged(int i);

    void startParamsClicked();
    double getMinA();
    double getMaxA();
    double getMinD();
    double getMaxD();
    double getMaxDAutoJoin();
    int getMaxG();
    int getMaxGAutoJoin();
    int getMaxN();
    int getMaxCL();
    int getMinTD();
    int getMinTrackDisplayLength();
    int getTrackDisplayLength();
    bool getAppendOrOverwrite();
    
private:
    QDoubleSpinBox *minDistParam;
    QDoubleSpinBox *maxDistParam;
    QSpinBox *maxGapParam;
    QDoubleSpinBox *minAreaParam;
    QDoubleSpinBox *maxAreaParam;
    QSpinBox *maxNeighbourParam;
    QSpinBox *maxCoherenceLenghtParam;
    QSpinBox *minTrackDistParam;
    QSpinBox *trackDisplayLengthParam;
    QSpinBox *minTrackDisplayLengthParam;
    QCheckBox *appendOrOverwriteParam;
    QPushButton *startParams;
    QDoubleSpinBox *maxDistAutoJoinParam;
    QSpinBox *maxGapAutoJoinParam;
};
#endif // TRACKINGPARAMETERWIDGET_H  
