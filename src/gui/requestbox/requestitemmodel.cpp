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
#include "requestitemmodel.h"

RequestItemModel::RequestItemModel(QObject *parent) :
    QStandardItemModel (parent)
{
    QStringList headerLabels = {tr("Date"), tr("SR#"), tr("Source"), tr("Target"),
                           tr("Requester"), tr("Type"), tr("State"), tr("Description")};
    setHorizontalHeaderLabels(headerLabels);
}

void RequestItemModel::appendRequest(OBSRequest *request)
{
    QString id = request->getId();

    if (!id.isEmpty() && !idList.contains(id)) {
        appendRow(requestToItems(request));
        idList.append(request->getId());
    }
}

QString RequestItemModel::getDescription(const QModelIndex &index) const
{
    QStandardItem *currentItem = item(index.row(), 7);
    return currentItem->data(Qt::UserRole).toString();
}

QString RequestItemModel::itemToString(int row, int column, int role) const
{
    QStandardItem *standardItem = item(row, column);
    return standardItem->data(role).toString();
}

OBSRequest *RequestItemModel::getRequest(const QModelIndex &index)
{
    OBSRequest *request = new OBSRequest();
    request->setDate(itemToString(index.row(), 0, Qt::DisplayRole));
    request->setId(itemToString(index.row(), 1, Qt::DisplayRole));

    QString source = itemToString(index.row(), 2, Qt::DisplayRole);
    QString target = itemToString(index.row(), 3, Qt::DisplayRole);
    QString actionType = itemToString(index.row(), 5, Qt::DisplayRole);

    if (actionType=="submit") {
        QStringList sourceList = source.split("/");
        request->setSourceProject(sourceList.at(0));
        request->setSourcePackage(sourceList.at(1));
    }

    QStringList targetList = target.split("/");
    request->setTargetProject(targetList.at(0));
    request->setTargetPackage(targetList.at(1));

    request->setRequester(itemToString(index.row(), 4, Qt::DisplayRole));
    request->setActionType(actionType);
    request->setState(itemToString(index.row(), 6, Qt::DisplayRole));
    request->setDescription(itemToString(index.row(), 7, Qt::UserRole));

    return request;
}

bool RequestItemModel::removeRequest(const QString &id)
{
    QModelIndexList itemList = match(index(0, 1),
                                            Qt::DisplayRole, QVariant::fromValue(QString(id)),
                                            1, Qt::MatchExactly);
    if (!itemList.isEmpty()) {
        auto itemIndex = itemList.at(0);
        removeRow(itemIndex.row(), itemIndex.parent());
        idList.removeOne(id);
        return true;
    }
    return false;
}

void RequestItemModel::clear()
{
    removeRows(0, rowCount());
    idList.clear();
}

void RequestItemModel::syncRequests()
{
    if (oldIdList.size()>0) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QSet<QString> currentSet = QSet<QString>(idList.begin(), idList.end());
        QSet<QString> oldSet = QSet<QString>(oldIdList.begin(), oldIdList.end());
#else
        QSet<QString> currentSet = idList.toSet();
        QSet<QString> oldSet = oldIdList.toSet();
#endif
        // For code's clarity sake (substraction is perfomed on oldSet)
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        QStringList removedRequests = oldSet.subtract(currentSet).values();
#else
        QStringList removedRequests = oldSet.subtract(currentSet).toList();
#endif
        foreach (auto requestId, removedRequests) {
            removeRequest(requestId);
        }
    }
    oldIdList = idList;
}

QList<QStandardItem *> RequestItemModel::requestToItems(OBSRequest *request)
{
    QStandardItem *dateItem = new QStandardItem();
    dateItem->setData(request->getDate(), Qt::DisplayRole);

    QStandardItem *idItem = new QStandardItem();
    idItem->setData(request->getId(), Qt::DisplayRole);

    QStandardItem *sourceItem = new QStandardItem();
    sourceItem->setData(request->getSource(), Qt::DisplayRole);

    QStandardItem *targetItem = new QStandardItem();
    targetItem->setData(request->getTarget(), Qt::DisplayRole);

    QStandardItem *requesterItem = new QStandardItem();
    requesterItem->setData(request->getRequester(), Qt::DisplayRole);

    QStandardItem *typeItem = new QStandardItem();
    typeItem->setData(request->getActionType(), Qt::DisplayRole);

    QStandardItem *stateItem = new QStandardItem();
    stateItem->setData(request->getState(), Qt::DisplayRole);

    QStandardItem *descriptionItem = new QStandardItem();
    descriptionItem->setData(request->getDescription(), Qt::UserRole);

    QList<QStandardItem *> items = {dateItem, idItem, sourceItem, targetItem,
                                    requesterItem, typeItem, stateItem, descriptionItem};
    return items;
}
