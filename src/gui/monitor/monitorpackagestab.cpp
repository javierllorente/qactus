/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2021-2023 Javier Llorente <javier@opensuse.org>
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

#include "monitorpackagestab.h"
#include "ui_monitortab.h"

MonitorPackagesTab::MonitorPackagesTab(QWidget *parent, const QString &title, OBS *obs) :
    MonitorTab(parent, title, obs)
{
    setAcceptDrops(true);

    connect(this, &MonitorPackagesTab::obsUrlDropped, m_obs, &OBS::getAllBuildStatus);
    connect(m_obs, &OBS::finishedParsingResult, this, &MonitorPackagesTab::addDroppedPackage);

    connect(m_obs, &OBS::finishedParsingPackage, this, &MonitorPackagesTab::slotInsertStatus);
    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, &MonitorPackagesTab::slotEditRow);
    connect(m_obs, &OBS::finishedParsingResultList, this, &MonitorPackagesTab::finishedAddingPackages);
    connect(ui->treeWidget, &QTreeWidget::itemSelectionChanged, this, &MonitorPackagesTab::itemSelectionChanged);

    readSettings();
}

MonitorPackagesTab::~MonitorPackagesTab()
{
    writeSettings();
}

void MonitorPackagesTab::readSettings()
{
    QSettings settings;
    int size = settings.beginReadArray("Monitor");
    for (int i=0; i<size; ++i)
    {
        settings.setArrayIndex(i);
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, settings.value("Project").toString());
        item->setText(1, settings.value("Package").toString());
        item->setText(2, settings.value("Repository").toString());
        item->setText(3, settings.value("Arch").toString());
        ui->treeWidget->insertTopLevelItem(i, item);
    }
    settings.endArray();
}

void MonitorPackagesTab::writeSettings()
{
    QSettings settings;
    int rows = ui->treeWidget->topLevelItemCount();
    settings.beginWriteArray("Monitor");
    settings.remove("");
    for (int i=0; i<rows; ++i) {
        settings.setArrayIndex(i);
//        Save settings only if all the items in a row have text
        if (!ui->treeWidget->topLevelItem(i)->text(0).isEmpty() &&
                !ui->treeWidget->topLevelItem(i)->text(1).isEmpty() &&
                !ui->treeWidget->topLevelItem(i)->text(2).isEmpty() &&
                !ui->treeWidget->topLevelItem(i)->text(3).isEmpty())
        {
            settings.setValue("Project", ui->treeWidget->topLevelItem(i)->text(0));
            settings.setValue("Package", ui->treeWidget->topLevelItem(i)->text(1));
            settings.setValue("Repository", ui->treeWidget->topLevelItem(i)->text(2));
            settings.setValue("Arch", ui->treeWidget->topLevelItem(i)->text(3));
        }
    }
    settings.endArray();
}

void MonitorPackagesTab::refresh()
{
    qDebug() << __PRETTY_FUNCTION__;
        int rows = ui->treeWidget->topLevelItemCount();
    for (int r=0; r<rows; r++) {
//        Ignore rows with empty cells and process rows with data
        if (!ui->treeWidget->topLevelItem(r)->text(0).isEmpty() ||
                !ui->treeWidget->topLevelItem(r)->text(1).isEmpty() ||
                !ui->treeWidget->topLevelItem(r)->text(2).isEmpty() ||
                !ui->treeWidget->topLevelItem(r)->text(3).isEmpty()) {
            QStringList tableStringList;
            tableStringList.append(QString(ui->treeWidget->topLevelItem(r)->text(0)));
            tableStringList.append(QString(ui->treeWidget->topLevelItem(r)->text(2)));
            tableStringList.append(QString(ui->treeWidget->topLevelItem(r)->text(3)));
            tableStringList.append(QString(ui->treeWidget->topLevelItem(r)->text(1)));
//            Get build status
            m_obs->getBuildStatus(tableStringList, r);
        }
    }
}

bool MonitorPackagesTab::hasSelection()
{
    QItemSelectionModel *treeWidgetSelectionModel = ui->treeWidget->selectionModel();
    if (treeWidgetSelectionModel) {
        return treeWidgetSelectionModel->hasSelection();
    } else {
        return false;
    }
}

