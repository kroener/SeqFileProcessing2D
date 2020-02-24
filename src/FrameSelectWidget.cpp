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
#include "FrameSelectWidget.h"
#include "Header.hpp"
#include <QLabel>
#include <QFormLayout>
#include <QScrollArea>
#include <QSpinBox>
#include <QDoubleSpinBox>

FrameSelectWidget::FrameSelectWidget()
    : fromSelectParam( new QSpinBox ),
      toSelectParam( new QSpinBox ),
      stepSelectParam( new QSpinBox )
{
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(tr("&From:"), fromSelectParam);
    formLayout->addRow(tr("&To:"), toSelectParam);
    formLayout->addRow(tr("&Step:"), stepSelectParam);
    setLayout(formLayout);
    connect(fromSelectParam,SIGNAL(valueChanged(int)),this,SLOT(fromChanged(int)));
    connect(toSelectParam,SIGNAL(valueChanged(int)),this,SLOT(toChanged(int)));
    connect(stepSelectParam,SIGNAL(valueChanged(int)),this,SLOT(stepChanged(int)));
    useMeta=false;
}

bool FrameSelectWidget::update(Header* header)
{
    frames=header->allocatedFrames();
    fromSelectParam->setMinimum(0);
    fromSelectParam->setMaximum(frames-2);
    fromSelectParam->setValue(0);
    toSelectParam->setMinimum(1);
    toSelectParam->setMaximum(frames-1);
    toSelectParam->setValue(frames-1);
    stepSelectParam->setMinimum(1);
    stepSelectParam->setMaximum(frames-1);
    stepSelectParam->setValue(1);

    return true;
}

void FrameSelectWidget::toChanged(int i)
{
    emit toValueChanged(i);
}

void FrameSelectWidget::fromChanged(int i)
{
    emit fromValueChanged(i);
}

void FrameSelectWidget::stepChanged(int i)
{
    emit stepValueChanged(i);
}

void FrameSelectWidget::setTo(int i)
{
    if(i<toSelectParam->minimum()) i=toSelectParam->minimum();
    if(i>toSelectParam->maximum()) i=toSelectParam->maximum();
    toSelectParam->setValue(i);
}

void FrameSelectWidget::setFrom(int i)
{
    if(i<fromSelectParam->minimum()) i=fromSelectParam->minimum();
    if(i>fromSelectParam->maximum()) i=fromSelectParam->maximum();
    fromSelectParam->setValue(i);
}

unsigned long FrameSelectWidget::getFrom()
{
    return fromSelectParam->value();
}

unsigned long FrameSelectWidget::getTo()
{
    return toSelectParam->value();
}

unsigned long FrameSelectWidget::getStep()
{
    return stepSelectParam->value();
}

void FrameSelectWidget::setFromFrame(int i)
{
 if(i>=fromSelectParam->minimum() && i<= fromSelectParam->maximum())
  fromSelectParam->setValue(i);
 emit fromValueChanged(i);
}

void FrameSelectWidget::setToFrame(int i)
{
 if(i>=toSelectParam->minimum() && i<= toSelectParam->maximum())
  toSelectParam->setValue(i);
 emit toValueChanged(i);
}

void FrameSelectWidget::setFrameStep(int i)
{
 if(i>=stepSelectParam->minimum() && i<= stepSelectParam->maximum())
  stepSelectParam->setValue(i);
 emit stepValueChanged(i);
}

void FrameSelectWidget::setMin(int i)
{
 if(useMeta)
 {
    fromSelectParam->setMinimum(i);
    toSelectParam->setMinimum(i+1);
 } 
}

void FrameSelectWidget::setMax(int i)
{
 if(useMeta)
 {
    fromSelectParam->setMaximum(i);
    toSelectParam->setMaximum(i);
 }
}

void FrameSelectWidget::metaMode(bool b)
{
 useMeta=b;
 if(!useMeta)
  unsetMetaMode();
}

void FrameSelectWidget::unsetMetaMode()
{
    fromSelectParam->setMinimum(0);
    fromSelectParam->setMaximum(frames-2);
    toSelectParam->setMinimum(1);
    toSelectParam->setMaximum(frames-1);
    stepSelectParam->setMinimum(1);
    stepSelectParam->setMaximum(frames-1);
    
}

bool FrameSelectWidget::getMetaMode()
{
    return useMeta;
}
