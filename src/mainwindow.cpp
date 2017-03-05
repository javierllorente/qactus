/* 
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2010-2017 Javier Llorente <javier@opensuse.org>
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

    credentials = new Credentials(this);
    obs = new OBS();

    createToolbar();
    trayIcon = new TrayIcon(this);
    m_notify = false;
    createActions();
    createTreePackages();
    createTreeRequests();
    createStatusBar();

    loginDialog = new Login(this);
    errorBox = NULL;
    configureDialog = new Configure(this, obs);
    ui->hSplitterBrowser->setStretchFactor(1, 1);
    ui->hSplitterBrowser->setStretchFactor(0, 0);
    connect(ui->treeRequests, SIGNAL(customContextMenuRequested(const QPoint&)),this,
            SLOT(showContextMenu(const QPoint&)));
    ui->treeRequests->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(credentials, SIGNAL(errorReadingPassword(QString)),
            this, SLOT(errorReadingPasswordSlot(QString)));
    connect(credentials, SIGNAL(credentialsRestored(QString,QString)),
            this, SLOT(credentialsRestoredSlot(QString,QString)));
    connect(obs, SIGNAL(isAuthenticated(bool)), this, SLOT(isAuthenticated(bool)));
    connect(obs, SIGNAL(selfSignedCertificate(QNetworkReply*)),
            this, SLOT(handleSelfSignedCertificates(QNetworkReply*)));
    connect(obs, SIGNAL(networkError(QString)), this, SLOT(showNetworkError(QString)));
    connect(configureDialog, SIGNAL(apiChanged()), this, SLOT(apiChanged()));
    connect(configureDialog, SIGNAL(apiChanged()), loginDialog, SLOT(clearCredentials()));

    connect(obs, SIGNAL(projectListIsReady()), this, SLOT(insertProjectList()));
    connect(obs, SIGNAL(packageListIsReady()), this, SLOT(insertPackageList()));
    connect(obs, SIGNAL(finishedParsingFile(OBSFile*)), this, SLOT(insertFile(OBSFile*)));
    connect(obs, SIGNAL(finishedParsingPackage(OBSPackage*,int)),
            this, SLOT(insertBuildStatus(OBSPackage*, const int)));
    connect(obs, SIGNAL(finishedParsingResult(OBSResult*)),
            this, SLOT(insertResult(OBSResult*)));
    connect(obs, SIGNAL(finishedParsingResultList()),
            this, SLOT(finishedResultListSlot()));
    connect(obs, SIGNAL(finishedParsingRequest(OBSRequest*)),
            this, SLOT(insertRequest(OBSRequest*)));
    connect(obs, SIGNAL(removeRequest(const QString&)),
            this, SLOT(removeRequest(const QString&)));
    connect(obs, SIGNAL(srStatus(QString)), this, SLOT(srStatusSlot(QString)));

    readSettings();
    readSettingsTimer();
    credentials->readPassword(loginDialog->getUsername());
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

void MainWindow::errorReadingPasswordSlot(const QString &error)
{
    qDebug() << "MainWindow::errorReadingPasswordSlot()" << error;
    loginDialog->show();
}

void MainWindow::credentialsRestoredSlot(const QString &username, const QString &password)
{
    qDebug() << "MainWindow::credentialsRestored()";
    obs->setCredentials(username, password);
    emit updateStatusBar(tr("Logging in..."), false);
    obs->login();
}

void MainWindow::showNetworkError(const QString &networkError)
{
    qDebug() << "MainWindow::showNetworkError()";

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
    action_Refresh->setEnabled(authenticated);
    if (authenticated) {
        setupBrowser();
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
    action_Refresh->setEnabled(false);
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

void MainWindow::setupBrowser()
{
    qDebug() << "MainWindow::setupBrowser()";

    action_Add->setEnabled(false);
    action_Remove->setEnabled(false);
    action_MarkRead->setEnabled(false);
    ui->lineEditFilter->setFocus();

    connect(ui->lineEditFilter, SIGNAL(textChanged(QString)), this, SLOT(filterResults(QString)));
    connect(ui->radioButtonPackages, SIGNAL(clicked(bool)), this, SLOT(filterRadioButtonClicked(bool)));
    connect(ui->radioButtonProject, SIGNAL(clicked(bool)), this, SLOT(filterRadioButtonClicked(bool)));
    connect(configureDialog, SIGNAL(includeHomeProjectsChanged()), this, SLOT(refreshProjectFilter()));

    sourceModelProjects = new QStringListModel(ui->treeProjects);
    proxyModelProjects = new QSortFilterProxyModel(ui->treeProjects);
    sourceModelBuilds = new QStringListModel(ui->treeBuilds);
    proxyModelBuilds = new QSortFilterProxyModel(ui->treeBuilds);
    sourceModelFiles = NULL;
    sourceModelBuildResults = NULL;

    ui->treeProjects->setModel(proxyModelProjects);
    projectsSelectionModel = ui->treeProjects->selectionModel();
    ui->treeBuilds->setModel(proxyModelBuilds);
    buildsSelectionModel = ui->treeBuilds->selectionModel();

    connect(projectsSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
            SLOT(projectSelectionChanged(QItemSelection,QItemSelection)));
    connect(buildsSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
            SLOT(buildSelectionChanged(QItemSelection,QItemSelection)));

    ui->treeProjects->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeBuilds->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeFiles->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeBuildResults->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeFiles->setRootIsDecorated(false);

    emit updateStatusBar(tr("Getting projects..."), false);
    obs->getProjects();
}

void MainWindow::filterProjects(const QString &item)
{
    bool homeProjects = configureDialog->isIncludeHomeProjects();
    QString regExp = !homeProjects ? "^(?!home)(.*" + item + ")" : item;
    proxyModelProjects->setFilterRegExp(QRegExp(regExp, Qt::CaseInsensitive));
    proxyModelProjects->setFilterKeyColumn(0);

    QModelIndex currentIndex = ui->treeProjects->currentIndex();
    ui->treeProjects->scrollTo(currentIndex, QAbstractItemView::PositionAtTop);
}

void MainWindow::filterBuilds(const QString &item)
{
    qDebug() << "MainWindow::filterBuilds()" << item;
    proxyModelBuilds->setFilterRegExp(QRegExp(item, Qt::CaseInsensitive));
    proxyModelBuilds->setFilterKeyColumn(0);
}

void MainWindow::filterResults(const QString &item)
{
    qDebug() << "MainWindow::filterResults())";
    ui->radioButtonProject->isChecked() ? filterProjects(item) : filterBuilds(item);

    // Delete  treeBuilds' model rows when filter doesn't match a project
    if (proxyModelProjects->rowCount()==0 && ui->treeBuilds->model()->hasChildren()) {
        ui->treeBuilds->model()->removeRows(0, ui->treeBuilds->model()->rowCount());
    }
}

void MainWindow::filterRadioButtonClicked(bool)
{
    qDebug() << "MainWindow::filterRadioButtonClicked()";

    // Clear project filter on radio button click
    // if there were no matches for the project name
    if (proxyModelProjects->rowCount()==0 && ui->radioButtonPackages->isChecked()) {
        filterProjects("");
    }

    // Clear line edit text on radio button click
    // and set focus on line edit
    ui->lineEditFilter->clear();
    ui->lineEditFilter->setFocus();

    filterResults(ui->lineEditFilter->text());
}

void MainWindow::refreshProjectFilter()
{
    qDebug() << "MainWindow::refreshProjectFilter()";
    if (ui->radioButtonProject->isChecked()) {
        filterProjects(ui->lineEditFilter->text());
    }
}

void MainWindow::projectSelectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
    qDebug() << "MainWindow::projectSelectionChanged()";
    getPackages(ui->treeProjects->currentIndex());
    filterBuilds("");
}

void MainWindow::buildSelectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
    qDebug() << "MainWindow::buildSelectionChanged()";
    getPackageFiles(ui->treeBuilds->currentIndex());
}

void MainWindow::getPackages(QModelIndex index)
{
    QString project = index.data().toString();
    qDebug() << "MainWindow::getPackages()" << project;
    if (!project.isEmpty()) {
        emit updateStatusBar(tr("Getting packages..."), false);
        obs->getPackages(project);
    }
}

void MainWindow::getPackageFiles(QModelIndex index)
{
    qDebug() << "MainWindow::getPackageFiles()";
    emit updateStatusBar(tr("Getting package files.."), false);

    QStandardItemModel *oldModel = static_cast<QStandardItemModel*>(ui->treeFiles->model());
    sourceModelFiles = new QStandardItemModel(ui->treeFiles);
    QStringList treeFilesHeaders;
    treeFilesHeaders << tr("File name") << tr("Size") << tr("Modified Time");
    sourceModelFiles->setHorizontalHeaderLabels(treeFilesHeaders);
    ui->treeFiles->setModel(sourceModelFiles);
    ui->treeFiles->setColumnWidth(0, 200);
    delete oldModel;

    QString currentProject = ui->treeProjects->currentIndex().data().toString();
    QString currentPackage = index.data().toString();
    obs->getFiles(currentProject, currentPackage);
    emit updateStatusBar(tr("Getting package data..."), false);

    getBuildResults();
}

void MainWindow::getBuildResults()
{
    qDebug() << "MainWindow::getBuildResults()";
    emit updateStatusBar(tr("Getting build results..."), false);

    QStandardItemModel *oldModel = static_cast<QStandardItemModel*>(ui->treeBuildResults->model());
    sourceModelBuildResults = new QStandardItemModel(ui->treeBuildResults);
    QStringList treeBuildResultsHeaders;
    treeBuildResultsHeaders << tr("Repository") << tr("Arch") << tr("Status");
    sourceModelBuildResults->setHorizontalHeaderLabels(treeBuildResultsHeaders);
    ui->treeBuildResults->setModel(sourceModelBuildResults);
    ui->treeBuildResults->setColumnWidth(0, 200);
    delete oldModel;

    QString currentProject = ui->treeProjects->currentIndex().data().toString();
    QString currentPackage = ui->treeBuilds->currentIndex().data().toString();
    obs->getAllBuildStatus(currentProject, currentPackage);
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
        obs->getRequestDiff(item->text(2));
    } else {
        reqStateEditor->setDiff(item->text(5) + " " + item->text(3));
    }

    reqStateEditor->exec();
    delete reqStateEditor;
}

void MainWindow::srStatusSlot(const QString &status)
{
    qDebug() << "MainWindow::srStatusSlot()";
    if (status=="ok") {
        QTreeWidgetItem *item = ui->treeRequests->currentItem();
        item->setHidden(true);
    }
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

void MainWindow::finishedResultListSlot()
{
   qDebug() << "MainWindow::finishedResultListSlot()";
   emit updateStatusBar(tr("Done"), true);
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
    qDebug() << "MainWindow::refreshView()";
    int rows = ui->treePackages->topLevelItemCount();
    emit updateStatusBar(tr("Getting build statuses..."), false);

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
            obs->getBuildStatus(tableStringList, r);
        }
    }

//    Get SRs
    emit updateStatusBar(tr("Getting requests..."), false);
    obs->getRequests();
}

void MainWindow::markRead(QTreeWidgetItem* item, int)
{
    qDebug() << "MainWindow::markRead() " << "Row: " + QString::number(ui->treePackages->indexOfTopLevelItem(item));
    for (int i=0; i<ui->treePackages->columnCount(); i++) {
        if (item->font(0).bold()) {
            Utils::setItemBoldFont(item, false);
        }
    }

    setNotify(false);
}

void MainWindow::markAllRead()
{
    qDebug() << "MainWindow::markAllRead()";
    for (int i=0; i<ui->treePackages->topLevelItemCount(); i++) {
        if (ui->treePackages->topLevelItem(i)->font(0).bold()) {
            Utils::setItemBoldFont(ui->treePackages->topLevelItem(i), false);
        }
    }

    setNotify(false);
}

void MainWindow::createTreePackages()
{
    ui->treePackages->setOBS(obs);
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

void MainWindow::insertProjectList()
{
    qDebug() << "MainWindow::insertProjectList()";
    OBSXmlReader *reader = obs->getXmlReader();
    reader->readList();
    sourceModelProjects->setStringList(reader->getList());
    proxyModelProjects->setSourceModel(sourceModelProjects);
    ui->treeProjects->setModel(proxyModelProjects);
    emit updateStatusBar(tr("Done"), true);
}

void MainWindow::insertPackageList()
{
    if (ui->tabWidget->currentIndex()==0) {
        // Browser tab
        qDebug() << "MainWindow::insertPackageList()";

        OBSXmlReader *reader = obs->getXmlReader();
        reader->readList();

        sourceModelBuilds->setStringList(reader->getList());
        proxyModelBuilds->setSourceModel(sourceModelBuilds);
        ui->treeBuilds->setModel(proxyModelBuilds);

        delete sourceModelFiles;
        sourceModelFiles = NULL;

        delete sourceModelBuildResults;
        sourceModelBuildResults = NULL;
    }
    emit updateStatusBar(tr("Done"), true);
}

void MainWindow::insertFile(OBSFile *obsFile)
{
    qDebug() << "MainWindow::insertFile()";
    QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->treeFiles->model());
    if (model) {
        QStandardItem *itemName = new QStandardItem(obsFile->getName());
        QStandardItem *itemSize = new QStandardItem(Utils::fileSizeHuman(obsFile->getSize().toInt()));
        QString lastModified = Utils::unixTimeToDate(obsFile->getLastModified());
        QStandardItem *itemLastModified = new QStandardItem(lastModified);
        QList<QStandardItem*> items;
        items << itemName << itemSize << itemLastModified;
        model->appendRow(items);
    }
    delete obsFile;
}

void MainWindow::insertResult(OBSResult *obsResult)
{
    qDebug() << "MainWindow::insertResult()";

    if (ui->tabWidget->currentIndex()==1) {
        // Monitor tab
        ui->treePackages->insertDroppedPackage(obsResult);

    } else {
        QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->treeBuildResults->model());
        if (model) {
            QStandardItem *itemRepository = new QStandardItem(obsResult->getRepository());
            QStandardItem *itemArch = new QStandardItem(obsResult->getArch());
            QStandardItem *itemBuildResult = new QStandardItem(obsResult->getPackage()->getStatus());
            itemBuildResult->setForeground(Utils::getColorForStatus(itemBuildResult->text()));

            if (!obsResult->getPackage()->getDetails().isEmpty()) {
                QString details = obsResult->getPackage()->getDetails();
                details = Utils::breakLine(details, 250);
                itemBuildResult->setToolTip(details);
            }

            QList<QStandardItem*> items;
            items << itemRepository << itemArch << itemBuildResult;
            model->appendRow(items);
        }
        delete obsResult;
    }
}

void MainWindow::insertBuildStatus(OBSPackage* obsPackage, const int& row)
{
    qDebug() << "MainWindow::insertBuildStatus()";
    QString details = obsPackage->getDetails();
    QString status = obsPackage->getStatus();
    delete obsPackage;

//    If the line is too long (>250), break it
    details = Utils::breakLine(details, 250);
    if (details.size()>0) {
        qDebug() << "Details string size: " << details.size();
    }

    QTreeWidgetItem *item = ui->treePackages->topLevelItem(row);
    if (item) {
        QString oldStatus = item->text(4);
        item->setText(4, status);
        item->setToolTip(4, details);
        item->setForeground(4, Utils::getColorForStatus(status));

        qDebug() << "Build status" << status << "inserted in" << row
                 << "(Total rows:" << ui->treePackages->topLevelItemCount() << ")";

        //    If the old status is not empty and it is different from latest one,
        //    change the tray icon
        if (hasBuildStatusChanged(oldStatus, status)) {
            Utils::setItemBoldFont(item, true);
        }

        if (row == ui->treePackages->topLevelItemCount()-1) {
            emit updateStatusBar(tr("Done"), true);
        }
    } else {
        emit updateStatusBar(details, true);
    }
}

bool MainWindow::hasBuildStatusChanged(const QString &oldStatus, const QString &newStatus)
{
    qDebug() << "MainWindow::hasBuildStatusChanged()"
             << "Old status:" << oldStatus << "New status:" << newStatus;
    bool change = false;
    if (!oldStatus.isEmpty() && oldStatus != newStatus) {
        change = true;
        qDebug() << "MainWindow::hasBuildStatusChanged()" << change;
        setNotify(change);
    }
    return change;
}

void MainWindow::insertRequest(OBSRequest* obsRequest)
{
    qDebug() << "MainWindow::insertRequest()";
    int rows = ui->treeRequests->topLevelItemCount();
    int requests = obs->getRequestCount();
    qDebug() << "Table rows:" << rows+1 << "Total requests:" << requests;

    RequestTreeWidgetItem *item = new RequestTreeWidgetItem(ui->treeRequests);
    item->setText(0, obsRequest->getDate());
    item->setText(1, obsRequest->getId());
    item->setText(2, obsRequest->getSource());
    item->setText(3, obsRequest->getTarget());
    item->setText(4, obsRequest->getRequester());
    item->setText(5, obsRequest->getActionType());
    item->setText(6, obsRequest->getState());
    item->setDescription(obsRequest->getDescription());
    ui->treeRequests->addTopLevelItem(item);

    qDebug() << "Request added:" << obsRequest->getId();
    delete obsRequest;

    if (rows == ui->treeRequests->topLevelItemCount()-1) {
        emit updateStatusBar(tr("Done"), true);
    }
}

void MainWindow::removeRequest(const QString& id)
{
    QList<QTreeWidgetItem*> itemList = ui->treeRequests->findItems(id, Qt::MatchExactly, 1);
    delete itemList.at(0);
    qDebug() << "Request removed:" << id;
}

void MainWindow::getRequestDescription(QTreeWidgetItem* item, int)
{
    QString requestDescription = static_cast<RequestTreeWidgetItem*>(item)->getDescription();
    qDebug() << "getRequestDescription() " << "Row clicked: "
                + QString::number(ui->treeRequests->indexOfTopLevelItem(item));
    qDebug() << "Request description: " + requestDescription;
    ui->textBrowser->setText(requestDescription);
}

void MainWindow::setNotify(bool notify)
{
    qDebug() << "MainWindow::setNotify()" << notify;
    if (notify != m_notify) {
        m_notify = notify;
        emit notifyChanged(notify);
    }
}

void MainWindow::updateStatusBarSlot(const QString &message, bool progressBarHidden)
{
    qDebug() << "MainWindow::updateStatusBarSlot()";
    ui->statusbar->showMessage(message);
    progressBar->setHidden(progressBarHidden);
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

        loginDialog->isAutoLoginEnabled() ?
                    credentials->writeCredentials(loginDialog->getUsername(), loginDialog->getPassword()) :
                    credentials->deletePassword(loginDialog->getUsername());
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
                          "&copy; 2010-2017 Javier Llorente (Main developer)<br>"
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
    connect(this, SIGNAL(updateStatusBar(QString,bool)), this, SLOT(updateStatusBarSlot(QString,bool)));

    ui->statusbar->showMessage(tr("Offline"));
    progressBar = new QProgressBar(ui->statusbar);
    progressBar->setHidden(true);
    progressBar->setTextVisible(false);
    progressBar->setMaximum(0);
    progressBar->setMaximum(0);
    progressBar->setMaximumSize(50, ui->statusbar->height());
    ui->statusbar->addPermanentWidget(progressBar, 10);
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
        setNotify(false);
    }

    qDebug() << "MainWindow::trayIconClicked()";
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

    settings.beginGroup("Browser");
    settings.setValue("IncludeHomeProjects", configureDialog->isIncludeHomeProjects());
    settings.endGroup();
}

void MainWindow::readSettings()
{
    qDebug() << "MainWindow::readSettings()";
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

        settings.beginGroup("Browser");
        configureDialog->setIncludeHomeProjects(settings.value("IncludeHomeProjects").toBool());
        settings.endGroup();
}

void MainWindow::readSettingsTimer()
{
    QSettings settings("Qactus","Qactus");
    settings.beginGroup("Timer");
    if (settings.value("Active").toBool()) {
        qDebug() << "MainWindow::readSettingsTimer() Timer Active = true";
        configureDialog->setCheckedTimerCheckbox(true);
        qDebug() << "MainWindow::readSettingsTimer() Interval:" << settings.value("Value").toString() << "minutes";
        configureDialog->setTimerInterval(settings.value("Value").toInt());
    } else {
        qDebug() << "MainWindow::readSettingsTimer() Timer Active = false";
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
    // Enable add and remove for the monitor tab
    bool enabled = false;
    if(index==1) {
        enabled = true;
    }
    action_Add->setEnabled(enabled);
    action_Remove->setEnabled(enabled);
    action_MarkRead->setEnabled(enabled);
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
        setNotify(false);
        break;
    default:
        break;
    }
    return QMainWindow::event(event);
}
