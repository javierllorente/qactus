/*
 *  Qactus - A Qt-based OBS client
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

#include "requesttreewidget.h"

RequestTreeWidget::RequestTreeWidget(QWidget *parent) :
    QTreeView(parent),
    irModel(new RequestItemModel(this)),
    orModel(new RequestItemModel(this)),
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

void RequestTreeWidget::addIncomingRequest(OBSRequest *request)
{
    qDebug() << "RequestTreeWidget::addIncomingRequest()";
    irModel->appendRequest(request);
    delete request;
}

void RequestTreeWidget::irListFetched()
{
    irModel->syncRequests();
    emit updateStatusBar(tr("Done"), true);
}

void RequestTreeWidget::addOutgoingRequest(OBSRequest *request)
{
    qDebug() << "RequestTreeWidget::addOutgoingRequest()";
    orModel->appendRequest(request);
    delete request;
}

void RequestTreeWidget::orListFetched()
{
    orModel->syncRequests();
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
