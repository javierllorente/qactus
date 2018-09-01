/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2018 Javier Llorente <javier@opensuse.org>
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

#include "buildresulttreewidget.h"

BuildResultTreeWidget::BuildResultTreeWidget(QWidget *parent) :
    QTreeView(parent)
{
    createModel();
    setContextMenuPolicy(Qt::CustomContextMenu);
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
        QStandardItem *itemBuildResult = new QStandardItem(obsResult->getStatus()->getCode());
        itemBuildResult->setForeground(Utils::getColorForStatus(itemBuildResult->text()));

        if (!obsResult->getStatus()->getDetails().isEmpty()) {
            QString details = obsResult->getStatus()->getDetails();
            details = Utils::breakLine(details, 250);
            itemBuildResult->setToolTip(details);
        }

        QList<QStandardItem *> items;
        items << itemRepository << itemArch << itemBuildResult;
        resultModel->appendRow(items);
    }
}

void BuildResultTreeWidget::clearModel()
{
    model()->removeRows(0, model()->rowCount());
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
