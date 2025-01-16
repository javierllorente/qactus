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
#include "buildresulttreewidget.h"
#include <QHeaderView>
#include <QDebug>

BuildResultTreeWidget::BuildResultTreeWidget(QWidget *parent) :
    QTreeView(parent),
    firstTimeBuildResultsDisplayed(true)
{
    createModel();
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(header(), &QHeaderView::sortIndicatorChanged, this,
            [&](int logicalIndex, Qt::SortOrder order) {
        m_logicalIndex = logicalIndex;
        m_order = order;
    });
}

void BuildResultTreeWidget::createModel()
{
    QStringList treeBuildResultsHeaders;
    treeBuildResultsHeaders << tr("Repository") << tr("Arch") << tr("Status");
    sourceModelBuildResults = new QStandardItemModel(this);
    sourceModelBuildResults->setHorizontalHeaderLabels(treeBuildResultsHeaders);
    setModel(sourceModelBuildResults);
    setColumnWidth(0, 250);
}

void BuildResultTreeWidget::deleteModel()
{
    if (sourceModelBuildResults) {
        delete sourceModelBuildResults;
        sourceModelBuildResults = nullptr;
    }
}

void BuildResultTreeWidget::addResult(OBSResult *obsResult)
{
    QStandardItemModel *resultModel = static_cast<QStandardItemModel*>(model());

    if (resultModel) {
        QStandardItem *itemRepository = new QStandardItem(obsResult->getRepository());
        QStandardItem *itemArch = new QStandardItem(obsResult->getArch());

        if (!obsResult->getStatusList().isEmpty()) {
            QStandardItem *itemBuildResult = new QStandardItem(obsResult->getStatusList().first()->getCode());
            itemBuildResult->setForeground(Utils::getColorForStatus(itemBuildResult->text()));

            if (!obsResult->getStatusList().first()->getDetails().isEmpty()) {
                QString details = obsResult->getStatusList().first()->getDetails();
                details = Utils::breakLine(details, 250);
                itemBuildResult->setToolTip(details);
            }

            QList<QStandardItem *> items;
            items << itemRepository << itemArch << itemBuildResult;
            resultModel->appendRow(items);
        }
    }
}

bool BuildResultTreeWidget::hasSelection()
{
    QItemSelectionModel *treeBuildResultsSelectionModel = selectionModel();
    if (treeBuildResultsSelectionModel) {
        return treeBuildResultsSelectionModel->hasSelection();
    } else {
        return false;
    }
}

void BuildResultTreeWidget::clearModel()
{
    model()->removeRows(0, model()->rowCount());
    selectionModel()->clearSelection();
}

QString BuildResultTreeWidget::getCurrentRepository() const
{
    QModelIndexList indexList = selectionModel()->selectedIndexes();
    return indexList.at(0).data().toString();
}

QString BuildResultTreeWidget::getCurrentArch() const
{
    QModelIndexList indexList = selectionModel()->selectedIndexes();
    return indexList.at(1).data().toString();
}

void BuildResultTreeWidget::finishedAddingResults()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (firstTimeBuildResultsDisplayed) {
        model()->sort(0, Qt::AscendingOrder);
        header()->setSortIndicator(0, Qt::AscendingOrder);
        firstTimeBuildResultsDisplayed = false;
    } else {
        model()->sort(m_logicalIndex, m_order);
        header()->setSortIndicator(m_logicalIndex, m_order);
    }
    m_logicalIndex = header()->sortIndicatorSection();
    m_order = header()->sortIndicatorOrder();
}
