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
#ifndef SEGPARAMETERWIDGET_H
#define SEGPARAMETERWIDGET_H

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
  * \brief Widget to select Segmentation parameter
  */
class SegParameterWidget : public QWidget
{

    Q_OBJECT

public:
    SegParameterWidget();
    void setMinA(double v);
    void setMaxA(double v);
    void setFracN(double v);
    void setMThreshold(double v);
    void setUseIth(int i);
    void setErode(int i);
    void setDilute(int i);
    void setCleanCluster(double v);
    void setDisplayEachNFrames(int i);
    void setBlackOnWhite(bool b);
    void setMaskThreshold(int i);
    void setMedianBlur1Param(int i);
    void setMedianBlur2Param(int i);
    void setGaussianBlur1Param(int i);
    void setGaussianBlur2Param(int i);
    void setGaussianSigma1Param(double v);
    void setGaussianSigma2Param(double v);
    
signals:
    void minAreaValueChanged(double);
    void maxAreaValueChanged(double);
    void thresholdValueChanged(double);
    void minThresholdValueChanged(double);
    void useIthValueChanged(int);
    void erodeValueChanged(int);
    void cleanClusterValueChanged(double);
    void displayEachNFramesValueChanged(int);
    void checkBlackOnWhiteValueChanged(int);
    void diluteValueChanged(int);
    void onTestParamsClicked();
    void onStartParamsClicked();

public slots:
    void setMinThreshold(double mt);
    void minAChanged(double v);
    void maxAChanged(double v);
    void thChanged(double v);
    void mThChanged(double v);
    void ithChanged(int i);
    void erodeChanged(int i);
    void diluteChanged(int i);
    void cleanClusterChanged(double v);
    void displayEachNFramesChanged(int i);
    void checkBlackOnWhiteChanged(int i);
    void testParamsClicked();
    void startParamsClicked();
    double getMinA();
    double getMaxA();
    double getThreshold();
    double getMinThreshold();
    int getIth();
    int getErode();
    int getDilute();
    double getCleanCluster();
    int getDiplayEachNFrames();
    bool getBlackOnWhite();
    int getMedianBlur1Param();
    int getMedianBlur2Param();
    int getGaussianBlur1Param();
    int getGaussianBlur2Param();
    double getGaussianSigma1Param();
    double getGaussianSigma2Param();
    int getMaskThreshold();
    void checkG1(int i);
    void checkG2(int i);
    void checkM1(int i);
    void checkM2(int i);

private:
    QDoubleSpinBox *minAreaParam;
    QDoubleSpinBox *maxAreaParam;
    QDoubleSpinBox *thresholdParam;
    QDoubleSpinBox *minThresholdParam;
    QSpinBox *useIthParam;
    QPushButton *testParams;
    QPushButton *startParams;
    QSpinBox *erodeParam;
    QSpinBox *diluteParam;
    QDoubleSpinBox *cleanClusterParam;
    QSpinBox *displayEachNFramesParam;
    QCheckBox *checkBlackOnWhiteParam;
    QSpinBox *medianBlur1Param;
    QSpinBox *medianBlur2Param;
    QSpinBox *gaussianBlur1Param;
    QDoubleSpinBox *gaussianSigma1Param;
    QSpinBox *gaussianBlur2Param;
    QDoubleSpinBox *gaussianSigma2Param;
    QSpinBox *maskThresholdParam;
};
#endif // SEGPARAMETERWIDGET_H  
