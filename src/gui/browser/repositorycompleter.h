/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019-2020 Javier Llorente <javier@opensuse.org>
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
    QList<OBSDistribution *> m_distributions;
    QStringList m_repoNames;
    QStringList m_archs;
    QStringList m_path;

public slots:
    void slotItemChanged(QStandardItem *item);
    void slotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void slotFetchedDistribution(OBSDistribution *distribution);
};

#endif // REPOSITORYCOMPLETER_H
