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
#include "requeststreewidget.h"
#include <QDateTime>
#include <QHeaderView>

RequestsTreeWidget::RequestsTreeWidget(QWidget *parent) :
    QTreeView(parent),
    itemModel(nullptr),
    firstTimeRevisionListDisplayed(true)
{
    createModel();
    initTable();
}

void RequestsTreeWidget::createModel()
{
    itemModel = new RequestItemModel(this);
    setModel(itemModel);
}

void RequestsTreeWidget::initTable()
{
    setColumnWidth(0, 145); // Date
    setColumnWidth(1, 60); // SR ID
    setColumnWidth(2, 210); // Source project
    setColumnWidth(3, 210); // Target project
    setColumnWidth(4, 100); // Requester
    setColumnWidth(5, 60); // Type
    setColumnWidth(6, 60); // State
    setColumnHidden(7, true); // Description
    setSortingEnabled(true);
    setAlternatingRowColors(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(header(), &QHeaderView::sortIndicatorChanged, this,
            [&](int logicalIndex, Qt::SortOrder order) {
        this->logicalIndex = logicalIndex;
        this->order = order;
    });

    connect(this, &RequestsTreeWidget::clicked, this, [=](const QModelIndex &index){
        RequestItemModel *currentModel = static_cast<RequestItemModel *>(model());
        QString description = currentModel->getDescription(index);
        emit descriptionFetched(description);
    });
}

void RequestsTreeWidget::deleteModel()
{
    delete itemModel;
    itemModel = nullptr;
    m_dataLoaded = false;
}

void RequestsTreeWidget::clearModel()
{
    itemModel->clear();
    selectionModel()->clearSelection();
    m_dataLoaded = false;
}

QString RequestsTreeWidget::getProject() const
{
    return project;
}

QString RequestsTreeWidget::getPackage() const
{
    return package;
}

void RequestsTreeWidget::addRequest(OBSRequest *request)
{
    itemModel->appendRequest(request);
}

void RequestsTreeWidget::requestsAdded(const QString &project, const QString &package)
{
    if (firstTimeRevisionListDisplayed) {
        model()->sort(0, Qt::DescendingOrder);
        header()->setSortIndicator(0, Qt::DescendingOrder);
        firstTimeRevisionListDisplayed = false;
    } else {
        model()->sort(logicalIndex, order);
        header()->setSortIndicator(logicalIndex, order);
    }
    logicalIndex = header()->sortIndicatorSection();
    order = header()->sortIndicatorOrder();

    selectionModel()->clear(); // Emits selectionChanged() and currentChanged()
    this->project = project;
    this->package = package;
    m_dataLoaded = true;
    emit updateStatusBar(tr("Done"), true);
}
