/* 
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2010-2015 Javier Llorente <javier@opensuse.org>
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
#include "obs.h"
#include "trayicon.h"
#include "configure.h"
#include "login.h"
#include "roweditor.h"
#include "requeststateeditor.h"
#include "obspackage.h"
#include "autotooltipdelegate.h"
#include "requesttreewidgetitem.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    obs = new OBS();

    createToolbar();
    trayIcon = new TrayIcon(this);
    createActions();
    createTreePackages();
    createTreeRequests();
    createStatusBar();

    loginDialog = new Login(this);
    errorBox = NULL;
    configureDialog = new Configure(this, obs);
    connect(ui->treeRequests, SIGNAL(customContextMenuRequested(const QPoint&)),this,
            SLOT(showContextMenu(const QPoint&)));
    ui->treeRequests->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(obs, SIGNAL(isAuthenticated(bool)), this, SLOT(isAuthenticated(bool)));
    connect(obs, SIGNAL(selfSignedCertificate(QNetworkReply*)),
            this, SLOT(handleSelfSignedCertificates(QNetworkReply*)));
    connect(obs, SIGNAL(networkError(QString)), this, SLOT(showNetworkError(QString)));
    connect(configureDialog, SIGNAL(apiChanged()), this, SLOT(apiChanged()));
    connect(configureDialog, SIGNAL(apiChanged()), loginDialog, SLOT(clearCredentials()));

    connect(obs, SIGNAL(finishedParsingPackage(OBSPackage*,int)),
            this, SLOT(insertBuildStatus(OBSPackage*, const int)));
    connect(obs, SIGNAL(finishedParsingRequests(QList<OBSRequest*>)),
            this, SLOT(insertRequests(QList<OBSRequest*>)));
    connect(ui->treePackages, SIGNAL(obsUrlDropped(const QStringList&)),
            this, SLOT(addDroppedUrl(const QStringList&)));

    readSettings();
    readSettingsTimer();

    QKeychain::ReadPasswordJob job(QLatin1String("Qactus"));
    job.setAutoDelete(false);
    job.setKey(loginDialog->getUsername());
    QEventLoop loop;
    job.connect(&job, SIGNAL(finished(QKeychain::Job*)), &loop, SLOT(quit()));
    job.start();
    loop.exec();
    const QString pw = job.textData();
    if (job.error()) {
        qDebug() << "Restoring password failed: " << qPrintable(job.errorString());
        loginDialog->show();
    } else {
        qDebug() << "Password restored successfully";
        obs->setCredentials(job.key(), pw);
        obs->login();
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

void MainWindow::showNetworkError(const QString &networkError)
{
    qDebug() << "showNetworkError()";

    // The QMessageBox is only displayed once if there are
    // repeated errors (queued requests, probably same error)
    if(!errorBox) {
        errorBox = new QMessageBox(this);
    } else if(!errorBox->isVisible()) {
        errorBox->setWindowTitle(tr("Network Error"));
        errorBox->setText(networkError);
        errorBox->setIcon(QMessageBox::Critical);
        int ret = errorBox->exec();
        if (ret) {
            // Not very elegant.
            // Other options: fix code, use smart pointers?
            delete errorBox;
            errorBox = NULL;
        }
    }
}

void MainWindow::handleSelfSignedCertificates(QNetworkReply *reply)
{
    QSslConfiguration sslConfig;
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
            "/data/" + QCoreApplication::applicationName();
    QDir::setCurrent(dataDir);

    QString apiUrldomain = obs->getApiUrl().section('.', -2);
    apiUrldomain.replace(".", "");
    apiUrldomain.replace("/", "");
    QString filename =  apiUrldomain + ".pem";

    if (QFile::exists(filename)) {
        qDebug() << "Reading self-signed certificate" << filename;
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QByteArray byteArray = file.readAll();
        file.close();
        QSslCertificate sslCertificate(byteArray);
        QSslSocket::addDefaultCaCertificate(sslCertificate);
        reply->ignoreSslErrors();
        return;
    } else {
        sslConfig = reply->sslConfiguration();
        QSslCertificate sslCertificate = sslConfig.peerCertificate();
//        qDebug() << sslCertificate.toText();

        QStringList certInfo = sslCertificate.subjectInfo("CN");
        QMessageBox::StandardButton result = QMessageBox::warning(this,
                                                                  QString(tr("Warning")),
                                                                  QString(tr(
                                                                              "Do you want to accept this "
                                                                              "self-signed certificate from "
                                                                              )
                                                                          + certInfo.at(0)
                                                                          ),
                                                                  QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes)  {
            qDebug() << "Saving self-signed certificate as" << filename;

            QFile file(filename);
            file.open(QIODevice::WriteOnly);
            file.write(sslCertificate.toPem());
            file.close();

            QSslSocket::addDefaultCaCertificate(sslCertificate);
            obs->request(reply->url().toString(), reply->property("row").toInt());
            reply->ignoreSslErrors();
        }
    }
}

void MainWindow::apiChanged()
{
    loginDialog->show();
}

void MainWindow::isAuthenticated(bool authenticated)
{
    if (authenticated) {
        qDebug() << "User is authenticated";
        statusBar()->showMessage(tr("Online"), 0);
    } else {
        loginDialog->show();
    }
}

void MainWindow::createToolbar()
{
    action_Add = new QAction(tr("&Add"), this);
    action_Add->setIcon(QIcon(":/icons/list-add.png"));
    action_Add->setStatusTip(tr("Add a new row"));
    action_Add->setShortcut(QKeySequence("Ctrl+A"));
    ui->toolBar->addAction(action_Add);
    connect(action_Add, SIGNAL(triggered()), this, SLOT(addRow()));

    action_Remove = new QAction(tr("&Remove"), this);
    action_Remove->setIcon(QIcon(":/icons/list-remove.png"));
    action_Remove->setStatusTip(tr("Remove selected row"));
    action_Remove->setShortcut(QKeySequence::Delete);
    ui->toolBar->addAction(action_Remove);
    connect(action_Remove, SIGNAL(triggered()), this, SLOT(removeRow()));

    action_Refresh = new QAction(tr("&Refresh"), this);
    action_Refresh->setIcon(QIcon(":/icons/view-refresh.png"));
    action_Refresh->setStatusTip(tr("Refresh view"));
    action_Refresh->setShortcut(QKeySequence::Refresh);
    ui->toolBar->addAction(action_Refresh);
    connect(action_Refresh, SIGNAL(triggered()), this, SLOT(refreshView()));

    action_MarkRead = new QAction(tr("&Mark all as read"), this);
    action_MarkRead->setIcon(QIcon(":/icons/view-task.png"));
    action_MarkRead->setStatusTip(tr("Mark all as read"));
    action_MarkRead->setShortcut(QKeySequence("Ctrl+M"));
    ui->toolBar->addAction(action_MarkRead);
    connect(action_MarkRead, SIGNAL(triggered()), this, SLOT(markAllRead()));

    ui->toolBar->addSeparator();

    action_Configure = new QAction(tr("&Configure"), this);
    action_Configure->setIcon(QIcon(":/icons/configure.png"));
    action_Configure->setStatusTip(tr("Configure Qactus"));
    ui->toolBar->addAction(action_Configure);
    connect(action_Configure, SIGNAL(triggered()), this, SLOT(on_actionConfigure_Qactus_triggered()));

}

void MainWindow::showContextMenu(const QPoint& point)
{
    QMenu *treeRequestsMenu = new QMenu(ui->treeRequests);

    QAction *actionChangeRequestState =  new QAction(tr("Change &State"), this);
    actionChangeRequestState->setIcon(QIcon(":/icons/system-switch-user.png"));
    actionChangeRequestState->setText("Change State");
    connect(actionChangeRequestState, SIGNAL(triggered()), this, SLOT(changeRequestState()));

    treeRequestsMenu->addAction(actionChangeRequestState);

    QModelIndex index = ui->treeRequests->indexAt(point);
    if (index.isValid()) {
        treeRequestsMenu->exec(ui->treeRequests->mapToGlobal(point));
    }
}

void MainWindow::changeRequestState()
{
    qDebug() << "Launching RequestStateEditor...";
    RequestStateEditor *reqStateEditor = new RequestStateEditor(this, obs);
    QTreeWidgetItem *item = ui->treeRequests->currentItem();
    qDebug() << "Request selected:" << item->text(1);
    reqStateEditor->setRequestId(item->text(1));
    reqStateEditor->setDate(item->text(0));
    reqStateEditor->setSource(item->text(2));
    reqStateEditor->setTarget(item->text(3));
    reqStateEditor->setRequester(item->text(4));

    if(item->text(5)=="submit") {
        QProgressDialog progress(tr("Getting diff..."), 0, 0, 0, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();
        QString diff = obs->getRequestDiff(item->text(2));
        reqStateEditor->setDiff(diff);
        qDebug() << "diff";
        qDebug() << diff;
    } else {
        reqStateEditor->setDiff(item->text(5) + " " + item->text(3));
    }

    reqStateEditor->exec();
    if (reqStateEditor->getResult()=="ok") {
        item->setHidden(true);
    }
    delete reqStateEditor;
}

void MainWindow::addRow()
{
    qDebug() << "Launching RowEditor...";
    RowEditor *rowEditor = new RowEditor(this, obs);

    if (rowEditor->exec()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treePackages);
        item->setText(0, rowEditor->getProject());
        item->setText(1, rowEditor->getPackage());
        item->setText(2, rowEditor->getRepository());
        item->setText(3, rowEditor->getArch());
        ui->treePackages->addTopLevelItem(item);
        int index = ui->treePackages->indexOfTopLevelItem(item);
        qDebug() << "Build" << item->text(1) << "added at" << index;
    }
    delete rowEditor;
}

void MainWindow::addDroppedUrl(const QStringList& data)
{
    QString droppedUrlDomain = data[1].section('.', -2);
    qDebug() << "droppedUrlDomain:" << droppedUrlDomain;
    qDebug() << "apiUrl:" << obs->getApiUrl();

    if (obs->isAuthenticated()) {
        if (obs->getApiUrl().endsWith(droppedUrlDomain)) {
            QString project = data[2];
            QStringList repositoryList = obs->getProjectMetadata(project);
            QString package = data[3];

            foreach (QString repository, repositoryList) {
                QStringList archList = obs->getRepositoryArchs(repository);

                foreach (QString architecture, archList) {
                    QTreeWidgetItem *item = new QTreeWidgetItem(ui->treePackages);
                    item->setText(0, project);
                    item->setText(1, package);
                    item->setText(2, repository);
                    item->setText(3, architecture);
                    ui->treePackages->addTopLevelItem(item);
                    int index = ui->treePackages->indexOfTopLevelItem(item);
                    qDebug() << "Build" << item->text(1)
                             << "(" << project << "," << repository << "," << architecture << ")"
                             << "added at" << index;
                }
            }
        } else {
            qDebug() << "apiUrl doesn't end with droppedUrlDomain!";
        }
    } else {
        qDebug() << "Not authenticated!";
    }
}

void MainWindow::editRow(QTreeWidgetItem* item, int)
{
    qDebug() << "Launching RowEditor in edit mode...";
    RowEditor *rowEditor = new RowEditor(this, obs);
    rowEditor->setProject(item->text(0));
    rowEditor->setPackage(item->text(1));
    rowEditor->setRepository(item->text(2));
    rowEditor->setArch(item->text(3));
    rowEditor->show();

    if (rowEditor->exec()) {
        int index = ui->treePackages->indexOfTopLevelItem(item);
        item->setText(0, rowEditor->getProject());
        item->setText(1, rowEditor->getPackage());
        item->setText(2, rowEditor->getRepository());
        item->setText(3, rowEditor->getArch());
        item->setText(4, "");
        ui->treePackages->insertTopLevelItem(index, item);
        qDebug() << "Build edited:" << index;
        qDebug() << "Status at" << index << item->text(4) << "(it should be empty)";
    }
    delete rowEditor;
}

void MainWindow::removeRow()
{
//    Check if the tree is not empty to avoid "deleting nothing"
    if (ui->treePackages->topLevelItemCount()!=0) {
//    Remove selected row
    QTreeWidgetItem *item = ui->treePackages->currentItem();
    int index = ui->treePackages->indexOfTopLevelItem(item);

//    Remove statusList for selected row
//    -1 means that there is no row selected
        if (index!=-1) {
            ui->treePackages->takeTopLevelItem(index);
            qDebug() << "Row removed:" << index;
        } else {
            qDebug () << "No row selected";
        }
    } else {
//        If the tree is empty, do nothing
    }    
}

void MainWindow::refreshView()
{
    qDebug() << "Refreshing view...";
    int rows = ui->treePackages->topLevelItemCount();

    for (int r=0; r<rows; r++) {
//        Ignore rows with empty cells and process rows with data
        if (!ui->treePackages->topLevelItem(r)->text(0).isEmpty() ||
                !ui->treePackages->topLevelItem(r)->text(1).isEmpty() ||
                !ui->treePackages->topLevelItem(r)->text(2).isEmpty() ||
                !ui->treePackages->topLevelItem(r)->text(3).isEmpty()) {
            QStringList tableStringList;
            tableStringList.append(QString(ui->treePackages->topLevelItem(r)->text(0)));
            tableStringList.append(QString(ui->treePackages->topLevelItem(r)->text(2)));
            tableStringList.append(QString(ui->treePackages->topLevelItem(r)->text(3)));
            tableStringList.append(QString(ui->treePackages->topLevelItem(r)->text(1)));
//            Get build status
            statusBar()->showMessage(tr("Getting build statuses..."), 5000);
            obs->getBuildStatus(tableStringList, r);
        }
    }

//    Get SRs
    statusBar()->showMessage(tr("Getting requests..."), 5000);
    obs->getRequests();
    statusBar()->showMessage(tr("Done"), 0);
}

void MainWindow::markRead(QTreeWidgetItem* item, int)
{
    qDebug() << "markRead() " << "Row: " + QString::number(ui->treePackages->indexOfTopLevelItem(item));
    for (int i=0; i<ui->treePackages->columnCount(); i++) {
        if (item->font(0).bold()) {
            setItemBoldFont(item, false);
        }
    }
    if (trayIcon->hasChangedIcon()) {
        trayIcon->normal();
    }
}

void MainWindow::markAllRead()
{
    qDebug() << "markAllRead()";
    for (int i=0; i<ui->treePackages->topLevelItemCount(); i++) {
        if (ui->treePackages->topLevelItem(i)->font(0).bold()) {
            setItemBoldFont(ui->treePackages->topLevelItem(i), false);
        }
    }
    if (trayIcon->hasChangedIcon()) {
        trayIcon->normal();
    }
}

void MainWindow::createTreePackages()
{
    ui->treePackages->setColumnCount(5);
    ui->treePackages->setColumnWidth(0, 185); // Project
    ui->treePackages->setColumnWidth(1, 160); // Package
    ui->treePackages->setColumnWidth(2, 140); // Repository
    ui->treePackages->setColumnWidth(3, 75); // Arch
    ui->treePackages->setColumnWidth(4, 100); // Status

    connect(ui->treePackages, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(editRow(QTreeWidgetItem*, int)));
    connect(ui->treePackages, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(markRead(QTreeWidgetItem*, int)));

    ui->treePackages->setItemDelegate(new AutoToolTipDelegate(ui->treePackages));
}

void MainWindow::createTreeRequests()
{
    ui->treeRequests->setColumnCount(7);
    ui->treeRequests->setColumnWidth(0, 145); // Date
    ui->treeRequests->setColumnWidth(1, 60); // SR ID
    ui->treeRequests->setColumnWidth(2, 160); // Source project
    ui->treeRequests->setColumnWidth(3, 160); // Target project
    ui->treeRequests->setColumnWidth(4, 90); // Requester
    ui->treeRequests->setColumnWidth(5, 60); // Type
    ui->treeRequests->setColumnWidth(6, 60); // State

    connect(ui->treeRequests, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(changeRequestState()));
    connect(ui->treeRequests, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(getRequestDescription(QTreeWidgetItem*, int)));

    ui->treeRequests->setItemDelegate(new AutoToolTipDelegate(ui->treeRequests));
}

void MainWindow::insertBuildStatus(OBSPackage* obsPackage, const int& row)
{
    QString details = obsPackage->getDetails();
    QString status = obsPackage->getStatus();
    delete obsPackage;

//    If the line is too long (>250), break it
    details = breakLine(details, 250);
    if (details.size()>0) {
        qDebug() << "Details string size: " << details.size();
    }

    QTreeWidgetItem *item = ui->treePackages->topLevelItem(row);
    QString oldStatus = item->text(4);
    item->setText(4, status);
    QString newStatus = item->text(4);
    item->setToolTip(4, details);
    item->setForeground(4, getColorForStatus(status));

    qDebug() << "Build status" << status << "inserted in" << row
             << "(Total rows:" << ui->treePackages->topLevelItemCount() << ")";

//    If the old status is not empty and it is different from latest one,
//    change the tray icon
    if ((oldStatus != "") && (oldStatus != newStatus)) {
        qDebug() << "Build status has changed!";
        trayIcon->notify();
        setItemBoldFont(item, true);
    }
    qDebug() << "Old status:" << oldStatus << "New status:" << newStatus;
}

QString MainWindow::breakLine(QString& details, const int& maxSize)
{
    int i = maxSize;
    if (details.size()>i) {
        for (; i<details.size(); i++) {
            if (details[i]==QChar(',') || details[i]==QChar('-') || details[i]==QChar(' ')) {
                details.insert(++i,QString("<br>"));
                break;
            }
        }
    }
    return details;
}

QColor MainWindow::getColorForStatus(const QString& status)
{
//    Change the status' colour according to the status itself
    QColor color;
    color = Qt::black;

    if(status=="succeeded")
    {
        color = Qt::darkGreen;
    }
    else if(status=="blocked")
    {
        color = Qt::gray;
    }
    else if(status=="scheduled"||status=="building")
    {
        color = Qt::darkBlue;
    }
    else if(status=="disabled")
    {
        color = Qt::gray;
    }
    else if(status=="failed")
    {
        color = Qt::red;
    }
    else if(status=="unresolvable")
    {
        color = Qt::darkRed;
    }
    else if(status.contains("unknown")||status=="404")
    {
        color = Qt::red;
    }

    return color;
}

void MainWindow::setItemBoldFont(QTreeWidgetItem *item, bool bold)
{
    QFont font = item->font(0);
    font.setBold(bold);
    for (int i=0; i<5; i++) {
        item->setFont(i, font);
    }
}

void MainWindow::insertRequests(QList<OBSRequest*> obsRequests)
{
//    If we already have inserted submit requests,
//    we remove them and insert the latest ones
    int rows = ui->treeRequests->topLevelItemCount();
    int requests = obs->getRequestCount();
    qDebug() << "InsertRequests() " << "Rows:" << rows << "Requests:" << requests;

    if (rows>0) {
        // Clears the tree widget by removing all of its items and selections.
        // Note: Each item is removed from the tree widget before being deleted.
        // http://doc.qt.io/qt-5/qtreewidget.html#clear
        ui->treeRequests->clear();
    }

    qDebug() << "RequestCount: " << obs->getRequestCount();
    qDebug() << "requests: " << requests;
    qDebug() << "obsRequests size: " << obsRequests.size();

    for (int i=0; i<obsRequests.size(); i++) {
        RequestTreeWidgetItem *item = new RequestTreeWidgetItem(ui->treeRequests);
        item->setText(0, obsRequests.at(i)->getDate());
        item->setText(1, obsRequests.at(i)->getId());
        item->setText(2, obsRequests.at(i)->getSource());
        item->setText(3, obsRequests.at(i)->getTarget());
        item->setText(4, obsRequests.at(i)->getRequester());
        item->setText(5, obsRequests.at(i)->getActionType());
        item->setText(6, obsRequests.at(i)->getState());
        item->setDescription(obsRequests.at(i)->getDescription());

        ui->treeRequests->insertTopLevelItem(i, item);
    }
    qDeleteAll(obsRequests.begin(), obsRequests.end());
    obsRequests.clear();
}

void MainWindow::getRequestDescription(QTreeWidgetItem* item, int)
{
    QString requestDescription = static_cast<RequestTreeWidgetItem*>(item)->getDescription();
    qDebug() << "getRequestDescription() " << "Row clicked: "
                + QString::number(ui->treeRequests->indexOfTopLevelItem(item));
    qDebug() << "Request description: " + requestDescription;
    ui->textBrowser->setText(requestDescription);
}

void MainWindow::pushButton_Login_clicked()
{
    obs->setCredentials(loginDialog->getUsername(), loginDialog->getPassword());

//    Display a warning if the username/password is empty.
    if (loginDialog->getUsername().isEmpty() || loginDialog->getPassword().isEmpty()) {
        QMessageBox::warning(this,tr("Error"), tr("Empty username/password"), QMessageBox::Ok );
    } else {
        loginDialog->close();
        QProgressDialog progress(tr("Logging in..."), 0, 0, 0, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();
        obs->login();

        if (loginDialog->isAutoLoginEnabled()) {
            QKeychain::WritePasswordJob job(QLatin1String("Qactus"));
            job.setAutoDelete(false);
            job.setKey(loginDialog->getUsername());
            job.setTextData(loginDialog->getPassword());
            QEventLoop loop;
            job.connect(&job, SIGNAL(finished(QKeychain::Job*)), &loop, SLOT(quit()));
            job.start();
            loop.exec();
            if (job.error()) {
                qDebug() << "Storing password failed: " << qPrintable(job.errorString());
            } else {
                qDebug() << "Password stored successfully";
            }
        } else {
            QKeychain::DeletePasswordJob job(QLatin1String("Qactus"));
            job.setAutoDelete(false);
            job.setKey(loginDialog->getUsername());
            QEventLoop loop;
            job.connect(&job, SIGNAL(finished(QKeychain::Job*)), &loop, SLOT(quit()));
            job.start();
            loop.exec();
            if (job.error()) {
                qDebug() << "Deleting password failed: " << qPrintable(job.errorString());
            }
            qDebug() << "Password deleted successfully";
        }
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

void MainWindow::about()
{
    QMessageBox::about(this,tr("About") + " " + QCoreApplication::applicationName(),
                       "<h2 align=\"left\">" + QCoreApplication::applicationName() + "</h2>" +
                       tr("A Qt-based OBS notifier") + "<br>" +
                       tr("Version:") + " " + QCoreApplication::applicationVersion() +
                       "<br><a href='https://github.com/javierllorente/qactus'>https://github.com/javierllorente/qactus</a>" +
                       "<div align=\"left\">" +
                          "<p>" +
                          "&copy; 2010-2015 Javier Llorente (Main developer)<br>"
                          "&copy; 2010-2011 Sivan Greenberg (Former contributor)  &nbsp; <br><br>"
                          + tr("Icons by the Oxygen Team") + "<br>"
                          + tr("Tray icon by the Open Build Service") + /*"<br>"*/
                          "</p>"
                          "<p>" +
                          "<b>" + tr("License") + "</b> <br>"
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
            trayIcon->normal();
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

    settings.beginGroup("Proxy");
    settings.setValue("Enabled", configureDialog->isProxyEnabled());
    settings.setValue("Type", configureDialog->getProxyType());
    settings.setValue("Server", configureDialog->getProxyServer());
    settings.setValue("Port", configureDialog->getProxyPort());
    settings.setValue("Username", configureDialog->getProxyUsername());
    settings.setValue("Password", configureDialog->getProxyPassword());
    settings.endGroup();

    settings.beginGroup("Auth");
    settings.setValue("ApiUrl", obs->getApiUrl() + "/");
    settings.setValue("Username", obs->getUsername());
    settings.setValue("AutoLogin", loginDialog->isAutoLoginEnabled());
    settings.endGroup();

    settings.beginGroup("Timer");
    settings.setValue("Active", configureDialog->isTimerActive());
    settings.setValue("Value", configureDialog->getTimerValue());
    settings.endGroup();

    int rows = ui->treePackages->topLevelItemCount();
    settings.beginWriteArray("Packages");
    settings.remove("");
    for (int i=0; i<rows; ++i)
    {
        settings.setArrayIndex(i);
//        Save settings only if all the items in a row have text
        if (!ui->treePackages->topLevelItem(i)->text(0).isEmpty() &&
                !ui->treePackages->topLevelItem(i)->text(1).isEmpty() &&
                !ui->treePackages->topLevelItem(i)->text(2).isEmpty() &&
                !ui->treePackages->topLevelItem(i)->text(3).isEmpty())
        {
            settings.setValue("Project",ui->treePackages->topLevelItem(i)->text(0));
            settings.setValue("Package",ui->treePackages->topLevelItem(i)->text(1));
            settings.setValue("Repository",ui->treePackages->topLevelItem(i)->text(2));
            settings.setValue("Arch",ui->treePackages->topLevelItem(i)->text(3));
        }
    }
    settings.endArray();
}

