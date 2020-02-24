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
#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QPen>
#include <QWidget>
#include <vector>

/**
  * \brief Widget to draw Histogramm data
  */
class HistogramWidget : public QWidget
{
    Q_OBJECT

public:

    HistogramWidget(QWidget *parent = 0);

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    void SetP(double th,double minth);
public slots:
    void setData(std::vector<std::vector<int> > hist, int maxI);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QPen pen;
    std::vector<std::vector<int> > Hist;
    int maxi;
    int fixXmin;
    int fixXmax;
    int fixYmin;
    int fixYmax;
    int fixX;
    int fixY;
    double minT;
    double fracN;
};

#endif // HISTOGRAMWIDGET_H
