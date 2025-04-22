/*
 * Copyright (C) 2021-2025 Javier Llorente <javier@opensuse.org>
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
#include "monitorpackagestab.h"
#include "ui_monitortab.h"

MonitorPackagesTab::MonitorPackagesTab(QWidget *parent, const QString &title, OBS *obs) :
    MonitorTab(parent, title, obs)
{
    setAcceptDrops(true);

    connect(this, &MonitorPackagesTab::obsUrlDropped, m_obs, &OBS::getPackageResults);
    connect(m_obs, &OBS::finishedParsingResult, this, &MonitorPackagesTab::addDroppedPackage);

    connect(m_obs, &OBS::finishedParsingPackage, this, &MonitorPackagesTab::insertStatus);
    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, &MonitorPackagesTab::editRow);
    connect(m_obs, &OBS::finishedParsingResultList, this, &MonitorPackagesTab::onPackagesAdded);
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
            emit updateStatusBar(tr("Getting build results..."), false);
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

bool MonitorPackagesTab::contains(const QString &project, const QString &package)
{
    int rows = ui->treeWidget->topLevelItemCount();
    for (int i = 0; i < rows; i++) {
        if (ui->treeWidget->topLevelItem(i)->text(0) == project
            && ui->treeWidget->topLevelItem(i)->text(1) == package) {
            return true;
        }
    }

    return false;
}

void MonitorPackagesTab::addPackage(const QString &package, const QList<OBSResult> &builds)
{
    foreach (auto build, builds) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, build.getProject());
        item->setText(1, package);
        item->setText(2, build.getRepository());
        item->setText(3, build.getArch());
        ui->treeWidget->addTopLevelItem(item);
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
        QRegularExpression rx("^(?:http|https)://(\\S+)/package/show/(\\S+)/(\\S+)");
        if(urlStr.contains(rx)) {
            qDebug () << "Valid OBS URL found!";
            QStringList list = rx.namedCaptureGroups();
            droppedProject = list[2];
            droppedPackage = list[3];
            emit obsUrlDropped(droppedProject, droppedPackage);
            emit updateStatusBar(tr("Get build results..."), false);
        }
    }
}

void MonitorPackagesTab::addDroppedPackage(QSharedPointer<OBSResult> result)
{
    qDebug() << Q_FUNC_INFO;

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
        qDebug() << Q_FUNC_INFO << "Package" << item->text(1)
                 << "(" << item->text(0) << "," << item->text(2) << "," << item->text(3) << ")"
                 << "added at" << index;
        emit updateStatusBar(tr("Done"), true);
    }
}

void MonitorPackagesTab::onPackagesAdded()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!droppedProject.isEmpty() && !droppedPackage.isEmpty()) {
        droppedProject = "";
        droppedPackage = "";
    }
}

void MonitorPackagesTab::insertStatus(QSharedPointer<OBSStatus> status, int row)
{
    qDebug() << __PRETTY_FUNCTION__;
    QString details = status->getDetails();
    QString code = status->getCode();

//    If the line is too long (>250), break it
    details = Utils::breakLine(details, 250);
    if (details.size()>0) {
        qDebug() << "Details string size: " << details.size();
    }

    QTreeWidgetItem *item = ui->treeWidget->topLevelItem(row);
    if (item) {
        QString oldCode = item->text(4);
        item->setText(4, code);
        if (!details.isEmpty()) {
            item->setToolTip(4, details);
        }
        item->setForeground(4, Utils::getColorForStatus(code));

        qDebug() << "Build status" << code << "inserted in" << row
                 << "(Total rows:" << ui->treeWidget->topLevelItemCount() << ")";

        //    If the old status is not empty and it is different from latest one,
        //    change the tray icon and enable the "Mark all as read" button
        if (hasStatusChanged(oldCode, code)) {
            Utils::setItemBoldFont(item, true);
        }

        if (row == ui->treeWidget->topLevelItemCount()-1) {
            emit updateStatusBar(tr("Done"), true);
        }
    } else {
        emit updateStatusBar(details, true);
    }
}

void MonitorPackagesTab::addRow()
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

void MonitorPackagesTab::removeRow()
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

void MonitorPackagesTab::editRow(QTreeWidgetItem *item, int column)
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

