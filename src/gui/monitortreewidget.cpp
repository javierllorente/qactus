/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015-2018 Javier Llorente <javier@opensuse.org>
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

#include "monitortreewidget.h"

MonitorTreeWidget::MonitorTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    setAcceptDrops(true);
}

void MonitorTreeWidget::setOBS(OBS *obs)
{
    connect(this, SIGNAL(obsUrlDropped(QString,QString)), obs, SLOT(getAllBuildStatus(QString,QString)));
    connect(obs, SIGNAL(finishedParsingResult(OBSResult*)), this, SLOT(addDroppedPackage(OBSResult*)));
    connect(obs, SIGNAL(finishedParsingResultList()), this, SLOT(finishedAddingPackages()));
}

void MonitorTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MonitorTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void MonitorTreeWidget::dropEvent(QDropEvent *event)
{
    qDebug() << "MonitorTreeWidget::dropEvent()";
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        QString urlStr = urlList.at(0).toString();

        qDebug () << "Dropped url:" << urlStr;
        QRegExp rx("^(?:http|https)://(\\S+)/package/show/(\\S+)/(\\S+)");
        if(urlStr.contains(rx)) {
            qDebug () << "Valid OBS URL found!";
            QStringList list = rx.capturedTexts();
            droppedProject = list[2];
            droppedPackage = list[3];
            emit obsUrlDropped(droppedProject, droppedPackage);
        }
    }
}

void MonitorTreeWidget::addDroppedPackage(OBSResult *result)
{
    qDebug() << "MonitorTreeWidget::addDroppedPackage()";

    if (droppedProject==result->getProject() && droppedPackage==result->getStatus()->getPackage()) {
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
        qDebug() << "Package" << item->text(1)
                 << "(" << item->text(0) << "," << item->text(2) << "," << item->text(3) << ")"
                 << "added at" << index;
//        delete result;
//        result = nullptr;
//        The last slot connected (MainWindow::addResult()) is in charge of deleting result

    }
}

void MonitorTreeWidget::finishedAddingPackages()
{
    qDebug() << "MonitorTreeWidget::finishedAddingPackages()";
    if (!droppedProject.isEmpty() && !droppedPackage.isEmpty()) {
        droppedProject = "";
        droppedPackage = "";
    }
}

