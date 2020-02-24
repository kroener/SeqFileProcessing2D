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
#include "DisplaySettingsParameterWidget.h"

#include <QLabel>
#include <QFormLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QScrollArea>

DisplaySettingsParameterWidget::DisplaySettingsParameterWidget()
    :showPoints( new QCheckBox ),
    showTracks( new QCheckBox ),
    pointSize( new QDoubleSpinBox ),
    trackWidth( new QDoubleSpinBox ),
    syncOnMeta( new QCheckBox ),
    selectOnMeta( new QCheckBox ),
    ArrayWidth( new QSpinBox ),
    ArrayHeight( new QSpinBox ),
    displayContours( new QCheckBox ),
    minG( new QSpinBox ),
    maxG( new QSpinBox ),
    searchRadius(new QDoubleSpinBox ),
    onlyOne( new QCheckBox ),
    disablePosBackup( new QCheckBox ),
    varPointRadius(new QDoubleSpinBox )
{
    QWidget *scrollWidget = new QWidget();
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setGeometry(0,0,200,200);
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(tr("Show Mosquitoes:"), showPoints);
    formLayout->addRow(tr("Show Tracks:"), showTracks);
    formLayout->addRow(tr("Display Contours:"), displayContours);
    formLayout->addRow(tr("Point Size:"), pointSize);
    formLayout->addRow(tr("Track Width:"), trackWidth);
    formLayout->addRow(tr("syncOnMeta:"), syncOnMeta);
    formLayout->addRow(tr("selectOnMeta:"), selectOnMeta);
    formLayout->addRow(tr("Disp. cols:"), ArrayWidth);
    formLayout->addRow(tr("Disp. rows:"), ArrayHeight);
    formLayout->addRow(tr("minG:"), minG);
    formLayout->addRow(tr("maxG:"), maxG);
    formLayout->addRow(tr("searchRadius:"), searchRadius);
    formLayout->addRow(tr("onlyOne:"), onlyOne);
    formLayout->addRow(tr("disablePosBackup:"), disablePosBackup);
    formLayout->addRow(tr("varPointR:"), varPointRadius);
    scrollWidget->setLayout(formLayout);
    scrollArea->setWidget(scrollWidget);
    QVBoxLayout *vLayout=new QVBoxLayout();
    vLayout->addWidget(scrollArea);
    setLayout(vLayout);

    showPoints->setChecked(true);
    disablePosBackup->setChecked(false);
    showTracks->setChecked(true);
    syncOnMeta->setChecked(false);
    pointSize->setMinimum(0);
    pointSize->setMaximum(100);
    pointSize->setValue(10);
    varPointRadius->setMinimum(0);
    varPointRadius->setMaximum(100);
    varPointRadius->setSingleStep(0.5);
    varPointRadius->setValue(0);

    displayContours->setChecked(true);

    trackWidth->setMinimum(0);
    trackWidth->setMaximum(10);
    trackWidth->setValue(10);
 
    ArrayWidth->setMinimum(1);
    ArrayWidth->setMaximum(4);
    ArrayWidth->setValue(1);
 
    ArrayHeight->setMinimum(1);
    ArrayHeight->setMaximum(4);
    ArrayHeight->setValue(1);

    minG->setMinimum(0);
    minG->setMaximum(254);
    minG->setValue(0);
    maxG->setMinimum(1);
    maxG->setMaximum(255);
    maxG->setValue(255);

    onlyOne->setChecked(true);

    searchRadius->setMinimum(0);
    searchRadius->setMaximum(1000);
    searchRadius->setValue(8);
 
    connect(showPoints,SIGNAL(stateChanged(int)),this,SLOT(showPointsStateChanged(int)));
    connect(showTracks,SIGNAL(stateChanged(int)),this,SLOT(showTracksStateChanged(int)));
    connect(pointSize,SIGNAL(valueChanged(double)),this,SLOT(pointSizeValueChanged(double)));
    connect(trackWidth,SIGNAL(valueChanged(double)),this,SLOT(trackWidthValueChanged(double)));
    connect(syncOnMeta,SIGNAL(stateChanged(int)),this,SLOT(syncOnMetaStateChanged(int)));
    connect(selectOnMeta,SIGNAL(stateChanged(int)),this,SLOT(selectOnMetaStateChanged(int)));
    connect(ArrayHeight,SIGNAL(valueChanged(int)),this,SLOT(ArrayHeightValueChanged(int)));
    connect(ArrayWidth,SIGNAL(valueChanged(int)),this,SLOT(ArrayWidthValueChanged(int)));
    connect(displayContours,SIGNAL(stateChanged(int)),this,SLOT(displayContoursStateChanged(int)));
    connect(minG,SIGNAL(valueChanged(int)),this,SLOT(minGValueChanged(int))); 
    connect(maxG,SIGNAL(valueChanged(int)),this,SLOT(maxGValueChanged(int))); 
    connect(disablePosBackup,SIGNAL(stateChanged(int)),this,SLOT(disablePosBackupStateChanged(int)));
    connect(varPointRadius,SIGNAL(valueChanged(double)),this,SLOT(varPointRadiusValueChanged(double))); 
}

