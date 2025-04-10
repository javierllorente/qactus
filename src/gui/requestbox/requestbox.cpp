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
#include "requestbox.h"
#include "ui_requestbox.h"
#include "requestviewer.h"
#include <QSettings>

RequestBox::RequestBox(QWidget *parent, OBS *obs) :
    QWidget(parent),
    ui(new Ui::RequestBox),
    m_obs(obs),
    incomingRequestsModel(new RequestItemModel(this)),
    outgoingRequestsModel(new RequestItemModel(this)),
    declinedRequestsModel(new RequestItemModel(this)),
    m_requestType(0)
{
    ui->setupUi(this);

    ui->horizontalSplitter->setSizes((QList<int>({100, 500})));

    ui->requestsWidget->setModel(incomingRequestsModel);
    ui->requestsWidget->setOBS(m_obs);
    connect(ui->requestsWidget, &RequestsWidget::descriptionFetched, this, &RequestBox::descriptionFetched);
    connect(ui->requestsWidget, &RequestsWidget::updateStatusBar, this, &RequestBox::updateStatusBar);

    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::requestTypeChanged, this, &RequestBox::requestTypeChanged);
    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::getIncomingRequests, this, &RequestBox::getIncomingRequests);
    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::getOutgoingRequests, this, &RequestBox::getOutgoingRequests);
    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::getDeclinedRequests, this, &RequestBox::getDeclinedRequests);

    connect(m_obs, &OBS::finishedParsingIncomingRequest, this, &RequestBox::addIncomingRequest);
    connect(m_obs, &OBS::finishedParsingIncomingRequestList, this, &RequestBox::incomingRequestsFetched);
    connect(m_obs, &OBS::finishedParsingOutgoingRequest, this, &RequestBox::addOutgoingRequest);
    connect(m_obs, &OBS::finishedParsingOutgoingRequestList, this, &RequestBox::orListFetched);
    connect(m_obs, &OBS::finishedParsingDeclinedRequest, this, &RequestBox::addDeclinedRequest);
    connect(m_obs, &OBS::finishedParsingDeclinedRequestList, this, &RequestBox::orListFetched);
    connect(m_obs, &OBS::finishedParsingRequestStatus, this, &RequestBox::onStatusFetched);

    readSettings();
}

RequestBox::~RequestBox()
{
    writeSettings();
    delete ui;
}

int RequestBox::getRequestType() const
{
    return m_requestType;
}

void RequestBox::readSettings()
{
    QSettings settings;
    settings.beginGroup("RequestBox");
    ui->horizontalSplitter->restoreState(settings.value("horizontalSplitterSizes").toByteArray());
    settings.endGroup();
}

void RequestBox::writeSettings()
{
    QSettings settings;
    settings.beginGroup("RequestBox");
    settings.setValue("horizontalSplitterSizes", ui->horizontalSplitter->saveState());
    settings.endGroup();
}

void RequestBox::addIncomingRequest(QSharedPointer<OBSRequest> request)
{
    qDebug() << Q_FUNC_INFO;
    incomingRequestsModel->appendRequest(request);
}

void RequestBox::incomingRequestsFetched()
{
    incomingRequestsModel->syncRequests();
    emit updateStatusBar(tr("Done"), true);
}

void RequestBox::addOutgoingRequest(QSharedPointer<OBSRequest> request)
{
    qDebug() << Q_FUNC_INFO;
    outgoingRequestsModel->appendRequest(request);
}

void RequestBox::orListFetched()
{
    outgoingRequestsModel->syncRequests();
    emit updateStatusBar(tr("Done"), true);
}

void RequestBox::addDeclinedRequest(QSharedPointer<OBSRequest> request)
{
    qDebug() << Q_FUNC_INFO;
    declinedRequestsModel->appendRequest(request);
}

void RequestBox::declinedRequestsFetched()
{
    declinedRequestsModel->syncRequests();
    emit updateStatusBar(tr("Done"), true);
}

bool RequestBox::removeIncomingRequest(const QString &id)
{
    return incomingRequestsModel->removeRequest(id);
}

bool RequestBox::removeOutgoingRequest(const QString &id)
{
    return outgoingRequestsModel->removeRequest(id);
}

bool RequestBox::removeDeclinedRequest(const QString &id)
{
    return declinedRequestsModel->removeRequest(id);
}

void RequestBox::requestTypeChanged(int index)
{
    qDebug() << Q_FUNC_INFO << index;
    m_requestType = index;
    RequestItemModel *model = nullptr;
    switch (index) {
    case 0:
        model = incomingRequestsModel;
        break;
    case 1:
        model = outgoingRequestsModel;
        break;
    case 2:
        model = declinedRequestsModel;
        break;
    }
    ui->requestsWidget->setModel(model);
}

void RequestBox::getIncomingRequests()
{
    qDebug() << Q_FUNC_INFO;
    ui->requestsWidget->clearDescription();
    m_obs->getIncomingRequests();
    emit updateStatusBar(tr("Getting incoming requests..."), false);
}

void RequestBox::getOutgoingRequests()
{
    qDebug() << Q_FUNC_INFO;
    ui->requestsWidget->clearDescription();
    m_obs->getOutgoingRequests();
    emit updateStatusBar(tr("Getting outgoing requests..."), false);
}

void RequestBox::getDeclinedRequests()
{
    qDebug() << Q_FUNC_INFO;
    ui->requestsWidget->clearDescription();
    m_obs->getDeclinedRequests();
    emit updateStatusBar(tr("Getting declined requests..."), false);
}

void RequestBox::onStatusFetched(QSharedPointer<OBSStatus> status)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (status->getCode()=="ok") {
        QSharedPointer<OBSRequest> request = ui->requestsWidget->getCurrentRequest();
        removeIncomingRequest(request->getId());
        ui->requestsWidget->clearDescription();
    }
}
