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
#ifndef REQUESTITEMMODEL_H
#define REQUESTITEMMODEL_H

#include <QObject>
#include <QStandardItemModel>
#include <QSet>
#include <QSharedPointer>
#include "obsrequest.h"

class RequestItemModel : public QStandardItemModel
{
    Q_OBJECT

public:
    RequestItemModel(QObject *parent = nullptr);

    void appendRequest(QSharedPointer<OBSRequest> request);
    QString getDescription(const QModelIndex &index) const;
    QSharedPointer<OBSRequest> getRequest(const QModelIndex &index);
    bool removeRequest(const QString &id);
    void clear();
    void syncRequests();

private:
    QStringList idList;
    QStringList oldIdList;
    QList<QStandardItem *> requestToItems(QSharedPointer<OBSRequest> request);
    QString itemToString(int row, int column, int role) const;
};

#endif // REQUESTITEMMODEL_H
