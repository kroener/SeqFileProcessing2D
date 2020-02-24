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
#include "SeqInfoWidget.h"
#include "Header.hpp"
#include <QLabel>
#include <QString>
#include <QFormLayout>
#include <QScrollArea>
#include <QComboBox>
#include <QCheckBox>

SeqInfoWidget::SeqInfoWidget()
    : seqList( new QComboBox() ),
      nameInfo( new QLabel("") ),
      frameInfo( new QLabel("0") ),
      fpsInfo( new QLabel("0") ),
      widthInfo( new QLabel("0") ),
      heightInfo( new QLabel("0") ),
      bitsInfo( new QLabel("0") ),
      metaInfo( new QCheckBox() ),
      posFile( new QLabel("") ),
      trackFile( new QLabel("") ),
      posFileDesc(new QLabel("Pos. File:")),
      trackFileDesc(new QLabel("Track File:"))
{
    QWidget *scrollWidget = new QWidget();
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setGeometry(0,0,400,200);
    scrollArea->setWidgetResizable( true );
    metaInfo->setEnabled(false);
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(tr("File Name:"), nameInfo);
    formLayout->addRow(tr("Frames:"), frameInfo);
    formLayout->addRow(tr("Framerate:"), fpsInfo);
    formLayout->addRow(tr("Width(pix):"), widthInfo);
    formLayout->addRow(tr("Height(pix):"), heightInfo);
    formLayout->addRow(tr("Bits:"), bitsInfo);
    formLayout->addRow(tr("Metadata:"), metaInfo);
    formLayout->addRow(posFileDesc, posFile);
    formLayout->addRow(trackFileDesc, trackFile);
    QVBoxLayout *vLayout = new QVBoxLayout();
    scrollWidget->setLayout(formLayout);
    scrollArea->setWidget(scrollWidget);
    vLayout->addWidget(seqList);
    vLayout->addWidget(scrollArea);
    setLayout(vLayout);
    posFileDesc->hide();
    posFile->hide();
    trackFileDesc->hide();
    trackFile->hide();
    connect(seqList,SIGNAL(currentIndexChanged(int)),this, SLOT(on_seqFile_Selected(int)));    
}

bool SeqInfoWidget::updateInfo(string filename,Header* header, string pF/*=string()*/, string tF/*=string()*/)
{
    QString fFull=QString::fromStdString(filename);
    QString fName = fFull.section("/",-1,-1);
    nameInfo->setText(fName);
    frameInfo->setText(QString::number(header->allocatedFrames()));
    fpsInfo->setText(QString::number(header->framerate()));
    widthInfo->setText(QString::number(header->width()));
    heightInfo->setText(QString::number(header->height()));
    bitsInfo->setText(QString::number(header->depth()));
    if(header->seqHasMeta())
     metaInfo->setChecked(true);
    else
     metaInfo->setChecked(false); 
    if(pF!="")
    {
    QString pFull=QString::fromStdString(pF);
    QString pName = pFull.section("/",-1,-1);
    posFile->setText(pName);
    if(!posFile->isVisible())
    {
        posFileDesc->show();
        posFile->show();
    }
    }
    else if(posFile->isVisible())
    {
        posFileDesc->hide();
        posFile->hide();
    }
    if(tF!="")
    {
    QString tFull=QString::fromStdString(tF);
    QString tName = tFull.section("/",-1,-1);
    trackFile->setText(tName);
    if(!trackFile->isVisible())
    {
        trackFileDesc->show();
        trackFile->show();
    }
    }
    else if(trackFile->isVisible())
    {
        trackFileDesc->hide();
        trackFile->hide();
    }
    
    return true;
}

void SeqInfoWidget::addToList(string filename)
{
    QString fFull=QString::fromStdString(filename);
    QString fName = fFull.section("/",-1,-1);
    seqList->addItem(fName);
}

void SeqInfoWidget::removeFromList(int i)
{
    seqList->removeItem(i);
}

void SeqInfoWidget::setCurrent(int i)
{
    seqList->setCurrentIndex(i);
}

void SeqInfoWidget::on_seqFile_Selected(int i)
{
    emit seqFileSelected(i);
}
