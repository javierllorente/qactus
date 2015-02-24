/* 
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2010-2015 Javier Llorente <javier@opensuse.org>
 *  Copyright (C) 2010-2011 Sivan Greenberg <sivan@omniqueue.com>
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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "trayicon.h"
#include "obsxmlreader.h"
#include "configure.h"
#include "login.h"
#include "roweditor.h"

#include "obsaccess.h"
#include "obspackage.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    obsAccess = OBSaccess::getInstance();
    obsAccess->setApiUrl("https://api.opensuse.org");
    obsPackage = new OBSpackage();

    createToolbar();
    trayIcon = new TrayIcon(this);
    createActions();
    createTable();
    createTreeRequests();
    createStatusBar();

    loginDialog = new Login(this);
    configureDialog = new Configure(this);
    ui->actionConfigure_Qactus->setEnabled(false);

    connect(obsAccess, SIGNAL(isAuthenticated(bool)), this, SLOT(enableButtons(bool)));

    readSettings();

    // Show login dialog on startup if user isn't logged in
    if(!obsAccess->isAuthenticated()) {
        // Centre login dialog
        loginDialog->move(this->geometry().center().x()-loginDialog->geometry().center().x(),
                          this->geometry().center().y()-loginDialog->geometry().center().y());
        loginDialog->show();
    }
}

MainWindow::~MainWindow()
{
    writeSettings();
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::enableButtons(bool isAuthenticated)
{
    action_Refresh->setEnabled(isAuthenticated);
    action_Timer->setEnabled(isAuthenticated);
    ui->actionConfigure_Qactus->setEnabled(isAuthenticated);

    if (isAuthenticated) {
        qDebug() << "User is authenticated";
        readSettingsTimer();
        statusBar()->showMessage(tr("Ready"), 0);
    } else {
        loginDialog->show();
    }
}

void MainWindow::createToolbar()
{
    action_Add = new QAction(tr("&Add"), this);
    action_Add->setIcon(QIcon(":/icons/list-add.png"));
    action_Add->setStatusTip(tr("Add a new row"));
    ui->toolBar->addAction(action_Add);
    connect(action_Add, SIGNAL(triggered()), this, SLOT(addRow()));

    action_Remove = new QAction(tr("&Remove"), this);
    action_Remove->setIcon(QIcon(":/icons/list-remove.png"));
    action_Remove->setStatusTip(tr("Remove selected row"));
    ui->toolBar->addAction(action_Remove);
    connect(action_Remove, SIGNAL(triggered()), this, SLOT(removeRow()));

    action_Refresh = new QAction(tr("&Refresh"), this);
    action_Refresh->setIcon(QIcon(":/icons/view-refresh.png"));
    action_Refresh->setStatusTip(tr("Refresh view"));
    action_Refresh->setEnabled(false);
    ui->toolBar->addAction(action_Refresh);
    connect(action_Refresh, SIGNAL(triggered()), this, SLOT(refreshView()));

    ui->toolBar->addSeparator();

    action_Timer = new QAction(tr("&Timer"), this);
    action_Timer->setIcon(QIcon(":/icons/chronometer.png"));
    action_Timer->setStatusTip(tr("Timer"));
    action_Timer->setEnabled(false);
    ui->toolBar->addAction(action_Timer);
    connect(action_Timer, SIGNAL(triggered()), this, SLOT(on_actionConfigure_Qactus_triggered()));

}

void MainWindow::insertRow()
{
//    Append a row
    int row = ui->table->rowCount();
    ui->table->insertRow(row);
//    Append an empty statusList
    statusList.insert(row,"");
    qDebug() << "Row appended at: " << row;
}

void MainWindow::addRow()
{
    qDebug() << "Launching RowEditor...";
    RowEditor *rowEditor = new RowEditor(this);

    if (rowEditor->exec()) {
        insertRow();
        int row = ui->table->rowCount()-1;

        QTableWidgetItem *projectItem = new QTableWidgetItem();
        projectItem->setText(rowEditor->getProject());
        ui->table->setItem(row, 0, projectItem);
        projectItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

        QTableWidgetItem *packageItem= new QTableWidgetItem();
        packageItem->setText(rowEditor->getPackage());
        ui->table->setItem(row, 1, packageItem);
        packageItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

        QTableWidgetItem *repositoryItem = new QTableWidgetItem();
        repositoryItem->setText(rowEditor->getRepository());
        ui->table->setItem(row, 2, repositoryItem);
        repositoryItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

        QTableWidgetItem *archItem = new QTableWidgetItem();
        archItem->setText(rowEditor->getArch());
        ui->table->setItem(row, 3, archItem);
        archItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    }
    delete rowEditor;
}

void MainWindow::editRow(QTableWidgetItem* item)
{
    qDebug() << "Launching RowEditor in edit mode...";
    RowEditor *rowEditor = new RowEditor(this);
    int row = item->row();
    rowEditor->setProject(ui->table->item(row,0)->text());
    rowEditor->setPackage(ui->table->item(row,1)->text());
    rowEditor->setRepository(ui->table->item(row,2)->text());
    rowEditor->setArch(ui->table->item(row,3)->text());
    rowEditor->show();

    if (rowEditor->exec()) {
        QTableWidgetItem *projectItem = new QTableWidgetItem();
        projectItem->setText(rowEditor->getProject());
        ui->table->setItem(row, 0, projectItem);
        projectItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

        QTableWidgetItem *packageItem= new QTableWidgetItem();
        packageItem->setText(rowEditor->getPackage());
        ui->table->setItem(row, 1, packageItem);
        packageItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

        QTableWidgetItem *repositoryItem = new QTableWidgetItem();
        repositoryItem->setText(rowEditor->getRepository());
        ui->table->setItem(row, 2, repositoryItem);
        repositoryItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

        QTableWidgetItem *archItem = new QTableWidgetItem();
        archItem->setText(rowEditor->getArch());
        ui->table->setItem(row, 3, archItem);
        archItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

        statusList[row].clear();
        qDebug() << "statusList editRow:" << statusList.at(row);
    }
    delete rowEditor;
}

void MainWindow::removeRow()
{
//    Check if the table is not empty
    if (ui->table->rowCount()!=0) {
//    Remove selected row
    int row = ui->table->currentRow();

//    Remove statusList for selected row
//    -1 means that there is no row selected
        if (row!=-1) {
            ui->table->removeRow(row);
            statusList.removeAt(row);
            qDebug() << "Row removed: " << row;
        } else {
            qDebug () << "No row selected";
        }
    } else {
//        If the table is empty, do nothing
    }    
}

void MainWindow::refreshView()
{
    qDebug() << "Refreshing view...";
    int rows = ui->table->rowCount();

    for (int r=0; r<rows; r++) {
//        Ignore rows with empty cells and process rows with data
        if (ui->table->item(r,0)->text().isEmpty() ||
                ui->table->item(r,1)->text().isEmpty() ||
                ui->table->item(r,2)->text().isEmpty() ||
                ui->table->item(r,3)->text().isEmpty()) {
        } else {
            QStringList tableStringList;
            tableStringList.append(QString(ui->table->item(r,0)->text()));
            tableStringList.append(QString(ui->table->item(r,2)->text()));
            tableStringList.append(QString(ui->table->item(r,3)->text()));
            tableStringList.append(QString(ui->table->item(r,1)->text()));
//            Get build status
            obsPackage = obsAccess->getBuildStatus(tableStringList);
            insertBuildStatus(obsPackage, r);
        }
    }

    if (packageErrors.size()>1) {
        QMessageBox::critical(this,tr("Error"), packageErrors, QMessageBox::Ok );
        packageErrors.clear();
    }

//    Get SRs
    obsRequests = obsAccess->getRequests();
    insertRequests(obsRequests);
}

void MainWindow::createTable()
{
    ui->table->setColumnCount(5);
    ui->table->setColumnWidth(0, 150); // Project
    ui->table->setColumnWidth(1, 150); // Package
    ui->table->setColumnWidth(2, 115); // Repository
    ui->table->setColumnWidth(3, 75); // Arch
    ui->table->setColumnWidth(4, 140); // Status

    connect(ui->table, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(editRow(QTableWidgetItem*)));

//    connect(ui->table,SIGNAL(cellChanged(int,int)), this, SLOT(on_cellChanged()));

}

void MainWindow::createTreeRequests()
{
    ui->treeRequests->setColumnCount(7);

    ui->treeRequests->setColumnWidth(0, 140); // Date
    ui->treeRequests->setColumnWidth(1, 60); // SR ID
    ui->treeRequests->setColumnWidth(2, 160); // Source project
    ui->treeRequests->setColumnWidth(3, 160); // Target project
    ui->treeRequests->setColumnWidth(4, 90); // Requester
    ui->treeRequests->setColumnWidth(5, 60); // Type
    ui->treeRequests->setColumnWidth(6, 60); // State

    connect(ui->treeRequests, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(getDescription(QTreeWidgetItem*, int)));

}

void MainWindow::insertBuildStatus(OBSpackage* obsPackage, const int& row)
{
    QString details = obsPackage->getDetails();
    QString status = obsPackage->getStatus();
    newItem = new QTableWidgetItem();
    newItem->setText(status);

//    If the line is too long (>250), break it
    details = breakLine(details, 250);

    qDebug() << "String size: " << details.size();

    newItem->setToolTip(details);
    ui->table->setItem(row, 4, newItem);
    qDebug() << "Status " << status << "inserted in" << row;
    qDebug() << "Rows: " << ui->table->rowCount();

    checkStatus(status, row);
    paintStatus(status);

}

QString MainWindow::breakLine(QString& details, const int& maxSize)
{
    int i=maxSize;
    if (details.size()>i) {
        for (; i<details.size(); i++) {
            if (details[i]==QChar(',')||details[i]==QChar('-')||details[i]==QChar(' ')) {
                details.insert(++i,QString("<br>"));
                break;
            }
        }
    }
    return details;
}

void MainWindow::checkStatus(const QString& status, const int& row)
{
//    If the statusList is not empty and the status is different from last time, change the icon
    if ((statusList.at(row) != "") && (status != statusList.at(row))) {
        qDebug() << "MainWindow::checkStatus(): Status changed!" << status << statusList.at(row) << " row: " << row;
        trayIcon->change();
    }
    qDebug() << "MainWindow::checkStatus(): Status changed?" << status << statusList.at(row) << " row: " << row;
    qDebug() << "statusList Size: " << statusList.size();

//    Insert the last status into statusList
    statusList[row] = status;
}

void MainWindow::paintStatus(const QString& status)
{
//    Change the status' colour according to the status itself
    if(status=="succeeded")
    {
        newItem->setForeground(Qt::darkGreen);
    }
    else if(status=="blocked")
    {
        newItem->setForeground(Qt::gray);
    }
    else if(status=="scheduled"||status=="building")
    {
        newItem->setForeground(Qt::darkBlue);
    }
    else if(status=="failed")
    {
        newItem->setForeground(Qt::red);
    }
    else if(status=="unresolvable")
    {
        newItem->setForeground(Qt::darkRed);
    }
//    Set the status cell to non-editable
    newItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
}

void MainWindow::insertRequests(QList<OBSrequest*> obsRequests)
{

//    If we already have inserted submit requests,
//    we remove them and insert the latest ones
    int rows = ui->treeRequests->topLevelItemCount();
    int requests = obsAccess->getRequestNumber();
    qDebug() << "InsertRequests() " << "Rows:" << rows << "Requests:" << requests;

    if (rows>0) {
        ui->treeRequests->clear();
    }

    qDebug() << "RequestNumber: " << obsAccess->getRequestNumber();
    qDebug() << "requests: " << requests;
    qDebug() << "obsRequests size: " << obsRequests.size();

    for (int i=0; i<obsRequests.size(); i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeRequests);
        item->setText(0, obsRequests.at(i)->getDate());
        item->setText(1, obsRequests.at(i)->getId());
        item->setText(2, obsRequests.at(i)->getSource());
        item->setText(3, obsRequests.at(i)->getTarget());
        item->setText(4, obsRequests.at(i)->getRequester());
        item->setText(5, obsRequests.at(i)->getActionType());
        item->setText(6, obsRequests.at(i)->getState());

        ui->treeRequests->insertTopLevelItem(i, item);
    }
}

void MainWindow::getDescription(QTreeWidgetItem* item, int)
{
    qDebug() << "getDescription() " << "Row: " + QString::number(ui->treeRequests->indexOfTopLevelItem(item));
    qDebug() << "Description: " + obsRequests.at(ui->treeRequests->indexOfTopLevelItem(item))->getDescription();
    ui->textBrowser->setText(obsRequests.at(ui->treeRequests->indexOfTopLevelItem(item))->getDescription());
}

void MainWindow::pushButton_Login_clicked()
{
    obsAccess->setCredentials(loginDialog->getUsername(), loginDialog->getPassword());

//    Display a warning if the username/password is empty.
    if (loginDialog->getUsername().isEmpty() || loginDialog->getPassword().isEmpty()) {
        QMessageBox::warning(this,tr("Error"), tr("Empty username/password"), QMessageBox::Ok );
    } else {
        loginDialog->close();
        obsAccess->login();
    }
}

void MainWindow::lineEdit_Password_returnPressed()
{
    pushButton_Login_clicked();
}

void MainWindow::on_actionQuit_triggered(bool)
{
    qApp->quit();
}

void MainWindow::on_actionAbout_triggered(bool)
{
    about();
}

//void MainWindow::on_cellChanged()
//{
//    Not working properly
//    if(ui->table->currentColumn()==3)
//    {
//        if (!ui->table->item(r,0)||!ui->table->item(r,1)||!ui->table->item(r,2)||!ui->table->item(r,3))
//        {
////                Ignore rows with empty fields
////                (and process rows with data)
//        }
//        else
//        {
//            qDebug() << "Refreshing view...";
//
//            r=ui->table->currentRow();
//
////                URL format: https://api.opensuse.org/build/KDE:Release:45/openSUSE_11.3/x86_64/ktorrent/_status
//            QUrl url = "https://api.opensuse.org/build/" + QString(ui->table->item(r,0)->text()) + "/" + QString(ui->table->item(r,2)->text()) + "/" + QString(ui->table->item(r,3)->text()) + "/" + QString(ui->table->item(r,1)->text()) + "/_status";
//
//            makeRequest(url);
//        }
//    }
//}


void MainWindow::about()
{
    QMessageBox::about(this,tr("About") + " " + QCoreApplication::applicationName(),
                       "<h2 align=\"left\">" + QCoreApplication::applicationName() + "</h2>" +
                       tr("A Qt-based OBS notifier") + "<br>" +
                       tr("Version:") + " " + QCoreApplication::applicationVersion() +
                       "<div align=\"left\">" +
                          "<p>" +
                          "<b>" + tr("Author:") + "</b><br>"
                          "Javier Llorente<br>"
                          "<a href='mailto:javier@opensuse.org'>javier@opensuse.org</a><br><br>"
                          "<b>" + tr("Contributors:") + "</b> <br>"
                          "Sivan Greenberg<br>"
                          "<a href='sivan@omniqueue.com'>sivan@omniqueue.com</a><br><br>"
                          "<b>" + tr("Artwork:") + "</b> <br>" +
                          tr("Icons by the Oxygen Team") + "<br>"
                          "<a href=\"http://www.oxygen-icons.org/\">http://www.oxygen-icons.org/</a><br><br>" +
                          tr("Tray icon by the Open Build Service") + "<br>"
                          "<a href=\"http://openbuildservice.org/\">http://openbuildservice.org/</a>"
                          "</p>"
                          "<p>" +
                          "<b>" + tr("License:") + "</b> <br>"
                          "<nobr>" + tr("This program is under the GPLv3") + "</nobr>"
                          "</p>"
                          "</div>"
                       );
}

void MainWindow::createActions()
{
    action_aboutQt = new QAction(tr("About &Qt"), this);
    connect(ui->action_aboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

//    action_Configure = new QAction(tr("&Configure"), trayIcon);
//    action_Configure->setIcon(QIcon(":/icons/configure.png"));
//    connect(action_Configure, SIGNAL(triggered()), this, SLOT(on_actionConfigure_Qactus_triggered()));
//    trayIcon->trayIconMenu->addAction(action_Configure);

    trayIcon->trayIconMenu->addSeparator();

    action_Restore = new QAction(tr("&Minimise"), trayIcon);
    connect(action_Restore, SIGNAL(triggered()), this, SLOT(toggleVisibility()));
    trayIcon->trayIconMenu->addAction(action_Restore);

    action_Quit = new QAction(tr("&Quit"), trayIcon);
    action_Quit->setIcon(QIcon(":/icons/application-exit.png"));
    connect(action_Quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    trayIcon->trayIconMenu->addAction(action_Quit);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Offline"));
}

void MainWindow::toggleVisibility()
{
    if (this->isVisible()) {
        hide();
        action_Restore->setText(tr("Restore"));
    } else {
        showNormal();
        action_Restore->setText(tr("Minimise"));
    }
}

void MainWindow::trayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason==QSystemTrayIcon::Trigger) {
        toggleVisibility();
        if (trayIcon->hasChangedIcon()) {
            trayIcon->setTrayIcon("obs.png");
        }
    }

    qDebug() << "trayicon clicked";
}

void MainWindow::writeSettings()
{
    QSettings settings("Qactus","Qactus");
    settings.beginGroup("MainWindow");
    settings.setValue("pos", pos());
    settings.endGroup();

    settings.beginGroup("Auth");
    settings.setValue("Username", obsAccess->getUsername());
    settings.endGroup();

    settings.beginGroup("Timer");
    settings.setValue("Active", configureDialog->isTimerActive());
    settings.setValue("Value", configureDialog->getTimerValue());
    settings.endGroup();

    int rows=ui->table->rowCount();
    settings.beginWriteArray("Packages");
    for (int i=0; i<rows; ++i)
    {
        settings.setArrayIndex(i);
//        Save settings only if all the cells in a row r have text
        if(ui->table->item(i,0)&&ui->table->item(i,1)&&ui->table->item(i,2)&&ui->table->item(i,3))
        {
            settings.setValue("Project",ui->table->item(i, 0)->text());
            settings.setValue("Package",ui->table->item(i, 1)->text());
            settings.setValue("Repository",ui->table->item(i, 2)->text());
            settings.setValue("Arch",ui->table->item(i, 3)->text());
        }
    }
    settings.endArray();
}

void MainWindow::readSettings()
{
    QSettings settings("Qactus","Qactus");
    settings.beginGroup("MainWindow");
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();

    settings.beginGroup("auth");
    loginDialog->setUsername(settings.value("username").toString());
    settings.endGroup();   

    int size = settings.beginReadArray("Packages");
    for (int i=0; i<size; ++i)
        {
            settings.setArrayIndex(i);
            insertRow();

            QTableWidgetItem *projectItem = new QTableWidgetItem();
            projectItem->setText(settings.value("Project").toString());
            ui->table->setItem(i, 0, projectItem);
            projectItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

            QTableWidgetItem *packageItem = new QTableWidgetItem();
            packageItem->setText(settings.value("Package").toString());
            ui->table->setItem(i, 1, packageItem);
            packageItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

            QTableWidgetItem *repositoryItem = new QTableWidgetItem();
            repositoryItem->setText(settings.value("Repository").toString());
            ui->table->setItem(i, 2, repositoryItem);
            repositoryItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

            QTableWidgetItem *archItem = new QTableWidgetItem();
            archItem->setText(settings.value("Arch").toString());
            ui->table->setItem(i, 3, archItem);
            archItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

            QTableWidgetItem *statusItem = new QTableWidgetItem();
            statusItem->setText("");
            ui->table->setItem(i, 4, statusItem);
            statusItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

        }
        settings.endArray();
}

void MainWindow::readSettingsTimer()
{
    QSettings settings("Qactus","Qactus");
    settings.beginGroup("Timer");
    if (settings.value("Active").toBool()) {
        qDebug () << "Timer Active=true";
        configureDialog->setCheckedTimerCheckbox(true);
        configureDialog->startTimer(settings.value("value").toInt()*60000);
    } else {
        qDebug () << "Timer Active=false";
        configureDialog->setTimerValue(settings.value("value").toInt());
    }
    settings.endGroup();
}

void MainWindow::on_actionConfigure_Qactus_triggered()
{
    configureDialog->show();
}

void MainWindow::on_actionLogin_triggered()
{
    loginDialog->show();
}

void MainWindow::on_tabWidget_currentChanged(const int& index)
{
    // Disable add and remove for the request tab
    action_Add->setEnabled(!index);
    action_Remove->setEnabled(!index);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    toggleVisibility();
}

bool MainWindow::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::WindowActivate:
        qDebug() << "Window activated";
        if (trayIcon->hasChangedIcon()) {
            trayIcon->setTrayIcon("obs.png");
        }
        break;
    default:
        break;
    }
    return QMainWindow::event(event);
}
