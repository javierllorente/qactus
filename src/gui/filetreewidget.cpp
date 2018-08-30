/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2018 Javier Llorente <javier@opensuse.org>
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

#include "filetreewidget.h"

FileTreeWidget::FileTreeWidget(QWidget *parent) :
    QTreeView(parent)
{
    createModel();
    setContextMenuPolicy(Qt::CustomContextMenu);
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

QString FileTreeWidget::getCurrentFile() const
{
    return currentIndex().data().toString();
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
