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
#include "requeststateeditor.h"
#include <QSettings>

RequestBox::RequestBox(QWidget *parent, OBS *obs) :
    QWidget(parent),
    ui(new Ui::RequestBox),
    m_obs(obs),
    irModel(new RequestItemModel(this)),
    orModel(new RequestItemModel(this)),
    drModel(new RequestItemModel(this)),
    m_requestType(0)
{
    ui->setupUi(this);

    ui->horizontalSplitter->setSizes((QList<int>({100, 500})));
    ui->verticalSplitter->setSizes((QList<int>({240, 400})));

    ui->treeRequests->setModel(irModel);
    connect(ui->treeRequests, &RequestTreeWidget::descriptionFetched, this, &RequestBox::descriptionFetched);
    connect(ui->treeRequests, &RequestTreeWidget::changeRequestState, this, &RequestBox::changeRequestState);
    connect(ui->treeRequests, &RequestTreeWidget::descriptionFetched, ui->textBrowser, &QTextBrowser::setText);
    connect(ui->treeRequests, &RequestTreeWidget::updateStatusBar, this, &RequestBox::updateStatusBar);

    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::requestTypeChanged, this, &RequestBox::requestTypeChanged);
    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::getIncomingRequests, this, &RequestBox::getIncomingRequests);
    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::getOutgoingRequests, this, &RequestBox::getOutgoingRequests);
    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::getDeclinedRequests, this, &RequestBox::getDeclinedRequests);

    connect(m_obs, &OBS::finishedParsingIncomingRequest, this, &RequestBox::addIncomingRequest);
    connect(m_obs, &OBS::finishedParsingIncomingRequestList, this, &RequestBox::irListFetched);
    connect(m_obs, &OBS::finishedParsingOutgoingRequest, this, &RequestBox::addOutgoingRequest);
    connect(m_obs, &OBS::finishedParsingOutgoingRequestList, this, &RequestBox::orListFetched);
    connect(m_obs, &OBS::finishedParsingDeclinedRequest, this, &RequestBox::addDeclinedRequest);
    connect(m_obs, &OBS::finishedParsingDeclinedRequestList, this, &RequestBox::orListFetched);
    connect(m_obs, &OBS::finishedParsingRequestStatus, this, &RequestBox::slotRequestStatusFetched);

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
    ui->verticalSplitter->restoreState(settings.value("verticalSplitterSizes").toByteArray());
    settings.endGroup();
}

void RequestBox::writeSettings()
{
    QSettings settings;
    settings.beginGroup("RequestBox");
    settings.setValue("horizontalSplitterSizes", ui->horizontalSplitter->saveState());
    settings.setValue("verticalSplitterSizes", ui->verticalSplitter->saveState());
    settings.endGroup();
}

void RequestBox::addIncomingRequest(QSharedPointer<OBSRequest> request)
{
    qDebug() << Q_FUNC_INFO;
    irModel->appendRequest(request);
}

void RequestBox::irListFetched()
{
    irModel->syncRequests();
    emit updateStatusBar(tr("Done"), true);
}

void RequestBox::addOutgoingRequest(QSharedPointer<OBSRequest> request)
{
    qDebug() << Q_FUNC_INFO;
    orModel->appendRequest(request);
}

void RequestBox::orListFetched()
{
    orModel->syncRequests();
    emit updateStatusBar(tr("Done"), true);
}

void RequestBox::addDeclinedRequest(QSharedPointer<OBSRequest> request)
{
    qDebug() << Q_FUNC_INFO;
    drModel->appendRequest(request);
}

void RequestBox::drListFetched()
{
    drModel->syncRequests();
    emit updateStatusBar(tr("Done"), true);
}

bool RequestBox::removeIncomingRequest(const QString &id)
{
    return irModel->removeRequest(id);
}

bool RequestBox::removeOutgoingRequest(const QString &id)
{
    return orModel->removeRequest(id);
}

bool RequestBox::removeDeclinedRequest(const QString &id)
{
    return drModel->removeRequest(id);
}

void RequestBox::requestTypeChanged(int index)
{
    qDebug() << Q_FUNC_INFO << index;
    m_requestType = index;
    RequestItemModel *model = nullptr;
    switch (index) {
    case 0:
        model = irModel;
        break;
    case 1:
        model = orModel;
        break;
    case 2:
        model = drModel;
        break;
    }
    ui->treeRequests->setModel(model);
}

void RequestBox::changeRequestState()
{
    qDebug() << __PRETTY_FUNCTION__;
    QSharedPointer<OBSRequest> request = ui->treeRequests->currentRequest();
    QScopedPointer<RequestStateEditor> requestStateEditor(new RequestStateEditor(this, m_obs, request));
    requestStateEditor->exec();
}

void RequestBox::getIncomingRequests()
{
    qDebug() << __PRETTY_FUNCTION__;
    ui->textBrowser->clear();
    m_obs->getIncomingRequests();
}

void RequestBox::getOutgoingRequests()
{
    qDebug() << __PRETTY_FUNCTION__;
    ui->textBrowser->clear();
    m_obs->getOutgoingRequests();
}

void RequestBox::getDeclinedRequests()
{
    qDebug() << __PRETTY_FUNCTION__;
    ui->textBrowser->clear();
    m_obs->getDeclinedRequests();
}

void RequestBox::slotRequestStatusFetched(QSharedPointer<OBSStatus> status)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (status->getCode()=="ok") {
        QSharedPointer<OBSRequest> request = ui->treeRequests->currentRequest();
        removeIncomingRequest(request->getId());
        ui->textBrowser->clear();
    }
}
