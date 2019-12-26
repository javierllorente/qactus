/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2018-2019 Javier Llorente <javier@opensuse.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BUILDRESULTTREEWIDGET_H
#define BUILDRESULTTREEWIDGET_H

#include <QObject>
#include <QTreeView>
#include <QStandardItemModel>
#include "obsresult.h"
#include "utils.h"

class BuildResultTreeWidget : public QTreeView
{
    Q_OBJECT

public:
    BuildResultTreeWidget(QWidget *parent = 0);
    void createModel();
    void deleteModel();
    void addResult(OBSResult *obsResult);
    void clearModel();
    QString getCurrentRepository() const;
    QString getCurrentArch() const;

private:
    QStandardItemModel *sourceModelBuildResults;
    bool firstTimeBuildResultsDisplayed;
    int m_logicalIndex;
    Qt::SortOrder m_order;

public slots:
    void finishedAddingResults();

};

#endif // BUILDRESULTTREEWIDGET_H
