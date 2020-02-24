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
#include "SegParameterWidget.h"

#include <QLabel>
#include <QFormLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QScrollArea>

SegParameterWidget::SegParameterWidget()
    : minAreaParam( new QDoubleSpinBox ),
      maxAreaParam( new QDoubleSpinBox ),
      thresholdParam( new QDoubleSpinBox ),
      minThresholdParam( new QDoubleSpinBox ),
      useIthParam( new QSpinBox ),
      testParams( new QPushButton ("Test") ),
      startParams( new QPushButton ("Start") ),
      erodeParam( new QSpinBox ),
      diluteParam( new QSpinBox ),
      cleanClusterParam( new QDoubleSpinBox ),
      displayEachNFramesParam( new QSpinBox ),
      checkBlackOnWhiteParam( new QCheckBox ),
      medianBlur1Param( new QSpinBox ),
      medianBlur2Param( new QSpinBox ),
      gaussianBlur1Param( new QSpinBox ),
      gaussianSigma1Param( new QDoubleSpinBox ),
      gaussianBlur2Param( new QSpinBox ),
      gaussianSigma2Param( new QDoubleSpinBox ),
      maskThresholdParam( new QSpinBox )
{
    QWidget *scrollWidget = new QWidget();
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setGeometry(0,0,200,200);
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(tr("&MinArea:"), minAreaParam);
    formLayout->addRow(tr("&MaxArea:"), maxAreaParam);
    formLayout->addRow(tr("&Threshold:"), thresholdParam);
    formLayout->addRow(tr("&MinThreshold:"), minThresholdParam);
    formLayout->addRow(tr("&Use the ith:"), useIthParam);
    formLayout->addRow(tr("&Remove GB:"), erodeParam);
    formLayout->addRow(tr("&Add FG:"), diluteParam);
    formLayout->addRow(tr("&ClusterDist:"), cleanClusterParam);
    formLayout->addRow(tr("&Display each N:"), displayEachNFramesParam);
    formLayout->addRow(tr("&Black On White:"), checkBlackOnWhiteParam);
    formLayout->addRow(tr("medianBlur1"), medianBlur1Param);
    formLayout->addRow(tr("medianBlur2"), medianBlur2Param);
    formLayout->addRow(tr("gaussian kernel 1"), gaussianBlur1Param);
    formLayout->addRow(tr("gaussian sigma 1"), gaussianSigma1Param);
    formLayout->addRow(tr("gaussian kernel 2"), gaussianBlur2Param);
    formLayout->addRow(tr("gaussian sigma 2"), gaussianSigma2Param);
    formLayout->addRow(tr("Mask Threshold"), maskThresholdParam);
    scrollWidget->setLayout(formLayout);
    scrollArea->setWidget(scrollWidget);

    QHBoxLayout *hLayout=new QHBoxLayout();
    hLayout->addWidget(testParams);
    hLayout->addWidget(startParams);
    QVBoxLayout *vLayout=new QVBoxLayout();
    vLayout->addWidget(scrollArea);
    vLayout->addLayout(hLayout);
    setLayout(vLayout);
    connect(minAreaParam,SIGNAL(valueChanged(double)),this,SLOT(minAChanged(double)));
    connect(maxAreaParam,SIGNAL(valueChanged(double)),this,SLOT(maxAChanged(double)));
    connect(thresholdParam,SIGNAL(valueChanged(double)),this,SLOT(thChanged(double)));
    connect(minThresholdParam,SIGNAL(valueChanged(double)),this,SLOT(mThChanged(double)));
    connect(useIthParam,SIGNAL(valueChanged(int)),this,SLOT(ithChanged(int)));
    connect(erodeParam,SIGNAL(valueChanged(int)),this,SLOT(erodeChanged(int)));
    connect(diluteParam,SIGNAL(valueChanged(int)),this,SLOT(diluteChanged(int)));
    connect(testParams,SIGNAL(clicked()),this,SLOT(testParamsClicked()));
    connect(startParams,SIGNAL(clicked()),this,SLOT(startParamsClicked()));
    connect(cleanClusterParam,SIGNAL(valueChanged(double)),this,SLOT(cleanClusterChanged(double)));
    connect(displayEachNFramesParam,SIGNAL(valueChanged(int)),this,SLOT(displayEachNFramesChanged(int)));
    connect(checkBlackOnWhiteParam,SIGNAL(stateChanged(int)),this,SLOT(checkBlackOnWhiteChanged(int)));
    connect(gaussianBlur1Param,SIGNAL(valueChanged(int)),this,SLOT(checkG1(int)));
    connect(gaussianBlur2Param,SIGNAL(valueChanged(int)),this,SLOT(checkG2(int)));
    connect(medianBlur1Param,SIGNAL(valueChanged(int)),this,SLOT(checkM1(int)));
    connect(medianBlur2Param,SIGNAL(valueChanged(int)),this,SLOT(checkM2(int))); 
    minAreaParam->setMinimum(0);
    minAreaParam->setMaximum(4194304);
    minAreaParam->setValue(8);
    maxAreaParam->setMinimum(0);
    maxAreaParam->setMaximum(4194304);
    maxAreaParam->setValue(600);
    thresholdParam->setMinimum(0);
    thresholdParam->setMaximum(1);
    thresholdParam->setValue(0.15);
    thresholdParam->setSingleStep(0.01);
    minThresholdParam->setMinimum(0);
    minThresholdParam->setMaximum(1024);//value? 10bit
    minThresholdParam->setValue(6);
    useIthParam->setMinimum(1);
    useIthParam->setMaximum(100); //will be replaced..
    useIthParam->setValue(1);
    erodeParam->setMinimum(0);
    erodeParam->setMaximum(100); //will be replaced..
    erodeParam->setValue(5);
    diluteParam->setMinimum(0);
    diluteParam->setMaximum(100); //will be replaced..
    diluteParam->setValue(5);
    cleanClusterParam->setMinimum(0);
    cleanClusterParam->setMaximum(2048);
    cleanClusterParam->setValue(0);
    cleanClusterParam->setSingleStep(1);
    displayEachNFramesParam->setMinimum(0);
    displayEachNFramesParam->setMaximum(1000000);
    displayEachNFramesParam->setValue(1);
    displayEachNFramesParam->setSingleStep(1);
    checkBlackOnWhiteParam->setChecked(true);
    medianBlur1Param->setMinimum(0);
    medianBlur1Param->setMaximum(100);
    medianBlur1Param->setValue(0);
    medianBlur1Param->setSingleStep(2);
    medianBlur2Param->setMinimum(0);
    medianBlur2Param->setMaximum(100);
    medianBlur2Param->setValue(0);
    medianBlur2Param->setSingleStep(2);

    gaussianBlur1Param->setMinimum(0);
    gaussianBlur1Param->setMaximum(100);
    gaussianBlur1Param->setValue(0);
    gaussianBlur1Param->setSingleStep(2);
    gaussianSigma1Param->setMinimum(0);
    gaussianSigma1Param->setMaximum(100);
    gaussianSigma1Param->setSingleStep(0.01);
    gaussianSigma1Param->setValue(0);
    gaussianBlur2Param->setMinimum(0);
    gaussianBlur2Param->setMaximum(100);
    gaussianBlur2Param->setValue(0);
    gaussianBlur2Param->setSingleStep(2);
    gaussianSigma2Param->setMinimum(0);
    gaussianSigma2Param->setSingleStep(0.01);
    gaussianSigma2Param->setMaximum(100);
    gaussianSigma2Param->setValue(0);

    maskThresholdParam->setMinimum(-255);
    maskThresholdParam->setMaximum(255);
    maskThresholdParam->setValue(0);
}

