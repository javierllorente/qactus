/*
 * Copyright (C) 2024-2025 Javier Llorente <javier@opensuse.org>
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
#ifndef REVISIONTREEWIDGET_H
#define REVISIONTREEWIDGET_H

#include <QTreeView>
#include <QObject>
#include <QStandardItemModel>
#include <QSharedPointer>
#include "datacontroller.h"
#include "obsrevision.h"

class RevisionTreeWidget : public QTreeView, public DataController
{
    Q_OBJECT
public:
    RevisionTreeWidget(QWidget *parent = nullptr);
    void createModel();
    void initTable();
    void deleteModel();
    void clearModel();
    QString getProject() const;
    QString getPackage() const;

private:
    QStandardItemModel *itemModel;
    bool firstTimeRevisionListDisplayed;
    int logicalIndex;
    Qt::SortOrder order;
    QString project;
    QString package;

public slots:
    void addRevision(QSharedPointer<OBSRevision> revision);
    void revisionsAdded(const QString &project, const QString &package);

signals:
    void updateStatusBar(QString message, bool progressBarHidden);
};

#endif // REVISIONTREEWIDGET_H
