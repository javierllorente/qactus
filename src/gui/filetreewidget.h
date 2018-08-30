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

#ifndef FILETREEWIDGET_H
#define FILETREEWIDGET_H

#include <QObject>
#include <QTreeView>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>
#include <QStandardItemModel>
#include "obs.h"
#include "utils.h"

class FileTreeWidget : public QTreeView
{
        Q_OBJECT

public:
    FileTreeWidget(QWidget *parent = 0);
    void createModel();
    void deleteModel();
    void addFile(OBSFile *obsFile);
    QString getCurrentFile() const;
    bool removeFile(const QString &fileName);
    void clearModel();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    QStandardItemModel *sourceModelFiles;

signals:
    void droppedFile(QString);

};

#endif // FILETREEWIDGET_H
