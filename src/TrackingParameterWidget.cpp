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
#include "TrackingParameterWidget.h"

#include <QLabel>
#include <QFormLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QScrollArea>

TrackingParameterWidget::TrackingParameterWidget()
    : minDistParam( new QDoubleSpinBox ),
      maxDistParam( new QDoubleSpinBox ),
      maxGapParam( new QSpinBox ),
      minAreaParam( new QDoubleSpinBox ),
      maxAreaParam( new QDoubleSpinBox ),
      maxNeighbourParam( new QSpinBox ),
      maxCoherenceLenghtParam( new QSpinBox ),
      minTrackDistParam( new QSpinBox ),
      trackDisplayLengthParam( new QSpinBox ),
      minTrackDisplayLengthParam( new QSpinBox ),
      appendOrOverwriteParam( new QCheckBox ),
      startParams( new QPushButton ("Start") ),
      maxDistAutoJoinParam( new QDoubleSpinBox ),
      maxGapAutoJoinParam( new QSpinBox )
{
    QWidget *scrollWidget = new QWidget();
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setGeometry(0,0,200,200);
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(tr("&MinDist:"), minDistParam);
    formLayout->addRow(tr("&MaxDist:"), maxDistParam);
    formLayout->addRow(tr("&MaxGap:"), maxGapParam);
    formLayout->addRow(tr("&MinArea:"), minAreaParam);
    formLayout->addRow(tr("&MaxArea:"), maxAreaParam);
    formLayout->addRow(tr("&MaxNeigh:"), maxNeighbourParam);
    formLayout->addRow(tr("&MaxCohLen:"), maxCoherenceLenghtParam);
    formLayout->addRow(tr("&MinTrDist:"), minTrackDistParam);
    formLayout->addRow(tr("&T.D.L.:"), trackDisplayLengthParam);
    formLayout->addRow(tr("&m.T.D.L.:"), minTrackDisplayLengthParam);
    formLayout->addRow(tr("&append:"), appendOrOverwriteParam);
    formLayout->addRow(tr("&MaxDist(aj):"), maxDistAutoJoinParam);
    formLayout->addRow(tr("&MaxGap(aj):"), maxGapAutoJoinParam);

    scrollWidget->setLayout(formLayout);
    scrollArea->setWidget(scrollWidget);

    QHBoxLayout *hLayout=new QHBoxLayout();
    hLayout->addWidget(startParams);
    QVBoxLayout *vLayout=new QVBoxLayout();
    vLayout->addWidget(scrollArea);
    vLayout->addLayout(hLayout);
    setLayout(vLayout);
    connect(minDistParam,SIGNAL(valueChanged(double)),this,SLOT(minDChanged(double)));
    connect(maxDistParam,SIGNAL(valueChanged(double)),this,SLOT(maxDChanged(double)));
    connect(maxDistAutoJoinParam,SIGNAL(valueChanged(double)),this,SLOT(maxDAutoJoinChanged(double)));
    connect(maxGapParam,SIGNAL(valueChanged(int)),this,SLOT(maxGChanged(int)));
    connect(maxGapAutoJoinParam,SIGNAL(valueChanged(int)),this,SLOT(maxGAutoJoinChanged(int)));
    connect(minAreaParam,SIGNAL(valueChanged(double)),this,SLOT(minAChanged(double)));
    connect(maxAreaParam,SIGNAL(valueChanged(double)),this,SLOT(maxAChanged(double)));
    connect(maxNeighbourParam,SIGNAL(valueChanged(int)),this,SLOT(maxNChanged(int)));
    connect(maxCoherenceLenghtParam,SIGNAL(valueChanged(int)),this,SLOT(maxCLChanged(int)));
    connect(minTrackDistParam,SIGNAL(valueChanged(int)),this,SLOT(minTDChanged(int)));
    connect(trackDisplayLengthParam,SIGNAL(valueChanged(int)),this,SLOT(trackDisplayLengthChanged(int)));
    connect(minTrackDisplayLengthParam,SIGNAL(valueChanged(int)),this,SLOT(minTrackDisplayLengthChanged(int)));
    connect(appendOrOverwriteParam,SIGNAL(stateChanged(int)),this,SLOT(appendOrOverwriteChanged(int)));
    connect(startParams,SIGNAL(clicked()),this,SLOT(startParamsClicked()));

    minAreaParam->setMinimum(0);
    minAreaParam->setMaximum(4194304);
    minAreaParam->setValue(8);
    maxAreaParam->setMinimum(0);
    maxAreaParam->setMaximum(4194304);
    maxAreaParam->setValue(600);

    minDistParam->setMinimum(0);
    minDistParam->setMaximum(4194304);
    minDistParam->setValue(10);
    maxDistParam->setMinimum(0);
    maxDistParam->setMaximum(4194304);
    maxDistParam->setValue(100);

    maxDistAutoJoinParam->setMinimum(0);
    maxDistAutoJoinParam->setMaximum(4194304);
    maxDistAutoJoinParam->setValue(100);

    maxGapParam->setMinimum(0);
    maxGapParam->setMaximum(10000);
    maxGapParam->setValue(15);

    maxGapAutoJoinParam->setMinimum(0);
    maxGapAutoJoinParam->setMaximum(10000);
    maxGapAutoJoinParam->setValue(15);

    maxNeighbourParam->setMinimum(0);
    maxNeighbourParam->setMaximum(10000);
    maxNeighbourParam->setValue(100);

    maxCoherenceLenghtParam->setMinimum(0);
    maxCoherenceLenghtParam->setMaximum(10000);
    maxCoherenceLenghtParam->setValue(15);

    minTrackDistParam->setMinimum(0);
    minTrackDistParam->setMaximum(1000000);
    minTrackDistParam->setValue(3);

    trackDisplayLengthParam->setMinimum(0);
    trackDisplayLengthParam->setMaximum(1000000);
    trackDisplayLengthParam->setValue(100);

    minTrackDisplayLengthParam->setMinimum(0);
    minTrackDisplayLengthParam->setMaximum(1000000);
    minTrackDisplayLengthParam->setValue(3);

    appendOrOverwriteParam->setChecked(false);
    
}

