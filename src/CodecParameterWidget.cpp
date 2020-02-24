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
#include "CodecParameterWidget.h"

#include <QLabel>
#include <QFormLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>

CodecParameterWidget::CodecParameterWidget()
    :codec( new QComboBox ),
    preset( new QComboBox ),
    hardware( new QCheckBox ),
    bitrate( new QSpinBox ),
    gopSize( new QSpinBox ),
    fNum( new QSpinBox ),
    fDen( new QSpinBox ),
    useCrf( new QCheckBox ),
    crf( new QSpinBox )
{

    QHBoxLayout *fr=new QHBoxLayout();
    fr->addWidget(fDen);
    fr->addWidget(fNum);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(tr("Codec:"), codec);
    formLayout->addRow(tr("Preset:"), preset);
    formLayout->addRow(tr("Hardware:"), hardware);
    formLayout->addRow(tr("Bitrate:"), bitrate);
    formLayout->addRow(tr("gop Size:"), gopSize);
    formLayout->addRow(tr("framerate:"), fr);
    formLayout->addRow(tr("Use CRF:"), useCrf);
    formLayout->addRow(tr("CRF:"), crf);
    QVBoxLayout *vLayout=new QVBoxLayout();
    vLayout->addLayout(formLayout);
    setLayout(vLayout);

    //set default values:
    hardware->setChecked(true);

    bitrate->setMinimum(1000);
    bitrate->setMaximum(100000000);
    bitrate->setValue(17000000);
    bitrate->setSingleStep(500000);

    gopSize->setMinimum(0);
    gopSize->setMaximum(5000);
    gopSize->setValue(50);

    fNum->setMinimum(1);
    fNum->setMaximum(65536);
    fNum->setValue(1);
    fDen->setMinimum(1);
    fDen->setMaximum(65536);
    fDen->setValue(50);

    codec->addItem(tr("H.264"));
    codec->addItem(tr("H.265"));

    preset->addItem(tr("slow"));
    preset->addItem(tr("fast"));

    crf->setMinimum(1);
    crf->setMaximum(51);
    crf->setValue(18);

    useCrf->setChecked(false);
    
}

bool CodecParameterWidget::getHardware()
{
    return hardware->isChecked();
}

int CodecParameterWidget::getBitrate()
{
    return bitrate->value();
}

int CodecParameterWidget::getGopSize()
{
    return gopSize->value();
}

int CodecParameterWidget::getFrameNum()
{
    return fNum->value();
}

int CodecParameterWidget::getFrameDen()
{
    return fDen->value();
}

int CodecParameterWidget::getCodec()
{
    return codec->currentIndex();
}

int CodecParameterWidget::getPreset()
{
    return preset->currentIndex();
}

void CodecParameterWidget::setHardware(bool b)
{
    hardware->setChecked(b);
}

void CodecParameterWidget::setBitrate(int i)
{
    bitrate->setValue(i);
}

void CodecParameterWidget::setGopSize(int i)
{
    gopSize->setValue(i);
}

void CodecParameterWidget::setFrameNum(int i)
{
    fNum->setValue(i);
}

void CodecParameterWidget::setFrameDen(int i)
{
    fDen->setValue(i);
}
void CodecParameterWidget::setCodec(int i)
{
    if(i<codec->count()&& i>=0) 
        codec->setCurrentIndex(i);
}

void CodecParameterWidget::setPreset(int i)
{
    if(i<preset->count()&& i>=0) 
        preset->setCurrentIndex(i);
}

bool CodecParameterWidget::getUseCrf()
{
    return useCrf->isChecked();
}

int CodecParameterWidget::getCrf()
{
    return crf->value();
}

void CodecParameterWidget::setUseCrf(bool b)
{
    useCrf->setChecked(b);
}

void CodecParameterWidget::setCrf(int i)
{
    crf->setValue(i);
}
