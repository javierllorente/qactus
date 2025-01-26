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
#ifndef REQUESTTREEWIDGET_H
#define REQUESTTREEWIDGET_H

#include <QObject>
#include <QTreeView>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QSharedPointer>
#include "obsrequest.h"
#include "requestitemmodel.h"
#include "requeststateeditor.h"
#include "autotooltipdelegate.h"

class RequestTreeWidget : public QTreeView
{
    Q_OBJECT

public:
    explicit RequestTreeWidget(QWidget *parent = nullptr);
    OBSRequest *currentRequest();
    int getRequestType() const;

signals:
    void updateStatusBar(const QString &message, bool progressBarHidden);
    void descriptionFetched(const QString &description);
    void changeRequestState();

public slots:
    void addIncomingRequest(QSharedPointer<OBSRequest> request);
    void irListFetched();
    void addOutgoingRequest(QSharedPointer<OBSRequest> request);
    void orListFetched();
    void addDeclinedRequest(QSharedPointer<OBSRequest> request);
    void drListFetched();
    bool removeIncomingRequest(const QString &id);
    bool removeOutgoingRequest(const QString &id);
    bool removeDeclinedRequest(const QString &id);
    void requestTypeChanged(int index);

private:
     RequestItemModel *irModel;
     RequestItemModel *orModel;
     RequestItemModel *drModel;
     QMenu *m_menu;
     int m_requestType;

private slots:
    void slotContextMenuRequests(const QPoint &point);

};

#endif // REQUESTTREEWIDGET_H
