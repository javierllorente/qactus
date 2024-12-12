/*
 * Copyright (C) 2024 Javier Llorente <javier@opensuse.org>
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
    QStringList headerLabels;
    headerLabels << tr("Created") << tr("Source") << tr("Target")
                 << tr("Requester") << tr("Action") << tr("ID");
    itemModel = new QStandardItemModel(this);
    itemModel->setHorizontalHeaderLabels(headerLabels);
    setModel(itemModel);
}

void RequestsTreeWidget::initTable()
{
    setColumnWidth(0, 145);
    setColumnWidth(1, 150);
    setColumnWidth(2, 150);
    setColumnWidth(3, 100);
    setColumnWidth(4, 75);
    setColumnWidth(5, 75);
    setSortingEnabled(true);
    setAlternatingRowColors(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(header(), &QHeaderView::sortIndicatorChanged, this,
            [&](int logicalIndex, Qt::SortOrder order) {
        this->logicalIndex = logicalIndex;
        this->order = order;
    });
}

void RequestsTreeWidget::deleteModel()
{
    delete itemModel;
    itemModel = nullptr;
}

void RequestsTreeWidget::clearModel()
{
    model()->removeRows(0, model()->rowCount());
    selectionModel()->clearSelection();
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
    QStandardItemModel *itemModel = static_cast<QStandardItemModel *>(model());
    if (itemModel) {
        itemModel->setSortRole(Qt::UserRole);

        QStandardItem *createdItem = new QStandardItem();
        QDate date = QDate::fromString(request->getDate());
        createdItem->setData(date, Qt::UserRole);
        createdItem->setData(request->getDate(), Qt::DisplayRole);

        QStandardItem *sourceItem = new QStandardItem();
        sourceItem->setData(request->getSource(), Qt::UserRole);
        sourceItem->setData(request->getSource(), Qt::DisplayRole);

        QStandardItem *targetItem = new QStandardItem();
        targetItem->setData(request->getTarget(), Qt::UserRole);
        targetItem->setData(request->getTarget(), Qt::DisplayRole);

        QStandardItem *creatorItem = new QStandardItem();
        creatorItem->setData(request->getCreator(), Qt::UserRole);
        creatorItem->setData(request->getCreator(), Qt::DisplayRole);

        QStandardItem *actionItem = new QStandardItem();
        actionItem->setData(request->getActionType(), Qt::UserRole);
        actionItem->setData(request->getActionType(), Qt::DisplayRole);

        QStandardItem *idItem = new QStandardItem();
        idItem->setData(request->getId(), Qt::UserRole);
        idItem->setData(request->getId(), Qt::DisplayRole);

        QList<QStandardItem *> items;
        items << createdItem << sourceItem << targetItem << creatorItem
              << actionItem << idItem;
        itemModel->appendRow(items);
    }
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
    emit updateStatusBar(tr("Done"), true);
}
