/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015-2019 Javier Llorente <javier@opensuse.org>
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

    setColumnCount(5);
    setColumnWidth(0, 185); // Project
    setColumnWidth(1, 160); // Package
    setColumnWidth(2, 140); // Repository
    setColumnWidth(3, 75); // Arch
    setColumnWidth(4, 100); // Status

    setItemDelegate(new AutoToolTipDelegate(this));

    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotEditRow(QTreeWidgetItem*, int)));
    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(slotMarkRead(QTreeWidgetItem*, int)));

    obs = nullptr;
    readSettings();
}

MonitorTreeWidget::~MonitorTreeWidget()
{
    writeSettings();
}

void MonitorTreeWidget::readSettings()
{
    QSettings settings;
    int size = settings.beginReadArray("Monitor");
    for (int i=0; i<size; ++i)
    {
        settings.setArrayIndex(i);
        QTreeWidgetItem *item = new QTreeWidgetItem(this);
        item->setText(0, settings.value("Project").toString());
        item->setText(1, settings.value("Package").toString());
        item->setText(2, settings.value("Repository").toString());
        item->setText(3, settings.value("Arch").toString());
        insertTopLevelItem(i, item);
    }
    settings.endArray();
}

void MonitorTreeWidget::writeSettings()
{
    QSettings settings;
    int rows = topLevelItemCount();
    settings.beginWriteArray("Monitor");
    settings.remove("");
    for (int i=0; i<rows; ++i) {
        settings.setArrayIndex(i);
//        Save settings only if all the items in a row have text
        if (!topLevelItem(i)->text(0).isEmpty() &&
                !topLevelItem(i)->text(1).isEmpty() &&
                !topLevelItem(i)->text(2).isEmpty() &&
                !topLevelItem(i)->text(3).isEmpty())
        {
            settings.setValue("Project", topLevelItem(i)->text(0));
            settings.setValue("Package", topLevelItem(i)->text(1));
            settings.setValue("Repository", topLevelItem(i)->text(2));
            settings.setValue("Arch", topLevelItem(i)->text(3));
        }
    }
    settings.endArray();
}


void MonitorTreeWidget::setOBS(OBS *obs)
{
    this->obs = obs;
    connect(obs, SIGNAL(finishedParsingPackage(OBSStatus*,int)), this, SLOT(slotInsertStatus(OBSStatus*,int)));
    connect(this, SIGNAL(obsUrlDropped(QString,QString)), obs, SLOT(getAllBuildStatus(QString,QString)));
    connect(obs, SIGNAL(finishedParsingResult(OBSResult*)), this, SLOT(addDroppedPackage(OBSResult*)));
    connect(obs, SIGNAL(finishedParsingResultList()), this, SLOT(finishedAddingPackages()));
}

