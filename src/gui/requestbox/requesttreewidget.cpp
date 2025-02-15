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
#include "requestitemmodel.h"
#include "autotooltipdelegate.h"

RequestTreeWidget::RequestTreeWidget(QWidget *parent) :
    QTreeView(parent),
    m_menu(new QMenu(this))
{
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

    connect(this, &RequestTreeWidget::customContextMenuRequested, this, &RequestTreeWidget::onContextMenuRequested);

    setItemDelegate(new AutoToolTipDelegate(this));
    setContextMenuPolicy(Qt::CustomContextMenu);
}

QSharedPointer<OBSRequest> RequestTreeWidget::getCurrentRequest()
{
    RequestItemModel *currentModel = static_cast<RequestItemModel *>(model());
    return currentModel->getRequest(currentIndex());
}

void RequestTreeWidget::onContextMenuRequested(const QPoint &point)
{
    QModelIndex index = indexAt(point);
    if (index.isValid()) {
        m_menu->exec(mapToGlobal(point));
    }
}
