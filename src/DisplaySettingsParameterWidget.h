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
#ifndef DISPLAYSETTINGSPARAMETERWIDGET_H
#define DISPLAYSETTINGSPARAMETERWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>

class QLabel;
class QPushButton;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QComboBox;
/** 
  * \brief Widget to select Tracking parameter
  */
class DisplaySettingsParameterWidget : public QWidget
{

    Q_OBJECT

public:
    DisplaySettingsParameterWidget();

signals:
    void showPointsChanged(int);
    void disablePosBackupChanged(int);
    void showTracksChanged(int);
    void syncOnMetaChanged(int);
    void selectOnMetaChanged(int);
    void pointSizeChanged(double);
    void trackWidthChanged(double);
    void ArrayWidthChanged(int);
    void ArrayHeightChanged(int);
    void displayContoursChanged(int);
    void minGChanged(int);
    void maxGChanged(int);
    void varPointRadiusChanged(double);
public slots:
    bool getShowPoints();
    bool getShowTracks();
    bool getSyncOnMeta();
    bool getSelectOnMeta();
    double getPointSize();
    double getTrackWidth();
    int getArrayWidth();
    int getArrayHeight();
    int getMinG();
    int getMaxG();
    bool setShowPoints(bool b);
    bool setShowTracks(bool b);
    bool setSyncOnMeta(bool b);
    bool setSelectOnMeta(bool b);
    bool setPointSize(double v);
    bool setTrackWidth(double v);
    bool setArrayWidth(int i);
    bool setArrayHeight(int i);
    bool setMinG(int i);
    bool setMaxG(int i);
    void showPointsStateChanged(int i);
    void showTracksStateChanged(int i);
    void syncOnMetaStateChanged(int i);
    void selectOnMetaStateChanged(int i);
    void pointSizeValueChanged(double v);
    void trackWidthValueChanged(double v);
    void ArrayWidthValueChanged(int i);
    void ArrayHeightValueChanged(int i);
    void disablePosBackupStateChanged(int i);

    void minGValueChanged(int i);
    void maxGValueChanged(int i);
    bool getDisplayContours();
    bool setDisplayContours(bool b);
    void displayContoursStateChanged(int i);

    bool getOnlyOne();
    bool setOnlyOne(bool b);
    double getSearchRadius();
    bool setSearchRadius(double v);
    double getVarPointRadius();
    bool setVarPointRadius(double v);
    void varPointRadiusValueChanged(double v);
    
private:
    QCheckBox *showPoints;
    QCheckBox *showTracks;
    QDoubleSpinBox *pointSize;
    QDoubleSpinBox *trackWidth;
    QCheckBox *syncOnMeta;
    QCheckBox *selectOnMeta;
    QSpinBox *ArrayWidth;
    QSpinBox *ArrayHeight;
    QCheckBox *displayContours;
    QSpinBox *minG;
    QSpinBox *maxG;
    QDoubleSpinBox *searchRadius;
    QCheckBox *onlyOne;
    QCheckBox *disablePosBackup;
    QDoubleSpinBox *varPointRadius;
};
#endif // DISPLAYSETTINGSPARAMETERWIDGET_H  
