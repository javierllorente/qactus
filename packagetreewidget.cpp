/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015 Javier Llorente <javier@opensuse.org>
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

#include "packagetreewidget.h"

PackageTreeWidget::PackageTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    setAcceptDrops(true);
}

void PackageTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void PackageTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void PackageTreeWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        QString urlStr = urlList.at(0).toString();

        qDebug () << "Dropped url:" << urlStr;
        QRegExp rx("^(?:http|https)://(\\S+)/package/show/(\\S+)/(\\S+)");
        if(urlStr.contains(rx)) {
            qDebug () << "Valid OBS URL found!";
            QStringList list = rx.capturedTexts();
            emit obsUrlDropped(list);
        }
    }


}
