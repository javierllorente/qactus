/*
 * Copyright (C) 2025 Javier Llorente <javier@opensuse.org>
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
#ifndef REQUESTSWIDGET_H
#define REQUESTSWIDGET_H

#include <QWidget>
#include <QSharedPointer>
#include "datacontroller.h"
#include "requestitemmodel.h"
#include "obsrequest.h"
#include "obs.h"

namespace Ui {
class RequestsWidget;
}

class RequestsWidget : public QWidget, public DataController
{
    Q_OBJECT

public:
    explicit RequestsWidget(QWidget *parent = nullptr);
    ~RequestsWidget();
    void setModel(QAbstractItemModel *model);
    void setOBS(OBS *obs);
    QSharedPointer<OBSRequest> getCurrentRequest();
    void clearModel();
    void clearDescription();


private:
    void readSettings();
    void writeSettings();
    Ui::RequestsWidget *ui;
    RequestItemModel *itemModel;
    OBS *obs;
    bool firstTimeRevisionListDisplayed;
    int logicalIndex;
    Qt::SortOrder order;
    QString project;
    QString package;

signals:
    void updateStatusBar(const QString &message, bool progressBarHidden);
    void descriptionFetched(const QString &description);

public slots:
    void addRequest(QSharedPointer<OBSRequest> request);
    void requestsAdded(const QString &project, const QString &package);

private slots:
    void changeRequestState();

};

#endif // REQUESTSWIDGET_H
