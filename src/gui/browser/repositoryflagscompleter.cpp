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

#include "repositoryflagscompleter.h"
#include <QLineEdit>
#include <QCompleter>
#include <QDebug>

RepositoryFlagsCompleter::RepositoryFlagsCompleter(QWidget *parent) :
    QStyledItemDelegate(parent)
{
    m_repositoryList = QStringList() << "all";
}

QWidget *RepositoryFlagsCompleter::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    QString field = index.model()->headerData(index.column(), Qt::Horizontal).toString();
    qDebug() << __PRETTY_FUNCTION__ << field;

    QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(false);
    QCompleter *completer = nullptr;

    if (field.endsWith("repository")) {
        completer = new QCompleter(m_repositoryList, parent);
    }

    if (field == "Enabled") {
        QStringList booleanList = QStringList() << "true" << "false";
        completer = new QCompleter(booleanList, parent);
    }

    if (completer) {
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        editor->setCompleter(completer);
    }

    return editor;
}

void RepositoryFlagsCompleter::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}

void RepositoryFlagsCompleter::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

void RepositoryFlagsCompleter::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}

void RepositoryFlagsCompleter::appendRepository(const QString &repository)
{
    m_repositoryList.append(repository);
}

void RepositoryFlagsCompleter::slotItemActivated(QTreeWidgetItem *item, int column)
{
      if (column==0) {
          m_currentItem = item->text(0);
      }
}

void RepositoryFlagsCompleter::slotItemChanged(QTreeWidgetItem *item, int column)
{
    qDebug() << __PRETTY_FUNCTION__ << item->text(0);
    if (column==0) {
        int index = m_repositoryList.indexOf(m_currentItem);
        if (index!=-1) {
            m_repositoryList.replace(index, item->text(0));
        } else {
            m_repositoryList.append(item->text(0));
        }
    }
}

void RepositoryFlagsCompleter::slotCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)
    qDebug() << __PRETTY_FUNCTION__ << current.data(Qt::DisplayRole).toString() << current.column();

    if (current.column()==0) {
        m_currentItem = current.data(Qt::DisplayRole).toString();
    }
}

void RepositoryFlagsCompleter::slotRowsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(last)
    qDebug() << __PRETTY_FUNCTION__ << first;
    m_repositoryList.removeAt(first+1); // +1 for "all"
}
