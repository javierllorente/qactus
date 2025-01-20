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
#ifndef REQUESTSTREEWIDGET_H
#define REQUESTSTREEWIDGET_H

#include <QTreeView>
#include <QObject>
#include "datacontroller.h"
#include "obsrequest.h"
#include "requestitemmodel.h"

class RequestsTreeWidget : public QTreeView, public DataController
{
    Q_OBJECT
public:
    RequestsTreeWidget(QWidget *parent = nullptr);
    void createModel();
    void initTable();
    void deleteModel();
    void clearModel();
    QString getProject() const;
    QString getPackage() const;

private:
    RequestItemModel *itemModel;
    bool firstTimeRevisionListDisplayed;
    int logicalIndex;
    Qt::SortOrder order;
    QString project;
    QString package;

public slots:
    void addRequest(OBSRequest *request);
    void requestsAdded(const QString &project, const QString &package);

signals:
    void updateStatusBar(QString message, bool progressBarHidden);
    void descriptionFetched(const QString &description);
};

#endif // REQUESTSTREEWIDGET_H
