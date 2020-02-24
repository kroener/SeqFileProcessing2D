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
#ifndef SEQINFOWIDGET_H
#define SEQINFOWIDGET_H
#include "Header.hpp"
#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
class QLabel;
class QComboBox;
class QCheckBox;

/**
  * \brief Widget to display details of selected *.seq file
  */
class SeqInfoWidget : public QWidget
{

    Q_OBJECT

public:
    SeqInfoWidget();
    void setCurrent(int i);
    void addToList(string filename);
    void removeFromList(int i);

public slots:
    bool updateInfo(string filename,Header* header, string pF=string(), string tF=string());

private slots:
    void on_seqFile_Selected(int i);

signals:
    void seqFileSelected(int);

private:
    QComboBox *seqList;
    QLabel *nameInfo;
    QLabel *frameInfo;
    QLabel *fpsInfo;
    QLabel *widthInfo;
    QLabel *heightInfo;
    QLabel *bitsInfo;
    QCheckBox *metaInfo;
    QLabel *posFile;
    QLabel *trackFile;
    QLabel *posFileDesc;
    QLabel *trackFileDesc;
};
#endif // SEQINFOWIDGET_H  
