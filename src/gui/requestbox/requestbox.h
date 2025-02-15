/*
 * Copyright (C) 2019-2025 Javier Llorente <javier@opensuse.org>
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
#ifndef REQUESTBOX_H
#define REQUESTBOX_H

#include <QWidget>
#include <QSharedPointer>
#include "obs.h"
#include "obsrequest.h"
#include "requestitemmodel.h"

namespace Ui {
class RequestBox;
}

class RequestBox : public QWidget
{
    Q_OBJECT

public:
    explicit RequestBox(QWidget *parent = nullptr, OBS *obs = nullptr);
    ~RequestBox();

    int getRequestType() const;

private:
    void readSettings();
    void writeSettings();
    Ui::RequestBox *ui;
    OBS *m_obs;
    RequestItemModel *incomingRequestsModel;
    RequestItemModel *outgoingRequestsModel;
    RequestItemModel *declinedRequestsModel;
    int m_requestType;

signals:
    void updateStatusBar(const QString &message, bool progressBarHidden);
    void descriptionFetched(const QString &description);

public slots:
    void addIncomingRequest(QSharedPointer<OBSRequest> request);
    void incomingRequestsFetched();
    void addOutgoingRequest(QSharedPointer<OBSRequest> request);
    void orListFetched();
    void addDeclinedRequest(QSharedPointer<OBSRequest> request);
    void declinedRequestsFetched();
    bool removeIncomingRequest(const QString &id);
    bool removeOutgoingRequest(const QString &id);
    bool removeDeclinedRequest(const QString &id);
    void requestTypeChanged(int index);

private slots:
    void getIncomingRequests();
    void getOutgoingRequests();
    void getDeclinedRequests();
    void onStatusFetched(QSharedPointer<OBSStatus> status);

};

#endif // REQUESTBOX_H
