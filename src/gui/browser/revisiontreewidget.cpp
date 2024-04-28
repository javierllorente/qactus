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
#include "revisiontreewidget.h"
#include <QDateTime>
#include <QHeaderView>

RevisionTreeWidget::RevisionTreeWidget(QWidget *parent) :
    QTreeView(parent),
    itemModel(nullptr),
    firstTimeRevisionListDisplayed(true)
{
    createModel();
    initTable();
}

void RevisionTreeWidget::createModel()
{
    QStringList headerLabels;
    headerLabels << tr("Rev") << tr("Date") << tr("User") << tr("Comments");
    itemModel = new QStandardItemModel(this);
    itemModel->setHorizontalHeaderLabels(headerLabels);
    setModel(itemModel);
}

void RevisionTreeWidget::initTable()
{
    setColumnWidth(0, 50);
    setColumnWidth(1, 130);
    setColumnWidth(2, 140);
    setColumnWidth(3, 425);
    setSortingEnabled(true);
    setAlternatingRowColors(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(header(), &QHeaderView::sortIndicatorChanged, this,
            [&](int logicalIndex, Qt::SortOrder order) {
        this->logicalIndex = logicalIndex;
        this->order = order;
    });
}

void RevisionTreeWidget::deleteModel()
{
    delete itemModel;
    itemModel = nullptr;
}

void RevisionTreeWidget::clearModel()
{
    model()->removeRows(0, model()->rowCount());
}

QString RevisionTreeWidget::getProject() const
{
    return project;
}

QString RevisionTreeWidget::getPackage() const
{
    return package;
}

void RevisionTreeWidget::addRevision(OBSRevision *revision)
{
    QStandardItemModel *itemModel = static_cast<QStandardItemModel *>(model());
    if (itemModel) {
        itemModel->setSortRole(Qt::UserRole);

        QStandardItem *revItem = new QStandardItem();
        revItem->setData(revision->getRev(), Qt::UserRole);
        revItem->setData(revision->getRev(), Qt::DisplayRole);
        revItem->setData(Qt::AlignTop, Qt::TextAlignmentRole);

        QStandardItem *dateItem = new QStandardItem();
        QString dateStr;
        uint unixTime = revision->getTime();
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(qint64(unixTime), Qt::UTC);
        dateStr = dateTime.toString("dd/MM/yyyy H:mm");
        dateItem->setData(unixTime, Qt::UserRole);
        dateItem->setData(dateStr, Qt::DisplayRole);
        dateItem->setData(Qt::AlignTop, Qt::TextAlignmentRole);

        QStandardItem *userItem = new QStandardItem();
        userItem->setData(revision->getUser(), Qt::UserRole);
        userItem->setData(revision->getUser(), Qt::DisplayRole);
        userItem->setData(Qt::AlignTop, Qt::TextAlignmentRole);

        QStandardItem *commentsItem = new QStandardItem();
        commentsItem->setData(revision->getComment(), Qt::UserRole);
        commentsItem->setData(revision->getComment(), Qt::DisplayRole);

        QList<QStandardItem *> items;
        items << revItem << dateItem << userItem << commentsItem;
        itemModel->appendRow(items);
    }
}

void RevisionTreeWidget::revisionsAdded(const QString &project, const QString &package)
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
