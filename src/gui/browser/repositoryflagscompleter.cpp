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
#include "repositoryflagscompleter.h"
#include <QLineEdit>
#include <QCompleter>
#include <QDebug>

RepositoryFlagsCompleter::RepositoryFlagsCompleter(QWidget *parent) :
    QStyledItemDelegate(parent)
{
    m_repositoryList = QStringList() << "all";
    m_itemCount = QList<int>() << 1;
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
    int index = m_repositoryList.indexOf(repository);
    if (index==-1) {
        m_repositoryList.append(repository);
        m_itemCount.append(1);
    } else {
        int count = m_itemCount.at(index);
        m_itemCount.replace(index, ++count);
    }
}

void RepositoryFlagsCompleter::slotItemActivated(QTreeWidgetItem *item, int column)
{
      if (column==0) {
          m_currentItem = item->text(0);
      }
}

void RepositoryFlagsCompleter::slotItemChanged(QStandardItem *item)
{
    qDebug() << __PRETTY_FUNCTION__ << item->text() << item->column();

    if (item->column()==0) {
        int index = m_repositoryList.indexOf(m_currentItem);
        int count = -1;
        if (index!=-1) {
            count = m_itemCount.at(index);
        }

        if (index>=0) {
            if (count==1) {
                m_repositoryList.replace(index, item->text());
                m_itemCount.replace(index, --count);
            }

            if (count>1) {
                m_repositoryList.append(item->text());
                m_itemCount.append(1);
                m_itemCount.replace(index, --count);
            }

        } else {
            // Not found
            m_repositoryList.append(item->text());
            m_itemCount.append(1);
        }

        m_currentItem = item->text();
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

void RepositoryFlagsCompleter::slotCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous)
    m_currentItem = current->text(0);
}

void RepositoryFlagsCompleter::slotRepositoryRemoved(const QString &repository)
{
    qDebug() << __PRETTY_FUNCTION__ << repository;

    if (!repository.isEmpty()) {
        int index = m_repositoryList.indexOf(repository);
        int count = m_itemCount.at(index);

        if (count==1) {
            m_repositoryList.removeAt(index);
            m_itemCount.removeAt(index);
        } else {
            m_itemCount.replace(index, --count);
        }
    }
}

void RepositoryFlagsCompleter::slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)
    QModelIndex index = selected.indexes().at(0);
    m_currentItem = index.data(Qt::DisplayRole).toString();
}
