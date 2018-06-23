/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015-2016 Javier Llorente <javier@opensuse.org>
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

void PackageTreeWidget::setOBS(OBS *obs)
{
    connect(this, SIGNAL(obsUrlDropped(QString,QString)), obs, SLOT(getAllBuildStatus(QString,QString)));
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
    qDebug() << "PackageTreeWidget::dropEvent()";
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        QString urlStr = urlList.at(0).toString();

        qDebug () << "Dropped url:" << urlStr;
        QRegExp rx("^(?:http|https)://(\\S+)/package/show/(\\S+)/(\\S+)");
        if(urlStr.contains(rx)) {
            qDebug () << "Valid OBS URL found!";
            QStringList list = rx.capturedTexts();
            QString project = list[2];
            QString package = list[3];
            emit obsUrlDropped(project, package);
        }
    }
}

void PackageTreeWidget::insertDroppedPackage(OBSResult *result)
{
    qDebug() << "PackageTreeWidget::insertDroppedPackage()";

    QTreeWidgetItem *item = new QTreeWidgetItem(this);
    item->setText(0, result->getProject());
    item->setText(1, result->getStatus()->getPackage());
    item->setText(2, result->getRepository());
    item->setText(3, result->getArch());
    QString status = result->getStatus()->getCode();
    item->setText(4, status);
    if (!result->getStatus()->getDetails().isEmpty()) {
        QString details = result->getStatus()->getDetails();
        details = Utils::breakLine(details, 250);
        item->setToolTip(4, details);
    }
    item->setForeground(4, Utils::getColorForStatus(status));

    addTopLevelItem(item);
    int index = indexOfTopLevelItem(item);
    qDebug() << "Build" << item->text(1)
             << "(" << item->text(0) << "," << item->text(2) << "," << item->text(3) << ")"
             << "added at" << index;
}

