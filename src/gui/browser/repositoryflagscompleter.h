/*
 * Copyright (C) 2019-2020 Javier Llorente <javier@opensuse.org>
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
#ifndef REPOSITORYFLAGSCOMPLETER_H
#define REPOSITORYFLAGSCOMPLETER_H

#include <QStyledItemDelegate>
#include <QTreeWidgetItem>
#include <QStandardItem>
#include <QList>
#include "obsdistribution.h"

class RepositoryFlagsCompleter : public QStyledItemDelegate
{
    Q_OBJECT

public:
    RepositoryFlagsCompleter(QWidget *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void appendRepository(const QString &repository);

private:
    QStringList m_repositoryList;
    QList<int> m_itemCount;
    QString m_currentItem;

public slots:
    void slotItemActivated(QTreeWidgetItem *item, int column);
    void slotItemChanged(QStandardItem *item);
    void slotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void slotCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void slotRepositoryRemoved(const QString &repository);
    void slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
};

#endif // REPOSITORYFLAGSCOMPLETER_H