void MonitorTreeWidget::getBuildStatus()
{
    qDebug() << "MonitorTreeWidget::getBuildStatus()";
    int rows = topLevelItemCount();
    for (int r=0; r<rows; r++) {
//        Ignore rows with empty cells and process rows with data
        if (!topLevelItem(r)->text(0).isEmpty() ||
                !topLevelItem(r)->text(1).isEmpty() ||
                !topLevelItem(r)->text(2).isEmpty() ||
                !topLevelItem(r)->text(3).isEmpty()) {
            QStringList tableStringList;
            tableStringList.append(QString(topLevelItem(r)->text(0)));
            tableStringList.append(QString(topLevelItem(r)->text(2)));
            tableStringList.append(QString(topLevelItem(r)->text(3)));
            tableStringList.append(QString(topLevelItem(r)->text(1)));
//            Get build status
            obs->getBuildStatus(tableStringList, r);
        }
    }
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
        delete result;
        result = nullptr;
//        FIXME: The last slot connected is in charge of deleting result

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

void MonitorTreeWidget::slotAddRow()
{
    qDebug() << "MonitorTreeWidget::slotAddRow()";
    RowEditor *rowEditor = new RowEditor(this, obs);

    if (rowEditor->exec()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(this);
        item->setText(0, rowEditor->getProject());
        item->setText(1, rowEditor->getPackage());
        item->setText(2, rowEditor->getRepository());
        item->setText(3, rowEditor->getArch());
        addTopLevelItem(item);
        int index = indexOfTopLevelItem(item);
        qDebug() << "Build" << item->text(1) << "added at" << index;
    }
    delete rowEditor;
    rowEditor = nullptr;
}

void MonitorTreeWidget::slotEditRow(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    qDebug() << "MonitorTreeWidget::slotEditRow()";
    RowEditor *rowEditor = new RowEditor(this, obs);
    rowEditor->setProject(item->text(0));
    rowEditor->setPackage(item->text(1));
    rowEditor->setRepository(item->text(2));
    rowEditor->setArch(item->text(3));
    rowEditor->show();

    if (rowEditor->exec()) {
        int index = indexOfTopLevelItem(item);
        item->setText(0, rowEditor->getProject());
        item->setText(1, rowEditor->getPackage());
        item->setText(2, rowEditor->getRepository());
        item->setText(3, rowEditor->getArch());
        item->setText(4, "");
        insertTopLevelItem(index, item);
        qDebug() << "Build edited:" << index;
        qDebug() << "Status at" << index << item->text(4) << "(it should be empty)";
    }
    delete rowEditor;
    rowEditor = nullptr;
}

void MonitorTreeWidget::slotRemoveRow()
{
    qDebug () << "MonitorTreeWidget::slotRemoveRow()";
    QList<QTreeWidgetItem *> items = selectedItems();
    QList<QModelIndex> list = selectionModel()->selectedIndexes();
    foreach (QTreeWidgetItem *item, items) {
        delete item;
        item = nullptr;
    }

    if (!list.isEmpty()) {
        QTreeWidgetItem *curItem = currentItem();
        if (curItem) {
            curItem->setSelected(true);
        }
    }
}

void MonitorTreeWidget::slotInsertStatus(OBSStatus *obsStatus, int row)
{
    qDebug() << "MonitorTreeWidget::slotInsertStatus()";
    QString details = obsStatus->getDetails();
    QString status = obsStatus->getCode();
    delete obsStatus;
    obsStatus = nullptr;

//    If the line is too long (>250), break it
    details = Utils::breakLine(details, 250);
    if (details.size()>0) {
        qDebug() << "Details string size: " << details.size();
    }

    QTreeWidgetItem *item = topLevelItem(row);
    if (item) {
        QString oldStatus = item->text(4);
        item->setText(4, status);
        if (!details.isEmpty()) {
            item->setToolTip(4, details);
        }
        item->setForeground(4, Utils::getColorForStatus(status));

        qDebug() << "Build status" << status << "inserted in" << row
                 << "(Total rows:" << topLevelItemCount() << ")";

        //    If the old status is not empty and it is different from latest one,
        //    change the tray icon and enable the "Mark all as read" button
        if (hasStatusChanged(oldStatus, status)) {
            Utils::setItemBoldFont(item, true);
        }

        if (row == topLevelItemCount()-1) {
            emit updateStatusBar(tr("Done"), true);
        }
    } else {
        emit updateStatusBar(details, true);
    }
}

bool MonitorTreeWidget::hasStatusChanged(const QString &oldStatus, const QString &newStatus)
{
    qDebug() << "MonitorTreeWidget::hasBuildStatusChanged()"
             << "Old status:" << oldStatus << "New status:" << newStatus;
    bool change = false;
    if (!oldStatus.isEmpty() && oldStatus != newStatus) {
        change = true;
        qDebug() << "MonitorTreeWidget::hasBuildStatusChanged()" << change;
        emit notifyChanged(change);
    }
    return change;
}

void MonitorTreeWidget::slotMarkRead(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    qDebug() << "MonitorTreeWidget::markRead() " << "Row: " + QString::number(indexOfTopLevelItem(item));
    for (int i=0; i<columnCount(); i++) {
        if (item->font(0).bold()) {
            Utils::setItemBoldFont(item, false);
        }
    }

    emit notifyChanged(false);
}

void MonitorTreeWidget::slotMarkAllRead()
{
    qDebug() << "MonitorTreeWidget::slotMarkAllRead()";
    for (int i=0; i<topLevelItemCount(); i++) {
        if (topLevelItem(i)->font(0).bold()) {
            Utils::setItemBoldFont(topLevelItem(i), false);
        }
    }

    emit notifyChanged(false);
}
