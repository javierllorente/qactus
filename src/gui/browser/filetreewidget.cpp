/*
 * Copyright (C) 2018-2024 Javier Llorente <javier@opensuse.org>
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
#include "filetreewidget.h"
#include <QHeaderView>

FileTreeWidget::FileTreeWidget(QWidget *parent) :
    QTreeView(parent),
    firstTimeFileListDisplayed(true)
{
    createModel();
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(header(), &QHeaderView::sortIndicatorChanged, this,
            [&](int logicalIndex, Qt::SortOrder order) {
        m_logicalIndex = logicalIndex;
        m_order = order;
    });
}

void FileTreeWidget::createModel()
{
    QStringList treeFilesHeaders;
    treeFilesHeaders << tr("File name") << tr("Size") << tr("Modified time");
    sourceModelFiles = new QStandardItemModel(this);
    sourceModelFiles->setHorizontalHeaderLabels(treeFilesHeaders);
    setModel(sourceModelFiles);
    setColumnWidth(0, 250);
}

void FileTreeWidget::deleteModel()
{
    if (sourceModelFiles) {
        delete sourceModelFiles;
        sourceModelFiles = nullptr;
    }
}

void FileTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void FileTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void FileTreeWidget::dropEvent(QDropEvent *event)
{
    qDebug() << "FileTreeWidget::dropEvent()";
    QStringList pathList;

    foreach (const QUrl &url, event->mimeData()->urls()) {
        pathList.append(url.toLocalFile());
        qDebug() << "FileTreeWidget::dropEvent() Dropped file:" << pathList;
    }

    foreach (QString path, pathList) {
        emit droppedFile(path);
    }
}

void FileTreeWidget::filesAdded(const QString &project, const QString &package)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (firstTimeFileListDisplayed) {
        model()->sort(0, Qt::AscendingOrder);
        header()->setSortIndicator(0, Qt::AscendingOrder);
        firstTimeFileListDisplayed = false;
    } else {
        model()->sort(m_logicalIndex, m_order);
        header()->setSortIndicator(m_logicalIndex, m_order);
    }
    m_logicalIndex = header()->sortIndicatorSection();
    m_order = header()->sortIndicatorOrder();

    selectionModel()->clear(); // Emits selectionChanged() and currentChanged()
    this->project = project;
    this->package = package;
    emit updateStatusBar(tr("Done"), true);
}

void FileTreeWidget::addFile(OBSFile *obsFile)
{
    QStandardItemModel *fileModel = static_cast<QStandardItemModel*>(model());
    if (fileModel) {
        fileModel->setSortRole(Qt::UserRole);

        // Name
        QStandardItem *itemName = new QStandardItem();
        itemName->setData(obsFile->getName(), Qt::UserRole);
        itemName->setData(obsFile->getName(), Qt::DisplayRole);

        // Size
        QStandardItem *itemSize = new QStandardItem();
        QString fileSizeHuman;
#if QT_VERSION >= 0x051000
        QLocale locale = this->locale();
        fileSizeHuman = locale.formattedDataSize(obsFile->getSize().toInt());
#else
        fileSizeHuman = Utils::fileSizeHuman(obsFile->getSize().toInt());
#endif
        itemSize->setData(QVariant(fileSizeHuman), Qt::DisplayRole);
        itemSize->setData(obsFile->getSize().toInt(), Qt::UserRole);

        // Modified time
        QStandardItem *itemLastModified = new QStandardItem();
        QString lastModifiedStr;
        QString lastModifiedUnixTimeStr = obsFile->getLastModified();
#if QT_VERSION >= 0x050800
        QDateTime lastModifiedDateTime = QDateTime::fromSecsSinceEpoch(qint64(lastModifiedUnixTimeStr.toInt()), Qt::UTC);
        lastModifiedStr = lastModifiedDateTime.toString("dd/MM/yyyy H:mm");
#else
        lastModifiedStr = Utils::unixTimeToDate(lastModifiedUnixTimeStr);
#endif
        itemLastModified->setData(lastModifiedUnixTimeStr.toInt(), Qt::UserRole);
        itemLastModified->setData(lastModifiedStr, Qt::DisplayRole);

        QList<QStandardItem *> items;
        items << itemName << itemSize << itemLastModified;
        fileModel->appendRow(items);
    }
}

bool FileTreeWidget::hasLink() const
{
    QStandardItemModel *fileModel = static_cast<QStandardItemModel *>(model());

    if (fileModel) {
        QList<QStandardItem *> list = fileModel->findItems("_link", Qt::MatchExactly, 0);
        return !list.isEmpty();
    }

    return false;
}

QString FileTreeWidget::getCurrentFile() const
{
    QModelIndexList indexList = selectionModel()->selectedIndexes();
    return indexList.at(0).data().toString();
}

bool FileTreeWidget::removeFile(const QString &fileName)
{
    QModelIndexList itemList = model()->match(model()->index(0, 0),
                                              Qt::DisplayRole, QVariant::fromValue(QString(fileName)),
                                              1, Qt::MatchExactly);
    if (!itemList.isEmpty()) {
        auto itemIndex = itemList.at(0);
        model()->removeRow(itemIndex.row(), itemIndex.parent());
        return true;
    }
    return false;
}

void FileTreeWidget::clearModel()
{
    model()->removeRows(0, model()->rowCount());
}

QString FileTreeWidget::getProject() const
{
    return project;
}

QString FileTreeWidget::getPackage() const
{
    return package;
}