bool DisplaySettingsParameterWidget::getShowPoints()
{
    return showPoints->isChecked();
}

bool DisplaySettingsParameterWidget::getShowTracks()
{
    return showTracks->isChecked();
}

bool DisplaySettingsParameterWidget::getSyncOnMeta()
{
    return syncOnMeta->isChecked();
}

bool DisplaySettingsParameterWidget::getSelectOnMeta()
{
    return selectOnMeta->isChecked();
}

double DisplaySettingsParameterWidget::getPointSize()
{
    return pointSize->value();
}

double DisplaySettingsParameterWidget::getTrackWidth()
{
    return trackWidth->value();
}

int DisplaySettingsParameterWidget::getArrayWidth()
{
    return ArrayWidth->value();
}

int DisplaySettingsParameterWidget::getArrayHeight()
{
    return ArrayHeight->value();
}

int DisplaySettingsParameterWidget::getMinG()
{
    return minG->value();
}

int DisplaySettingsParameterWidget::getMaxG()
{
    return maxG->value();
}

bool DisplaySettingsParameterWidget::setShowPoints(bool b)
{
    showPoints->setChecked(b);
    return true;
}

bool DisplaySettingsParameterWidget::setShowTracks(bool b)
{
    showTracks->setChecked(b);
    return true;
}

bool DisplaySettingsParameterWidget::setSyncOnMeta(bool b)
{
    syncOnMeta->setChecked(b);
    return true;
}

bool DisplaySettingsParameterWidget::setSelectOnMeta(bool b)
{
    selectOnMeta->setChecked(b);
    return true;
}

bool DisplaySettingsParameterWidget::setPointSize(double v)
{
    pointSize->setValue(v);
    return true;
}

bool DisplaySettingsParameterWidget::setTrackWidth(double v)
{
    trackWidth->setValue(v);
    return true;
}

bool DisplaySettingsParameterWidget::setArrayWidth(int i)
{
    ArrayWidth->setValue(i);
    return true;
}

bool DisplaySettingsParameterWidget::setArrayHeight(int i)
{
    ArrayHeight->setValue(i);
    return true;
}

bool DisplaySettingsParameterWidget::setMinG(int i)
{
    minG->setValue(i);
    return true;
}

bool DisplaySettingsParameterWidget::setMaxG(int i)
{
    maxG->setValue(i);
    return true;
}


void DisplaySettingsParameterWidget::showPointsStateChanged(int i)
{
    emit showPointsChanged(i);
}

void DisplaySettingsParameterWidget::showTracksStateChanged(int i)
{
    emit showTracksChanged(i);
}

void DisplaySettingsParameterWidget::syncOnMetaStateChanged(int i)
{
    emit syncOnMetaChanged(i);
}

void DisplaySettingsParameterWidget::selectOnMetaStateChanged(int i)
{
    emit selectOnMetaChanged(i);
}

void DisplaySettingsParameterWidget::pointSizeValueChanged(double v)
{
    emit pointSizeChanged(v);
}

void DisplaySettingsParameterWidget::trackWidthValueChanged(double v)
{
    emit trackWidthChanged(v);
}

void DisplaySettingsParameterWidget::ArrayWidthValueChanged(int i)
{
    emit ArrayWidthChanged(i);
}

void DisplaySettingsParameterWidget::ArrayHeightValueChanged(int i)
{
    emit ArrayHeightChanged(i);
}

void DisplaySettingsParameterWidget::minGValueChanged(int i)
{
    emit minGChanged(i);
}

void DisplaySettingsParameterWidget::maxGValueChanged(int i)
{
    emit maxGChanged(i);
}

bool DisplaySettingsParameterWidget::getDisplayContours()
{
    return displayContours->isChecked();
}

bool DisplaySettingsParameterWidget::setDisplayContours(bool b)
{
    displayContours->setChecked(b);
    return true;
}

void DisplaySettingsParameterWidget::displayContoursStateChanged(int i)
{
    emit displayContoursChanged(i);
}

bool DisplaySettingsParameterWidget::getOnlyOne()
{
    return onlyOne->isChecked();
}

bool DisplaySettingsParameterWidget::setOnlyOne(bool b)
{
    onlyOne->setChecked(b);
    return true;
}

double DisplaySettingsParameterWidget::getSearchRadius()
{
    return searchRadius->value();
}

bool DisplaySettingsParameterWidget::setSearchRadius(double v)
{
    searchRadius->setValue(v);
    return true;
}

void DisplaySettingsParameterWidget::disablePosBackupStateChanged(int i)
{
    emit disablePosBackupChanged(i);
}

double DisplaySettingsParameterWidget::getVarPointRadius()
{
    return varPointRadius->value();
}

bool DisplaySettingsParameterWidget::setVarPointRadius(double v)
{
    varPointRadius->setValue(v);
    return true;
}

void DisplaySettingsParameterWidget::varPointRadiusValueChanged(double v)
{
    emit varPointRadiusChanged(v);
}