void SegParameterWidget::minAChanged(double v)
{
    emit minAreaValueChanged(v);
}

void SegParameterWidget::maxAChanged(double v)
{
    emit maxAreaValueChanged(v);
}

void SegParameterWidget::thChanged(double v)
{
    emit thresholdValueChanged(v);
}

void SegParameterWidget::mThChanged(double v)
{
    emit minThresholdValueChanged(v);
}

void SegParameterWidget::ithChanged(int i)
{
    emit useIthValueChanged(i);
}

void SegParameterWidget::erodeChanged(int i)
{
    emit erodeValueChanged(i);
}

void SegParameterWidget::cleanClusterChanged(double v)
{
    emit cleanClusterValueChanged(v);
}

void SegParameterWidget::displayEachNFramesChanged(int i)
{
    emit displayEachNFramesValueChanged(i);
}

void SegParameterWidget::checkBlackOnWhiteChanged(int i)
{
    emit checkBlackOnWhiteValueChanged(i);
}

void SegParameterWidget::diluteChanged(int i)
{
    emit diluteValueChanged(i);
}

void SegParameterWidget::setMinThreshold(double mt)
{
    minThresholdParam->setValue(mt);
}

void SegParameterWidget::testParamsClicked()
{
    emit onTestParamsClicked();
}

void SegParameterWidget::startParamsClicked()
{
    emit onStartParamsClicked();
}

double SegParameterWidget::getMinA()
{
    return minAreaParam->value();
}

double SegParameterWidget::getMaxA()
{
    return maxAreaParam->value();
}

double SegParameterWidget::getThreshold()
{
    return thresholdParam->value();
}

double SegParameterWidget::getCleanCluster()
{
    return cleanClusterParam->value();
}

