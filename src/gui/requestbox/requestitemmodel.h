/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
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

#ifndef REQUESTITEMMODEL_H
#define REQUESTITEMMODEL_H

#include <QObject>
#include <QStandardItemModel>
#include <QSet>
#include "obsrequest.h"

class RequestItemModel : public QStandardItemModel
{
    Q_OBJECT

public:
    RequestItemModel(QObject *parent = nullptr);

    void appendRequest(OBSRequest *request);
    QString getDescription(const QModelIndex &index) const;
    OBSRequest *getRequest(const QModelIndex &index);
    bool removeRequest(const QString &id);
    void syncRequests();

private:
    QStringList idList;
    QStringList oldIdList;
    QList<QStandardItem *> requestToItems(OBSRequest *request);
    QString itemToString(int row, int column, int role) const;
};

#endif // REQUESTITEMMODEL_H