void MonitorPackagesTab::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MonitorPackagesTab::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void MonitorPackagesTab::dropEvent(QDropEvent *event)
{
    qDebug() << __PRETTY_FUNCTION__;
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        QString urlStr = urlList.at(0).toString();

        qDebug () << __PRETTY_FUNCTION__ << "Dropped url:" << urlStr;
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

void MonitorPackagesTab::addDroppedPackage(OBSResult *result)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (droppedProject==result->getProject() && droppedPackage==result->getStatus()->getPackage()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
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

        ui->treeWidget->addTopLevelItem(item);
        int index = ui->treeWidget->indexOfTopLevelItem(item);
        qDebug() << "Package" << item->text(1)
                 << "(" << item->text(0) << "," << item->text(2) << "," << item->text(3) << ")"
                 << "added at" << index;
    }
}

void MonitorPackagesTab::finishedAddingPackages()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!droppedProject.isEmpty() && !droppedPackage.isEmpty()) {
        droppedProject = "";
        droppedPackage = "";
    }
}

void MonitorPackagesTab::slotInsertStatus(OBSStatus *obsStatus, int row)
{
    qDebug() << __PRETTY_FUNCTION__;
    QString details = obsStatus->getDetails();
    QString status = obsStatus->getCode();

//    If the line is too long (>250), break it
    details = Utils::breakLine(details, 250);
    if (details.size()>0) {
        qDebug() << "Details string size: " << details.size();
    }

    QTreeWidgetItem *item = ui->treeWidget->topLevelItem(row);
    if (item) {
        QString oldStatus = item->text(4);
        item->setText(4, status);
        if (!details.isEmpty()) {
            item->setToolTip(4, details);
        }
        item->setForeground(4, Utils::getColorForStatus(status));

        qDebug() << "Build status" << status << "inserted in" << row
                 << "(Total rows:" << ui->treeWidget->topLevelItemCount() << ")";

        //    If the old status is not empty and it is different from latest one,
        //    change the tray icon and enable the "Mark all as read" button
        if (hasStatusChanged(oldStatus, status)) {
            Utils::setItemBoldFont(item, true);
        }

        if (row == ui->treeWidget->topLevelItemCount()-1) {
            emit updateStatusBar(tr("Done"), true);
        }
    } else {
        emit updateStatusBar(details, true);
    }
}

void MonitorPackagesTab::slotAddRow()
{
    qDebug() << __PRETTY_FUNCTION__;
    RowEditor *rowEditor = new RowEditor(this, m_obs);

    if (rowEditor->exec()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, rowEditor->getProject());
        item->setText(1, rowEditor->getPackage());
        item->setText(2, rowEditor->getRepository());
        item->setText(3, rowEditor->getArch());
        ui->treeWidget->addTopLevelItem(item);
        int index = ui->treeWidget->indexOfTopLevelItem(item);
        qDebug() << "Build" << item->text(1) << "added at" << index;
    }
    delete rowEditor;
    rowEditor = nullptr;
}

void MonitorPackagesTab::slotRemoveRow()
{
    qDebug () << __PRETTY_FUNCTION__;
    QList<QTreeWidgetItem *> items = ui->treeWidget->selectedItems();
    QList<QModelIndex> list = ui->treeWidget->selectionModel()->selectedIndexes();
    foreach (QTreeWidgetItem *item, items) {
        delete item;
        item = nullptr;
    }

    if (!list.isEmpty()) {
        QTreeWidgetItem *curItem = ui->treeWidget->currentItem();
        if (curItem) {
            curItem->setSelected(true);
        }
    }
}

void MonitorPackagesTab::slotEditRow(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)

    qDebug() << __PRETTY_FUNCTION__;
    RowEditor *rowEditor = new RowEditor(this, m_obs);
    rowEditor->setProject(item->text(0));
    rowEditor->setPackage(item->text(1));
    rowEditor->setRepository(item->text(2));
    rowEditor->setArch(item->text(3));
    rowEditor->show();

    if (rowEditor->exec()) {
        int index = ui->treeWidget->indexOfTopLevelItem(item);
        item->setText(0, rowEditor->getProject());
        item->setText(1, rowEditor->getPackage());
        item->setText(2, rowEditor->getRepository());
        item->setText(3, rowEditor->getArch());
        item->setText(4, "");
        ui->treeWidget->insertTopLevelItem(index, item);
        qDebug() << "Build edited:" << index;
        qDebug() << "Status at" << index << item->text(4) << "(it should be empty)";
    }
    delete rowEditor;
    rowEditor = nullptr;
}

void MonitorPackagesTab::slotMarkRead(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)

    qDebug() << __PRETTY_FUNCTION__ << "Row: " + QString::number(ui->treeWidget->indexOfTopLevelItem(item));
    for (int i=0; i<ui->treeWidget->columnCount(); i++) {
        if (item->font(0).bold()) {
            Utils::setItemBoldFont(item, false);
        }
    }

    emit notifyChanged(false);
}

