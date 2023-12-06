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
#include "repositorycompleter.h"
#include <QLineEdit>
#include <QCompleter>
#include <QDebug>

RepositoryCompleter::RepositoryCompleter(QWidget *parent) :
    QStyledItemDelegate(parent)
{

}

QWidget *RepositoryCompleter::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    QString field = index.model()->headerData(index.column(), Qt::Horizontal).toString();
    qDebug() << __PRETTY_FUNCTION__ << field;

    QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(false);
    QCompleter *completer = nullptr;

    if (field == "Repository") {
        completer = new QCompleter(m_repoNames, parent);
    }

    if (field == "Arch") {
        completer = new QCompleter(m_archs, parent);
    }

    if (field == "Path") {
        completer = new QCompleter(m_path, parent);
    }

    if (completer) {
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        editor->setCompleter(completer);
    }

    return editor;
}

void RepositoryCompleter::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}

void RepositoryCompleter::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

void RepositoryCompleter::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}

void RepositoryCompleter::slotItemChanged(QStandardItem *item)
{
    qDebug() << __PRETTY_FUNCTION__ << item->text();

    if (item->column()==0) {
        int index = m_repoNames.indexOf(item->text());
        if (index!=-1) {
            m_archs = QStringList() << m_distributions.at(index)->getArchs();
            m_path = QStringList() << QString("%1/%2").arg(m_distributions.at(index)->getRepoName(),
                                                           m_distributions.at(index)->getRepository());
        }
    }
}

void RepositoryCompleter::slotCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)
    qDebug() << __PRETTY_FUNCTION__ << current.data(Qt::DisplayRole).toString() << current.column();

    if (current.column()==1) {
        m_archs.clear();
    }

    if (current.column()==2) {
        m_path.clear();
    }
}

void RepositoryCompleter::slotFetchedDistribution(OBSDistribution *distribution)
{
    m_distributions.append(distribution);
    m_repoNames.append(distribution->getRepoName());
}