void MainWindow::readSettings()
{
    qDebug() << "Reading settings...";
    QSettings settings("Qactus","Qactus");
    settings.beginGroup("MainWindow");
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();

    settings.beginGroup("Proxy");
    if (settings.value("Enabled").toBool()) {
        configureDialog->setCheckedProxyCheckbox(true);
        configureDialog->setProxyType(settings.value("Type").toInt());
        configureDialog->setProxyServer(settings.value("Server").toString());
        configureDialog->setProxyPort(settings.value("Port").toInt());
        configureDialog->setProxyUsername(settings.value("Username").toString());
        configureDialog->setProxyPassword(settings.value("Password").toString());
        // FIX-ME: If proxy is enabled on a non-proxy environment you have
        // to edit Qactus.conf and set Enabled=false to get Qactus to log in
        configureDialog->toggleProxy(true);
    }
    settings.endGroup();

    settings.beginGroup("Auth");
    configureDialog->setApiUrl(settings.value("ApiUrl").toString());
    loginDialog->setUsername(settings.value("Username").toString());
    loginDialog->setAutoLoginEnabled(settings.value("AutoLogin").toBool());
    settings.endGroup();   

    int size = settings.beginReadArray("Packages");
    for (int i=0; i<size; ++i)
        {
            settings.setArrayIndex(i);
            QTreeWidgetItem *item = new QTreeWidgetItem(ui->treePackages);
            item->setText(0, settings.value("Project").toString());
            item->setText(1, settings.value("Package").toString());
            item->setText(2, settings.value("Repository").toString());
            item->setText(3, settings.value("Arch").toString());
            ui->treePackages->insertTopLevelItem(i, item);
        }
        settings.endArray();
}

void MainWindow::readSettingsTimer()
{
    QSettings settings("Qactus","Qactus");
    settings.beginGroup("Timer");
    if (settings.value("Active").toBool()) {
        qDebug() << "Timer Active = true";
        configureDialog->setCheckedTimerCheckbox(true);
        qDebug() << "Interval:" << settings.value("Value").toString() << "minutes";
        configureDialog->startTimer(settings.value("Value").toInt());
    } else {
        qDebug() << "Timer Active = false";
        configureDialog->setTimerValue(settings.value("Value").toInt());
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
    action_MarkRead->setEnabled(!index);
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
            trayIcon->normal();
        }
        break;
    default:
        break;
    }
    return QMainWindow::event(event);
}