void TrackingParameterWidget::minDChanged(double v)
{
    emit minDValueChanged(v);
}

void TrackingParameterWidget::maxDChanged(double v)
{
    emit maxDValueChanged(v);
}

void TrackingParameterWidget::maxDAutoJoinChanged(double v)
{
    emit maxDAutoJoinValueChanged(v);
}

void TrackingParameterWidget::maxGChanged(int i)
{
    emit maxGValueChanged(i);
}

void TrackingParameterWidget::maxGAutoJoinChanged(int i)
{
    emit maxGAutoJoinValueChanged(i);
}

void TrackingParameterWidget::minAChanged(double v)
{
    emit minAValueChanged(v);
}

void TrackingParameterWidget::maxAChanged(double v)
{
    emit maxAValueChanged(v);
}

void TrackingParameterWidget::maxNChanged(int i)
{
    emit maxNValueChanged(i);
}

void TrackingParameterWidget::maxCLChanged(int i)
{
    emit maxCLValueChanged(i);
}

void TrackingParameterWidget::minTDChanged(int i)
{
    emit minTDValueChanged(i);
}

void TrackingParameterWidget::trackDisplayLengthChanged(int i)
{
    emit trackDisplayLengthValueChanged(i);
}

void TrackingParameterWidget::minTrackDisplayLengthChanged(int i)
{
    emit minTrackDisplayLengthValueChanged(i);
}

void TrackingParameterWidget::appendOrOverwriteChanged(int i)
{
    emit appendOrOverwriteValueChanged(i);
}

void TrackingParameterWidget::startParamsClicked()
{
    emit onStartParamsClicked();
}

double TrackingParameterWidget::getMinD()
{
    return minDistParam->value();
}

double TrackingParameterWidget::getMaxD()
{
    return maxDistParam->value();
}

double TrackingParameterWidget::getMaxDAutoJoin()
{
    return maxDistAutoJoinParam->value();
}

int TrackingParameterWidget::getMaxG()
{
    return maxGapParam->value();
}

int TrackingParameterWidget::getMaxGAutoJoin()
{
    return maxGapAutoJoinParam->value();
}

double TrackingParameterWidget::getMinA()
{
    return minAreaParam->value();
}

double TrackingParameterWidget::getMaxA()
{
    return maxAreaParam->value();
}

int TrackingParameterWidget::getMaxN()
{
    return maxNeighbourParam->value();
}

int TrackingParameterWidget::getMaxCL()
{
    return maxCoherenceLenghtParam->value();
}

int TrackingParameterWidget::getMinTD()
{
    return minTrackDistParam->value();
}

int TrackingParameterWidget::getTrackDisplayLength()
{
    return trackDisplayLengthParam->value();
}

int TrackingParameterWidget::getMinTrackDisplayLength()
{
    return minTrackDisplayLengthParam->value();
}

bool TrackingParameterWidget::getAppendOrOverwrite()
{
    return appendOrOverwriteParam->isChecked();
}

void TrackingParameterWidget::setMinD(double v)
{
    if(v>=0 && v<=4194304)
     minDistParam->setValue(v);
    emit minDValueChanged(v);
}

void TrackingParameterWidget::setMaxD(double v)
{
    if(v>=0 && v<=4194304)
     maxDistParam->setValue(v);
    emit maxDValueChanged(v);
}

void TrackingParameterWidget::setMaxG(int i)
{
    if(i>=0 && i<=1000)
     maxGapParam->setValue(i);
    emit maxGValueChanged(i);
}

void TrackingParameterWidget::setMinA(double v)
{
    if(v>=0 && v<=4194304)
     minAreaParam->setValue(v);
    emit minAValueChanged(v);
}

void TrackingParameterWidget::setMaxA(double v)
{
    if(v>=0 && v<=4194304)
     maxAreaParam->setValue(v);
    emit maxAValueChanged(v);
}

void TrackingParameterWidget::setMaxN(int i)
{
    if(i>=0 && i<=10000)
     maxNeighbourParam->setValue(i);
    emit maxNValueChanged(i);
}

void TrackingParameterWidget::setMaxCL(int i)
{
    if(i>=0 && i<=10000)
     maxCoherenceLenghtParam->setValue(i);
    emit maxCLValueChanged(i);
}

void TrackingParameterWidget::setMinTD(int i)
{
    if(i>=0 && i<=10000)
     minTrackDistParam->setValue(i);
    emit minTDValueChanged(i);
}

void TrackingParameterWidget::setTrackDisplayLength(int i)
{
    if(i>=0 && i<=10000)
     trackDisplayLengthParam->setValue(i);
    emit trackDisplayLengthValueChanged(i);
}

void TrackingParameterWidget::setMinTrackDisplayLength(int i)
{
    if(i>=0 && i<=10000)
     minTrackDisplayLengthParam->setValue(i);
    emit minTrackDisplayLengthValueChanged(i);
}

void TrackingParameterWidget::setAppendOrOverwrite(int i)
{
    if(i==0)
       appendOrOverwriteParam->setChecked(false);
    else
       appendOrOverwriteParam->setChecked(true);
    emit appendOrOverwriteValueChanged(i);
}
