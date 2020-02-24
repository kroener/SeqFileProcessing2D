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
#include "AnimationParameterWidget.h"

#include <QLabel>
#include <QFormLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>

AnimationParameterWidget::AnimationParameterWidget()
    : drawPoints( new QCheckBox ),
      drawTracks( new QCheckBox ),
      drawTime( new QCheckBox ),
      scaleOutput( new QCheckBox ),
      outputWidth( new QSpinBox ),
      colorPointR( new QSpinBox ),
      colorPointG( new QSpinBox ),
      colorPointB( new QSpinBox ),
      colorTrackR( new QSpinBox ),
      colorTrackG( new QSpinBox ),
      colorTrackB( new QSpinBox ),
      pointRadius( new QSpinBox ),
      trackThickness( new QSpinBox ),
      trackLength( new QSpinBox ),   
      trackMinLength( new QSpinBox ),    
      createAnimation( new QPushButton ("Create") ),
      trackColor( new QComboBox )
{

    QHBoxLayout *cP=new QHBoxLayout();
    cP->addWidget(colorPointR);
    cP->addWidget(colorPointG);
    cP->addWidget(colorPointB);
    QHBoxLayout *cT=new QHBoxLayout();
    cT->addWidget(colorTrackR);
    cT->addWidget(colorTrackG);
    cT->addWidget(colorTrackB);
    

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(tr("&Points:"), drawPoints);
    formLayout->addRow(tr("&Tracks:"), drawTracks);
    formLayout->addRow(tr("&Time:"), drawTime);
    formLayout->addRow(tr("&Scale:"), scaleOutput);
    formLayout->addRow(tr("&oWidth:"), outputWidth);
    formLayout->addRow(tr("&Point Color:"), cP);
    formLayout->addRow(tr("&Track Color By:"), trackColor );
    formLayout->addRow(tr("&Track Color:"), cT);
    formLayout->addRow(tr("&Radius:"), pointRadius);
    formLayout->addRow(tr("&T. thickness:"),trackThickness );
    formLayout->addRow(tr("&T. length:"), trackLength);
    formLayout->addRow(tr("&m.T.L.:"), trackMinLength);
    QHBoxLayout *hLayout=new QHBoxLayout();
    hLayout->addWidget(createAnimation);
    QVBoxLayout *vLayout=new QVBoxLayout();
    vLayout->addLayout(formLayout);
    vLayout->addLayout(hLayout);
    setLayout(vLayout);

    //set default values:
    drawPoints->setChecked(true);
    drawTracks->setChecked(true);
    drawTime->setChecked(true);
    scaleOutput->setChecked(false);
    
    outputWidth->setMinimum(128);
    outputWidth->setMaximum(2048);
    outputWidth->setValue(2048);

    colorPointR->setMinimum(0);
    colorPointR->setMaximum(255);
    colorPointR->setValue(0);

    colorPointG->setMinimum(0);
    colorPointG->setMaximum(255);
    colorPointG->setValue(0);

    colorPointB->setMinimum(0);
    colorPointB->setMaximum(255);
    colorPointB->setValue(255);

    colorTrackR->setMinimum(0);
    colorTrackR->setMaximum(255);
    colorTrackR->setValue(255);

    colorTrackG->setMinimum(0);
    colorTrackG->setMaximum(255);
    colorTrackG->setValue(0);

    colorTrackB->setMinimum(0);
    colorTrackB->setMaximum(255);
    colorTrackB->setValue(0);

    pointRadius->setMinimum(0);
    pointRadius->setMaximum(100);
    pointRadius->setValue(5);

    trackThickness->setMinimum(0);
    trackThickness->setMaximum(100);
    trackThickness->setValue(2);

    trackLength->setMinimum(0);
    trackLength->setMaximum(1000000);
    trackLength->setValue(1000);

    trackMinLength->setMinimum(0);
    trackMinLength->setMaximum(1000000);
    trackMinLength->setValue(2);
 
    trackColor->addItem(tr("FixedColor"));
    trackColor->addItem(tr("First ID"));
    trackColor->addItem(tr("Track ID"));
    trackColor->addItem(tr("Random"));
    trackColor->addItem(tr("Rainbow"));
    trackColor->setCurrentIndex(0);
    // 1 color track by first occurence
    // 2 color by track ID(rainbow)
    // 3 color by track ID(random)
    // 4 rainbow tail...
    connect(createAnimation,SIGNAL(clicked()),this,SLOT(createAnimationClicked()));
}

int AnimationParameterWidget::getOutputWidth()
{
    return outputWidth->value();
}

int AnimationParameterWidget::getColorPointR()
{
    return colorPointR->value();
}

int AnimationParameterWidget::getColorPointG()
{
    return colorPointG->value();
}

int AnimationParameterWidget::getColorPointB()
{
    return colorPointB->value();
}

int AnimationParameterWidget::getColorTrackR()
{
    return colorTrackR->value();
}

int AnimationParameterWidget::getColorTrackG()
{
    return colorTrackG->value();
}

int AnimationParameterWidget::getColorTrackB()
{
    return colorTrackB->value();
}

int AnimationParameterWidget::getPointRadius()
{
    return pointRadius->value();
}

int AnimationParameterWidget::getTrackThickness()
{
    return trackThickness->value();
}

int AnimationParameterWidget::getTrackLength()
{
    return trackLength->value();
}

int AnimationParameterWidget::getTrackMinLength()
{
    return trackMinLength->value();
}

bool AnimationParameterWidget::getDrawPoints()
{
    return drawPoints->isChecked();
}

bool AnimationParameterWidget::getDrawTracks()
{
    return drawTracks->isChecked();
}

bool AnimationParameterWidget::getDrawTime()
{
    return drawTime->isChecked();
}

bool AnimationParameterWidget::getScaleOutput()
{
    return scaleOutput->isChecked();
}

int AnimationParameterWidget::getTrackColorMode()
{
    return trackColor->currentIndex();
}


void AnimationParameterWidget::createAnimationClicked()
{
    emit onCreateAnimationClicked();
}
