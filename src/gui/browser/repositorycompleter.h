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
#ifndef REPOSITORYCOMPLETER_H
#define REPOSITORYCOMPLETER_H

#include <QStyledItemDelegate>
#include <QTreeWidgetItem>
#include <QList>
#include <QStandardItem>
#include "obsdistribution.h"

class RepositoryCompleter : public QStyledItemDelegate
{
    Q_OBJECT

public:
    RepositoryCompleter(QWidget *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

private:
    QList<QSharedPointer<OBSDistribution>> m_distributions;
    QStringList m_repoNames;
    QStringList m_archs;
    QStringList m_path;

public slots:
    void slotItemChanged(QStandardItem *item);
    void slotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void slotFetchedDistribution(QSharedPointer<OBSDistribution> distribution);
};

#endif // REPOSITORYCOMPLETER_H
