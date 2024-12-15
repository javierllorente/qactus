/*
 * Copyright (C) 2019-2024 Javier Llorente <javier@opensuse.org>
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
#ifndef PACKAGELISTMODEL_H
#define PACKAGELISTMODEL_H

#include <QAbstractListModel>
#include <QStringList>

class PackageListModel : public QAbstractListModel
{
public:
    PackageListModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void addPackage(const QString &package);
    void addPackageList(const QStringList &packages);
    bool removePackage(const QString &package);
    void clear();
    QStringList stringList() const;

private:
    QStringList m_packages;
};

#endif // PACKAGELISTMODEL_H
