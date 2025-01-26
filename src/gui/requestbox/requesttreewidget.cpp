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
#include "requesttreewidget.h"

RequestTreeWidget::RequestTreeWidget(QWidget *parent) :
    QTreeView(parent),
    irModel(new RequestItemModel(this)),
    orModel(new RequestItemModel(this)),
    drModel(new RequestItemModel(this)),
    m_menu(new QMenu(this)),
    m_requestType(0)
{
    setModel(irModel);

    setColumnWidth(0, 145); // Date
    setColumnWidth(1, 60); // SR ID
    setColumnWidth(2, 210); // Source project
    setColumnWidth(3, 210); // Target project
    setColumnWidth(4, 100); // Requester
    setColumnWidth(5, 60); // Type
    setColumnWidth(6, 60); // State
    setColumnHidden(7, true); // Description

    QAction *actionChangeRequestState = new QAction(tr("Change &State"), this);
    actionChangeRequestState->setIcon(QIcon::fromTheme("mail-reply-sender"));
    actionChangeRequestState->setText("Change State");
    m_menu->addAction(actionChangeRequestState);

    connect(actionChangeRequestState, &QAction::triggered, this, &RequestTreeWidget::changeRequestState);
    connect(this, &RequestTreeWidget::doubleClicked, this, &RequestTreeWidget::changeRequestState);

    connect(this, &RequestTreeWidget::clicked, this, [=](const QModelIndex &index){
        RequestItemModel *currentModel = static_cast<RequestItemModel *>(model());
        QString description = currentModel->getDescription(index);
        emit descriptionFetched(description);
    });

    connect(this, &RequestTreeWidget::customContextMenuRequested, this, &RequestTreeWidget::slotContextMenuRequests);

    setItemDelegate(new AutoToolTipDelegate(this));
    setContextMenuPolicy(Qt::CustomContextMenu);
}

OBSRequest *RequestTreeWidget::currentRequest()
{
    RequestItemModel *currentModel = static_cast<RequestItemModel *>(model());
    return currentModel->getRequest(currentIndex());
}

int RequestTreeWidget::getRequestType() const
{
    return m_requestType;
}

void RequestTreeWidget::addIncomingRequest(QSharedPointer<OBSRequest> request)
{
    qDebug() << "RequestTreeWidget::addIncomingRequest()";
    irModel->appendRequest(request);
}

void RequestTreeWidget::irListFetched()
{
    irModel->syncRequests();
    emit updateStatusBar(tr("Done"), true);
}

void RequestTreeWidget::addOutgoingRequest(QSharedPointer<OBSRequest> request)
{
    qDebug() << "RequestTreeWidget::addOutgoingRequest()";
    orModel->appendRequest(request);
}

void RequestTreeWidget::orListFetched()
{
    orModel->syncRequests();
    emit updateStatusBar(tr("Done"), true);
}

void RequestTreeWidget::addDeclinedRequest(QSharedPointer<OBSRequest> request)
{
    qDebug() << "RequestTreeWidget::addDeclinedRequest()";
    drModel->appendRequest(request);
}

void RequestTreeWidget::drListFetched()
{
    drModel->syncRequests();
    emit updateStatusBar(tr("Done"), true);
}

bool RequestTreeWidget::removeIncomingRequest(const QString &id)
{
    return irModel->removeRequest(id);
}

bool RequestTreeWidget::removeOutgoingRequest(const QString &id)
{
    return orModel->removeRequest(id);
}

bool RequestTreeWidget::removeDeclinedRequest(const QString &id)
{
    return drModel->removeRequest(id);
}

void RequestTreeWidget::requestTypeChanged(int index)
{
    qDebug() << "RequestTreeWidget::requestTypeChanged()" << index;
    m_requestType = index;
    switch (index) {
    case 0:
        setModel(irModel);
        break;
    case 1:
        setModel(orModel);
        break;
    case 2:
        setModel(drModel);
        break;
    }
}

void RequestTreeWidget::slotContextMenuRequests(const QPoint &point)
{
    qDebug() << "RequestTreeWidget::slotContextMenuRequests()";
    QModelIndex index = indexAt(point);
    if (index.isValid()) {
        m_menu->exec(mapToGlobal(point));
    }
}
