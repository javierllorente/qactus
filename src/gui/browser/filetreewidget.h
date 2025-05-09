/*
 * Copyright (C) 2018-2025 Javier Llorente <javier@opensuse.org>
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
#ifndef FILETREEWIDGET_H
#define FILETREEWIDGET_H

#include <QObject>
#include <QTreeView>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>
#include <QStandardItemModel>
#include <QSharedPointer>
#include "datacontroller.h"
#include "obsfile.h"

class FileTreeWidget : public QTreeView, public DataController
{
        Q_OBJECT

public:
    FileTreeWidget(QWidget *parent = nullptr);
    void createModel();
    void deleteModel();
    void addFile(QSharedPointer<OBSFile> file);
    bool hasLink() const;
    QString getCurrentFile() const;
    bool removeFile(const QString &fileName);
    void clearModel();
    QString getProject() const;
    QString getPackage() const;

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    QStandardItemModel *sourceModelFiles;
    bool firstTimeFileListDisplayed;
    int m_logicalIndex;
    Qt::SortOrder m_order;
    QString project;
    QString package;

public slots:
    void onFilesAdded(const QString &project, const QString &package);

signals:
    void droppedFile(QString);
    void updateStatusBar(QString message, bool progressBarHidden);

};

#endif // FILETREEWIDGET_H
