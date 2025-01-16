/*
 * Copyright (C) 2018-2025 Javier Llorente <javier@opensuse.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
    bool hasSelection();
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