int SegParameterWidget::getDiplayEachNFrames()
{
    return displayEachNFramesParam->value();
}

bool SegParameterWidget::getBlackOnWhite()
{
    return checkBlackOnWhiteParam->isChecked();
}

double SegParameterWidget::getMinThreshold()
{
    return minThresholdParam->value();
}

int SegParameterWidget::getIth()
{
    return useIthParam->value();
}

int SegParameterWidget::getErode()
{
    return erodeParam->value();
}

int SegParameterWidget::getDilute()
{
    return diluteParam->value();
}

void SegParameterWidget::setMinA(double v)
{
    if(v>=0 && v<=4194304)
     minAreaParam->setValue(v);
    emit minAreaValueChanged(v);
}

void SegParameterWidget::setMaxA(double v)
{
    if(v>=0 && v<=4194304)
     maxAreaParam->setValue(v);
    emit maxAreaValueChanged(v);
}

void SegParameterWidget::setFracN(double v)
{
    if(v>=0 && v<=1)
     thresholdParam->setValue(v);
     emit thresholdValueChanged(v);
}

void SegParameterWidget::setMThreshold(double v)
{
    if(v>=0 && v<=1024)
     minThresholdParam->setValue(v);
    emit minThresholdValueChanged(v);
}

void SegParameterWidget::setUseIth(int i)
{
    if(i>=1 && i<=useIthParam->maximum())
     useIthParam->setValue(i);
    emit useIthValueChanged(i);
}

void SegParameterWidget::setErode(int i)
{
    if(i>=0 && i<=erodeParam->maximum())
     erodeParam->setValue(i);
    emit erodeValueChanged(i);
}
void SegParameterWidget::setDilute(int i)
{
    if(i>=0 && i<=diluteParam->maximum())
     diluteParam->setValue(i);
    emit diluteValueChanged(i);
}

void SegParameterWidget::setCleanCluster(double v)
{
    if(v>=cleanClusterParam->minimum() && v<=cleanClusterParam->maximum())
     cleanClusterParam->setValue(v);
    emit cleanClusterValueChanged(v);
}

void SegParameterWidget::setDisplayEachNFrames(int i)
{
    if(i>=displayEachNFramesParam->minimum() && i<=displayEachNFramesParam->maximum())
     displayEachNFramesParam->setValue(i);
    emit displayEachNFramesValueChanged(i);
}

void SegParameterWidget::setBlackOnWhite(bool b)
{
     checkBlackOnWhiteParam->setChecked(b);
}

int SegParameterWidget::getMedianBlur1Param()
{
    return medianBlur1Param->value();
}

int SegParameterWidget::getMedianBlur2Param()
{
    return medianBlur2Param->value();
}

int SegParameterWidget::getGaussianBlur1Param()
{
    return gaussianBlur1Param->value();
}

int SegParameterWidget::getGaussianBlur2Param()
{
    return gaussianBlur2Param->value();
}

double SegParameterWidget::getGaussianSigma1Param()
{
    return gaussianSigma1Param->value();
}

double SegParameterWidget::getGaussianSigma2Param()
{
    return gaussianSigma2Param->value();
}

void SegParameterWidget::setMaskThreshold(int i)
{
     maskThresholdParam->setValue(i);
}

int SegParameterWidget::getMaskThreshold()
{
    return maskThresholdParam->value();
}

void SegParameterWidget::checkG1(int i)
{
    if(i>0 && i%2==0)
     gaussianBlur1Param->setValue(i-1);
}

void SegParameterWidget::checkG2(int i)
{
    if(i>0 && i%2==0)
     gaussianBlur2Param->setValue(i-1);
}

void SegParameterWidget::checkM1(int i)
{
    if(i>0 && i%2==0)
     medianBlur1Param->setValue(i-1);
}

void SegParameterWidget::checkM2(int i)
{
    if(i>0 && i%2==0)
     medianBlur2Param->setValue(i-1);
}

void SegParameterWidget::setMedianBlur1Param(int i)
{
    medianBlur1Param->setValue(i);
    checkM1(i);
}

void SegParameterWidget::setMedianBlur2Param(int i)
{
    medianBlur2Param->setValue(i);
    checkM2(i);
}

void SegParameterWidget::setGaussianBlur1Param(int i)
{
    gaussianBlur1Param->setValue(i);
    checkG1(i);
}

void SegParameterWidget::setGaussianBlur2Param(int i)
{
    gaussianBlur2Param->setValue(i);
    checkG2(i);
}

void SegParameterWidget::setGaussianSigma1Param(double v)
{
    gaussianSigma1Param->setValue(v);
}

void SegParameterWidget::setGaussianSigma2Param(double v)
{
    gaussianSigma2Param->setValue(v);
}
