/* 
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2010-2018 Javier Llorente <javier@opensuse.org>
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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    obs = new OBS();

    trayIcon = new TrayIcon(this);
    m_notify = false;
    createActions();
    setupIconBar();
    createTreePackages();
    createTreeRequests();
    setupBrowser();
    createStatusBar();

    loginDialog = nullptr;
    errorBox = nullptr;

    createTimer();

    ui->hSplitterBrowser->setStretchFactor(1, 1);
    ui->hSplitterBrowser->setStretchFactor(0, 0);
    connect(ui->treeRequests, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(slotContextMenuRequests(QPoint)));
    ui->treeRequests->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(obs, SIGNAL(isAuthenticated(bool)), this, SLOT(isAuthenticated(bool)));
    connect(obs, SIGNAL(selfSignedCertificate(QNetworkReply*)),
            this, SLOT(handleSelfSignedCertificates(QNetworkReply*)));
    connect(obs, SIGNAL(networkError(QString)), this, SLOT(showNetworkError(QString)));

    connect(obs, SIGNAL(finishedParsingAbout(OBSAbout*)), this, SLOT(slotAbout(OBSAbout*)));

    connect(obs, SIGNAL(projectListIsReady()), this, SLOT(insertProjectList()));
    connect(obs, SIGNAL(packageListIsReady()), this, SLOT(insertPackageList()));
    connect(obs, SIGNAL(finishedParsingFile(OBSFile*)), this, SLOT(addFile(OBSFile*)));
    connect(obs, SIGNAL(finishedParsingFileList()), this, SLOT(slotFileListAdded()));
    connect(obs, SIGNAL(finishedParsingPackage(OBSStatus*,int)),
            this, SLOT(insertBuildStatus(OBSStatus*,int)));

    connect(obs, SIGNAL(finishedParsingBranchPackage(OBSStatus*)),
            this, SLOT(slotBranchPackage(OBSStatus*)));

    connect(obs, SIGNAL(finishedParsingUploadFileRevision(OBSRevision*)),
            this, SLOT(slotUploadFile(OBSRevision*)));
    connect(obs, SIGNAL(cannotUploadFile(OBSStatus*)),
            this, SLOT(slotUploadFileError(OBSStatus*)));

    connect(obs, SIGNAL(finishedParsingDeletePrjStatus(OBSStatus*)),
            this, SLOT(slotDeleteProject(OBSStatus*)));
    connect(obs, SIGNAL(finishedParsingDeletePkgStatus(OBSStatus*)),
            this, SLOT(slotDeletePackage(OBSStatus*)));
    connect(obs, SIGNAL(finishedParsingDeleteFileStatus(OBSStatus*)),
            this, SLOT(slotDeleteFile(OBSStatus*)));
    connect(obs, SIGNAL(cannotDeleteProject(OBSStatus*)),
            this, SLOT(slotDeleteProject(OBSStatus*)));
    connect(obs, SIGNAL(cannotDeletePackage(OBSStatus*)),
            this, SLOT(slotDeletePackage(OBSStatus*)));
    connect(obs, SIGNAL(cannotDeleteFile(OBSStatus*)),
            this, SLOT(slotDeleteFile(OBSStatus*)));

    connect(obs, SIGNAL(finishedParsingResult(OBSResult*)),
            this, SLOT(addResult(OBSResult*)));
    connect(obs, SIGNAL(finishedParsingResultList()),
            this, SLOT(finishedAddingResults()));
    connect(obs, SIGNAL(finishedParsingRequest(OBSRequest*)),
            this, SLOT(insertRequest(OBSRequest*)));
    connect(obs, SIGNAL(removeRequest(const QString&)),
            this, SLOT(removeRequest(const QString&)));
    connect(obs, SIGNAL(srStatus(QString)), this, SLOT(srStatusSlot(QString)));

    readSettings();
    readTimerSettings();
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
    showLoginDialog();
}

void MainWindow::credentialsRestoredSlot(const QString &username, const QString &password)
{
    qDebug() << "MainWindow::credentialsRestored()";
    loginSlot(username, password);
    QProgressDialog progress(tr("Logging in..."), 0, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    emit updateStatusBar(tr("Logging in..."), false);
}

void MainWindow::showNetworkError(const QString &networkError)
{
    qDebug() << "MainWindow::showNetworkError()";

    // The QMessageBox is only displayed once if there are
    // repeated errors (queued requests, probably same error)
    if(!errorBox) {
        errorBox = new QMessageBox(this);
    }

    if (!errorBox->isVisible()) {
        errorBox->setWindowTitle(tr("Network Error"));
        errorBox->setText(networkError);
        errorBox->setIcon(QMessageBox::Critical);
        emit updateStatusBar(tr("Network error"), true);
        int ret = errorBox->exec();
        if (ret) {
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
        const QString title = QString(tr("Warning"));
        const QString text = QString(tr("Do you want to accept this self-signed certificate from %1?")
                                     .arg(certInfo.at(0)));
        QMessageBox::StandardButton result = QMessageBox::warning(this, title, text,
                                                                  QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes)  {
            qDebug() << "Saving self-signed certificate as" << filename;

            QFile file(filename);
            file.open(QIODevice::WriteOnly);
            file.write(sslCertificate.toPem());
            file.close();

            QSslSocket::addDefaultCaCertificate(sslCertificate);
            obs->request(reply);
            reply->ignoreSslErrors();
        }
    }
}

void MainWindow::apiChangedSlot()
{
    qDebug() << "MainWindow::apiChangedSlot()";
    showLoginDialog();
}

void MainWindow::isAuthenticated(bool authenticated)
{
    qDebug() << "MainWindow::isAuthenticated()" << authenticated;
    ui->action_Refresh->setEnabled(authenticated);
    if (authenticated) {
        loadProjects();
        on_action_Refresh_triggered();
        ui->actionAPI_information->setEnabled(true);
        delete loginDialog;
        loginDialog = nullptr;
    } else {
        emit updateStatusBar(tr("Authentication is required"), true);
        showLoginDialog();
    }
}

void MainWindow::setupBrowser()
{
    qDebug() << "MainWindow::setupBrowser()";

    browserFilter->setFocus();

    connect(browserFilter, SIGNAL(textChanged(QString)), this, SLOT(filterResults(QString)));
    connect(browserFilter, SIGNAL(projectClicked(bool)), this, SLOT(filterRadioButtonClicked(bool)));
    connect(browserFilter, SIGNAL(packageClicked(bool)), this, SLOT(filterRadioButtonClicked(bool)));

    connect(ui->treeProjects, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(slotContextMenuProjects(QPoint)));
    ui->treeProjects->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->treeBuilds, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(slotContextMenuPackages(QPoint)));
    ui->treeBuilds->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->treeFiles, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(slotContextMenuFiles(QPoint)));
    ui->treeFiles->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->treeBuildResults, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(slotContextMenuResults(QPoint)));
    ui->treeBuildResults->setContextMenuPolicy(Qt::CustomContextMenu);

    sourceModelProjects = new QStringListModel(ui->treeProjects);
    proxyModelProjects = new QSortFilterProxyModel(ui->treeProjects);
    sourceModelBuilds = new QStringListModel(ui->treeBuilds);
    proxyModelBuilds = nullptr;
    sourceModelFiles = nullptr;
    sourceModelBuildResults = nullptr;

    firstTimeFileListDisplayed = true;
    firstTimeBuildResultsDisplayed = true;

    deleteProjectConnected = false;
    deletePackageConnected = false;
    deleteFileConnected = false;
}

void MainWindow::setupProjectActions()
{
    qDebug() << "MainWindow::setupProjectActions()";
    ui->action_Branch_package->setEnabled(false);
    ui->action_Upload_file->setEnabled(false);
    actionDelete_file->setEnabled(false);
    actionDelete_package->setEnabled(false);
    actionDelete_project->setEnabled(true);
    ui->action_Delete->setEnabled(true);
    actionDelete_package->setShortcut(QKeySequence());
    actionDelete_file->setShortcut(QKeySequence());
    actionDelete_project->setShortcut(QKeySequence::Delete);

    ui->action_Delete->setEnabled(true);
    disconnect(ui->action_Delete, SIGNAL(triggered(bool)), this, SLOT(deletePackage()));
    disconnect(ui->action_Delete, SIGNAL(triggered(bool)), this, SLOT(deleteFile()));

    deletePackageConnected = false;
    deleteFileConnected = false;
    if (!deleteProjectConnected) {
        connect(ui->action_Delete, SIGNAL(triggered(bool)), this, SLOT(deleteProject()));
        deleteProjectConnected = true;
    }

    actionNew_project->setShortcut(QKeySequence::New);
    actionNew_package->setShortcut(QKeySequence());
}

void MainWindow::setupPackageActions()
{
    qDebug() << "MainWindow::setupPackageActions()";
    ui->action_Branch_package->setEnabled(true);
    ui->action_Upload_file->setEnabled(true);
    actionDelete_file->setEnabled(false);
    actionDelete_package->setEnabled(true);
    ui->action_Delete->setEnabled(true);
    actionDelete_project->setShortcut(QKeySequence());
    actionDelete_file->setShortcut(QKeySequence());
    actionDelete_package->setShortcut(QKeySequence::Delete);

    ui->action_Delete->setEnabled(true);
    disconnect(ui->action_Delete, SIGNAL(triggered(bool)), this, SLOT(deleteProject()));
    disconnect(ui->action_Delete, SIGNAL(triggered(bool)), this, SLOT(deleteFile()));

    deleteProjectConnected = false;
    deleteFileConnected = false;
    if (!deletePackageConnected) {
        connect(ui->action_Delete, SIGNAL(triggered(bool)), this, SLOT(deletePackage()));
        deletePackageConnected = true;
    }

    actionNew_project->setShortcut(QKeySequence());
    actionNew_package->setShortcut(QKeySequence::New);
}

void MainWindow::loadProjects()
{
    qDebug() << "MainWindow::loadProjects()";

    // Clean up package list, files and results
    if (proxyModelBuilds!=nullptr) {
        delete proxyModelBuilds;
        proxyModelBuilds = nullptr;
    }

    if (sourceModelFiles!=nullptr) {
        delete sourceModelFiles;
        sourceModelFiles = nullptr;
    }

    if (sourceModelBuildResults!=nullptr) {
        delete sourceModelBuildResults;
        sourceModelBuildResults = nullptr;
    }

    ui->treeProjects->setModel(proxyModelProjects);
    projectsSelectionModel = ui->treeProjects->selectionModel();

    proxyModelBuilds = new QSortFilterProxyModel(ui->treeBuilds);
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
    ui->action_Delete->setEnabled(false);

    emit updateStatusBar(tr("Getting projects..."), false);
    obs->getProjects();
}

void MainWindow::filterProjects(const QString &item)
{
    QString regExp = !includeHomeProjects ? "^(?!home)(.*" + item + ")" : item;
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
    browserFilter->isProjectChecked() ? filterProjects(item) : filterBuilds(item);

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
    if (proxyModelProjects->rowCount()==0 && browserFilter->isPackageChecked()) {
        filterProjects("");
    }

    // Clear line edit text on radio button click
    // and set focus on line edit
    browserFilter->clear();
    browserFilter->setFocus();

    filterResults(browserFilter->getText());
}

void MainWindow::refreshProjectFilter()
{
    qDebug() << "MainWindow::refreshProjectFilter()";
    readBrowserSettings();
    if (browserFilter->isProjectChecked()) {
        filterProjects(browserFilter->getText());
    }
}

void MainWindow::projectSelectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
    qDebug() << "MainWindow::projectSelectionChanged()";

    // Clean up files and build results on project click
    if (sourceModelFiles!=nullptr) {
        delete sourceModelFiles;
        sourceModelFiles = nullptr;
    }

    if (sourceModelBuildResults!=nullptr) {
        delete sourceModelBuildResults;
        sourceModelBuildResults = nullptr;
    }

    getPackages(ui->treeProjects->currentIndex());
    filterBuilds("");
    setupProjectActions();
}

void MainWindow::buildSelectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
    qDebug() << "MainWindow::buildSelectionChanged()";

    // Make sure the index is valid (eg: the filter yields results)
    if (ui->treeBuilds->currentIndex().isValid()) {
        getPackageFiles(ui->treeBuilds->currentIndex());
        setupPackageActions();

    } else {
        // If there is no package selected, clear both the file and build result lists
        if (sourceModelFiles!=nullptr) {
            delete sourceModelFiles;
            sourceModelFiles = nullptr;
        }

        if (sourceModelBuildResults!=nullptr) {
            delete sourceModelBuildResults;
            sourceModelBuildResults = nullptr;
        }

        setupProjectActions();
    }
}

void MainWindow::fileSelectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
    qDebug() << "MainWindow::fileSelectionChanged()";
    actionDelete_file->setEnabled(true);
    actionDelete_project->setShortcut(QKeySequence());
    actionDelete_package->setShortcut(QKeySequence());
    actionDelete_file->setShortcut(QKeySequence::Delete);

    ui->action_Delete->setEnabled(true);
    disconnect(ui->action_Delete, SIGNAL(triggered(bool)), this, SLOT(deleteProject()));
    disconnect(ui->action_Delete, SIGNAL(triggered(bool)), this, SLOT(deletePackage()));

    deleteProjectConnected = false;
    deletePackageConnected = false;
    if (!deleteFileConnected) {
        connect(ui->action_Delete, SIGNAL(triggered(bool)), this, SLOT(deleteFile()));
        deleteFileConnected = true;
    }

    actionNew_project->setShortcut(QKeySequence());
    actionNew_package->setShortcut(QKeySequence());
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

void MainWindow::reloadPackages()
{
    qDebug() << "MainWindow::reloadPackages()";
    getPackages(ui->treeProjects->currentIndex());

    // Clean up package files and results

    if (sourceModelFiles!=nullptr) {
        delete sourceModelFiles;
        sourceModelFiles = nullptr;
    }

    if (sourceModelBuildResults!=nullptr) {
        delete sourceModelBuildResults;
        sourceModelBuildResults = nullptr;
    }

    setupProjectActions();
}

void MainWindow::getPackageFiles(QModelIndex index)
{
    qDebug() << "MainWindow::getPackageFiles()";
    emit updateStatusBar(tr("Getting package files.."), false);

    QStandardItemModel *oldModel = static_cast<QStandardItemModel*>(ui->treeFiles->model());
    sourceModelFiles = new QStandardItemModel(ui->treeFiles);
    QStringList treeFilesHeaders;
    treeFilesHeaders << tr("File name") << tr("Size") << tr("Modified time");
    sourceModelFiles->setHorizontalHeaderLabels(treeFilesHeaders);
    ui->treeFiles->setModel(sourceModelFiles);
    ui->treeFiles->setColumnWidth(0, 250);
    delete oldModel;

    filesSelectionModel = ui->treeFiles->selectionModel();
    connect(filesSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
            SLOT(fileSelectionChanged(QItemSelection,QItemSelection)));

    QString currentProject = ui->treeProjects->currentIndex().data().toString();
    QString currentPackage = index.data().toString();
    obs->getFiles(currentProject, currentPackage);
    emit updateStatusBar(tr("Getting package data..."), false);

    actionDelete_file->setEnabled(true);
    ui->action_Delete->setEnabled(true);

    getBuildResults();
}

void MainWindow::reloadFiles()
{
    qDebug() << "MainWindow::reloadFiles()";
    getPackageFiles(ui->treeBuilds->currentIndex());
    setupPackageActions();
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
    ui->treeBuildResults->setColumnWidth(0, 250);
    delete oldModel;
    oldModel = nullptr;

    currentProject = ui->treeProjects->currentIndex().data().toString();
    currentPackage = ui->treeBuilds->currentIndex().data().toString();
    obs->getAllBuildStatus(currentProject, currentPackage);
}

void MainWindow::reloadResults()
{
    qDebug() << "MainWindow::reloadResults()";
    getBuildResults();
    setupPackageActions();
}

void MainWindow::slotContextMenuRequests(const QPoint &point)
{
    QMenu *treeRequestsMenu = new QMenu(ui->treeRequests);

    QAction *actionChangeRequestState =  new QAction(tr("Change &State"), this);
    actionChangeRequestState->setIcon(QIcon::fromTheme("mail-reply-sender"));
    actionChangeRequestState->setText("Change State");
    connect(actionChangeRequestState, SIGNAL(triggered()), this, SLOT(changeRequestState()));

    treeRequestsMenu->addAction(actionChangeRequestState);

    QModelIndex index = ui->treeRequests->indexAt(point);
    if (index.isValid()) {
        treeRequestsMenu->exec(ui->treeRequests->mapToGlobal(point));
    }
}

void MainWindow::slotContextMenuProjects(const QPoint &point)
{
    QMenu *treeProjectsMenu = new QMenu(ui->treeProjects);
    treeProjectsMenu->addAction(actionNew_project);
    treeProjectsMenu->addAction(action_ReloadProjects);
    treeProjectsMenu->addAction(actionDelete_project);

    QModelIndex index = ui->treeProjects->indexAt(point);
    if (index.isValid()) {
        treeProjectsMenu->exec(ui->treeProjects->mapToGlobal(point));
    }
}

void MainWindow::slotContextMenuPackages(const QPoint &point)
{
    QMenu *treePackagesMenu = new QMenu(ui->treeBuilds);
    treePackagesMenu->addAction(actionNew_package);
    treePackagesMenu->addAction(ui->action_Branch_package);
    treePackagesMenu->addAction(action_ReloadPackages);
    treePackagesMenu->addAction(actionDelete_package);

    QModelIndex index = ui->treeBuilds->indexAt(point);
    if (index.isValid()) {
        treePackagesMenu->exec(ui->treeBuilds->mapToGlobal(point));
    }
}

void MainWindow::slotContextMenuFiles(const QPoint &point)
{
    QMenu *treeFilesMenu = new QMenu(ui->treeFiles);
    treeFilesMenu->addAction(ui->action_Upload_file);
    treeFilesMenu->addAction(action_ReloadFiles);
    treeFilesMenu->addAction(actionDelete_file);

    QModelIndex index = ui->treeFiles->indexAt(point);
    if (index.isValid()) {
        treeFilesMenu->exec(ui->treeFiles->mapToGlobal(point));
    }
}

void MainWindow::slotContextMenuResults(const QPoint &point)
{
    QMenu *treeResultsMenu = new QMenu(ui->treeBuildResults);
    treeResultsMenu->addAction(action_ReloadResults);

    QModelIndex index = ui->treeBuildResults->indexAt(point);
    if (index.isValid()) {
        treeResultsMenu->exec(ui->treeBuildResults->mapToGlobal(point));
    }
}

void MainWindow::changeRequestState()
{
    qDebug() << "Launching RequestStateEditor...";
    RequestStateEditor *reqStateEditor = new RequestStateEditor(this, obs);
    disconnect(obs, SIGNAL(finishedParsingResult(OBSResult*)), this, SLOT(addResult(OBSResult*)));
    disconnect(obs, SIGNAL(finishedParsingResult(OBSResult*)), ui->treePackages, SLOT(addDroppedPackage(OBSResult*)));
    QTreeWidgetItem *item = ui->treeRequests->currentItem();
    qDebug() << "Request selected:" << item->text(1);
    reqStateEditor->setRequestId(item->text(1));
    reqStateEditor->setDate(item->text(0));
    reqStateEditor->setSource(item->text(2));
    reqStateEditor->setTarget(item->text(3));
    reqStateEditor->setRequester(item->text(4));

    if (item->text(5)=="submit") {
        QProgressDialog progress(tr("Getting diff..."), 0, 0, 0, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();

        // Get SR diff
        obs->getRequestDiff(item->text(2));

        // Get build results
        QStringList source = item->text(2).split("/");
        QString project = source.at(0);
        QString package = source.at(1);
        obs->getAllBuildStatus(project, package);
    } else {
        reqStateEditor->setDiff(item->text(5) + " " + item->text(3));
    }

    reqStateEditor->exec();
    delete reqStateEditor;
    reqStateEditor = nullptr;

    connect(obs, SIGNAL(finishedParsingResult(OBSResult*)), ui->treePackages, SLOT(addDroppedPackage(OBSResult*)));
    connect(obs, SIGNAL(finishedParsingResult(OBSResult*)), this, SLOT(addResult(OBSResult*)));
}

void MainWindow::srStatusSlot(const QString &status)
{
    qDebug() << "MainWindow::srStatusSlot()";
    if (status=="ok") {
        QTreeWidgetItem *item = ui->treeRequests->currentItem();
        item->setHidden(true);
        ui->textBrowser->clear();
    }
}

void MainWindow::on_action_Add_triggered()
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

void MainWindow::finishedAddingResults()
{
   qDebug() << "MainWindow::finishedAddingResults()";
   if (firstTimeBuildResultsDisplayed) {
       ui->treeBuildResults->sortByColumn(0, Qt::AscendingOrder);
       firstTimeBuildResultsDisplayed = false;
   } else {
       int column = ui->treeBuildResults->header()->sortIndicatorSection();
       ui->treeBuildResults->sortByColumn(-1);
       ui->treeBuildResults->sortByColumn(column);
   }

   if (!currentProject.isEmpty() && !currentPackage.isEmpty()) {
       currentProject = "";
       currentPackage = "";
   }

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

void MainWindow::slotEnableRemoveRow()
{
    QList<QModelIndex> list = ui->treePackages->selectionModel()->selectedIndexes();

    if (list.isEmpty()) {
        ui->action_Remove->setEnabled(false);
    } else if (!ui->action_Remove->isEnabled()) {
        ui->action_Remove->setEnabled(true);
    }

}

void MainWindow::on_action_Remove_triggered()
{
    qDebug () << "MainWindow::removeRow()";
    QList<QTreeWidgetItem *> items = ui->treePackages->selectedItems();
    QList<QModelIndex> list = ui->treePackages->selectionModel()->selectedIndexes();
    foreach (QTreeWidgetItem *item, items) {
        delete item;
    }

    if (!list.isEmpty()) {
        QTreeWidgetItem *currentItem = ui->treePackages->currentItem();
        if (currentItem) {
            currentItem->setSelected(true);
        }
    }
}

void MainWindow::on_action_Branch_package_triggered()
{
    QModelIndex prjIndex = ui->treeProjects->currentIndex();
    QString project = prjIndex.data().toString();
    QModelIndex pkgIndex = ui->treeBuilds->currentIndex();
    QString build = pkgIndex.data().toString();

    const QString title = tr("Branch confirmation");
    const QString text = tr("<b>Source</b><br> %1/%2<br><b>Destination</b><br> home:%3:branches:%4")
                           .arg(project, build, obs->getUsername(), project);

    QMessageBox::StandardButton result = QMessageBox::question(this, title, text,
                                                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        qDebug() << "Branching package..." << project << "/" << build;
        obs->branchPackage(project, build);
        const QString statusText = tr("Branching %1/%2...").arg(project, build);
        emit updateStatusBar(statusText, false);
    }
}

void MainWindow::on_action_Upload_file_triggered()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Upload file"));
    qDebug() << "MainWindow::on_action_Upload_file_triggered() path:" << path;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();
    qDebug() << "MainWindow::on_action_Upload_file_triggered() data.size()" << data.size();

    QModelIndex prjIndex = ui->treeProjects->currentIndex();
    QString project = prjIndex.data().toString();

    QModelIndex pkgIndex = ui->treeBuilds->currentIndex();
    QString build = pkgIndex.data().toString();

    QFileInfo fi(file.fileName());
    QString fileName = fi.fileName();

    obs->uploadFile(project, build, fileName, data);

    QString statusText = tr("Uploading %1 to %2/%3...").arg(fileName, project, build);
    emit updateStatusBar(statusText, false);
}

void MainWindow::newProject()
{
    qDebug() << "MainWindow:newProject()";

    QModelIndex prjIndex = ui->treeProjects->currentIndex();
    QString project = prjIndex.data().toString();

    CreateDialog *createDialog = new CreateDialog(obs, this);
    createDialog->setProjectMode(true);
    createDialog->setProject(project);
    createDialog->exec();
    delete createDialog;
    createDialog = nullptr;
}

void MainWindow::newPackage()
{
    qDebug() << "MainWindow:newPackage()";

    QModelIndex prjIndex = ui->treeProjects->currentIndex();
    QString project = prjIndex.data().toString();

    CreateDialog *createDialog = new CreateDialog(obs, this);
    createDialog->setProject(project);
    createDialog->exec();
    delete createDialog;
    createDialog = nullptr;
}

void MainWindow::deleteProject()
{
    qDebug() << "MainWindow:deleteProject()";

    QModelIndex prjIndex = ui->treeProjects->currentIndex();
    QString project = prjIndex.data().toString();

    const QString title = tr("Delete confirmation");
    const QString text = tr("Do you really want to delete project<br> %1?")
                           .arg(project);

    QMessageBox::StandardButton result = QMessageBox::question(this, title, text,
                                                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        qDebug() << "Deleting project" << project << "...";
        obs->deleteProject(project);
        const QString statusText = tr("Deleting %1...").arg(project);
        emit updateStatusBar(statusText, false);
    }
}

void MainWindow::deletePackage()
{
    qDebug() << "MainWindow:deletePackage()";

    QModelIndex prjIndex = ui->treeProjects->currentIndex();
    QString project = prjIndex.data().toString();
    QModelIndex pkgIndex = ui->treeBuilds->currentIndex();
    QString package = pkgIndex.data().toString();

    const QString title = tr("Delete confirmation");
    const QString text = tr("Do you really want to delete package<br> %1/%2?")
                           .arg(project, package);

    QMessageBox::StandardButton result = QMessageBox::question(this, title, text,
                                                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        qDebug() << "Deleting package" << package << "...";
        obs->deletePackage(project, package);
        const QString statusText = tr("Deleting %1...").arg(package);
        emit updateStatusBar(statusText, false);
    }
}

void MainWindow::deleteFile()
{
    qDebug() << "MainWindow:deleteFile()";

    QModelIndex prjIndex = ui->treeProjects->currentIndex();
    QString project = prjIndex.data().toString();
    QModelIndex pkgIndex = ui->treeBuilds->currentIndex();
    QString package = pkgIndex.data().toString();
    QModelIndex fileIndex = ui->treeFiles->currentIndex();
    QString fileName = fileIndex.data().toString();

    const QString title = tr("Delete confirmation");
    const QString text = tr("Do you really want to delete file<br> %1/%2/%3?")
                           .arg(project, package, fileName);

    QMessageBox::StandardButton result = QMessageBox::question(this, title, text,
                                                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        qDebug() << "Deleting file" << fileName << "...";
        obs->deleteFile(project, package, fileName);
        const QString statusText = tr("Deleting %1...").arg(fileName);
        emit updateStatusBar(statusText, false);
    }
}

void MainWindow::on_action_Refresh_triggered()
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

void MainWindow::on_action_Mark_all_as_read_triggered()
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
    connect(ui->treePackages, SIGNAL(itemSelectionChanged()), this, SLOT(slotEnableRemoveRow()));
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
    connect(ui->actionChange_request_state, SIGNAL(triggered()), this, SLOT(changeRequestState()));
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
    filterProjects("");
    emit updateStatusBar(tr("Done"), true);
}

void MainWindow::insertPackageList()
{
    if (ui->iconBar->currentRow()==0) {
        // Browser tab
        qDebug() << "MainWindow::insertPackageList()";

        OBSXmlReader *reader = obs->getXmlReader();
        reader->readList();

        sourceModelBuilds->setStringList(reader->getList());
        proxyModelBuilds->setSourceModel(sourceModelBuilds);
        ui->treeBuilds->setModel(proxyModelBuilds);
    }
    emit updateStatusBar(tr("Done"), true);
}

void MainWindow::addFile(OBSFile *obsFile)
{
    qDebug() << "MainWindow::addFile()";

    currentProject = ui->treeProjects->currentIndex().data().toString();
    currentPackage = ui->treeBuilds->currentIndex().data().toString();
    QString fileProject = obsFile->getProject();
    QString filePackage = obsFile->getPackage();

    if (currentProject==fileProject && currentPackage==filePackage) {


        QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->treeFiles->model());
        if (model) {
            model->setSortRole(Qt::UserRole);

            // Name
            QStandardItem *itemName = new QStandardItem();
            itemName->setData(obsFile->getName(), Qt::UserRole);
            itemName->setData(obsFile->getName(), Qt::DisplayRole);

            // Size
            QStandardItem *itemSize = new QStandardItem();
            QString fileSizeHuman;
#if QT_VERSION >= 0x051000
            QLocale locale = this->locale();
            fileSizeHuman = locale.formattedDataSize(obsFile->getSize().toInt());
#else
            fileSizeHuman = Utils::fileSizeHuman(obsFile->getSize().toInt());
#endif
            itemSize->setData(QVariant(fileSizeHuman), Qt::DisplayRole);
            itemSize->setData(obsFile->getSize().toInt(), Qt::UserRole);

            // Modified time
            QStandardItem *itemLastModified = new QStandardItem();
            QString lastModifiedStr;
            QString lastModifiedUnixTimeStr = obsFile->getLastModified();
#if QT_VERSION >= 0x050800
            QDateTime lastModifiedDateTime = QDateTime::fromSecsSinceEpoch(qint64(lastModifiedUnixTimeStr.toInt()), Qt::UTC);
            lastModifiedStr = lastModifiedDateTime.toString("dd/MM/yyyy H:mm");
#else
            lastModifiedStr = Utils::unixTimeToDate(lastModifiedUnixTimeStr);
#endif
            itemLastModified->setData(lastModifiedUnixTimeStr.toInt(), Qt::UserRole);
            itemLastModified->setData(lastModifiedStr, Qt::DisplayRole);

            QList<QStandardItem *> items;
            items << itemName << itemSize << itemLastModified;
            model->appendRow(items);
        }
    }
    delete obsFile;
    obsFile = nullptr;
}

void MainWindow::slotFileListAdded()
{
    qDebug() << "MainWindow::slotFileListAdded()";
    if (firstTimeFileListDisplayed) {
        ui->treeFiles->sortByColumn(0, Qt::AscendingOrder);
        firstTimeFileListDisplayed = false;
        return;
    }
    int column = ui->treeFiles->header()->sortIndicatorSection();
    ui->treeFiles->sortByColumn(-1);
    ui->treeFiles->sortByColumn(column);
}

void MainWindow::addResult(OBSResult *obsResult)
{
    qDebug() << "MainWindow::addResult()";

    currentProject = ui->treeProjects->currentIndex().data().toString();
    currentPackage = ui->treeBuilds->currentIndex().data().toString();
    QString resultProject = obsResult->getProject();
    QString resultPackage = obsResult->getStatus()->getPackage();

    if (currentProject==resultProject && currentPackage==resultPackage) {
        QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->treeBuildResults->model());

        if (model) {
            QStandardItem *itemRepository = new QStandardItem(obsResult->getRepository());
            QStandardItem *itemArch = new QStandardItem(obsResult->getArch());
            QStandardItem *itemBuildResult = new QStandardItem(obsResult->getStatus()->getCode());
            itemBuildResult->setForeground(Utils::getColorForStatus(itemBuildResult->text()));

            if (!obsResult->getStatus()->getDetails().isEmpty()) {
                QString details = obsResult->getStatus()->getDetails();
                details = Utils::breakLine(details, 250);
                itemBuildResult->setToolTip(details);
            }

            QList<QStandardItem*> items;
            items << itemRepository << itemArch << itemBuildResult;
            model->appendRow(items);
        }
    }

//  This slot is in charge of deleting result (last one connected)
    delete obsResult;
    obsResult = nullptr;
}

void MainWindow::insertBuildStatus(OBSStatus *obsStatus, int row)
{
    qDebug() << "MainWindow::insertBuildStatus()";
    QString details = obsStatus->getDetails();
    QString status = obsStatus->getCode();
    delete obsStatus;
    obsStatus = nullptr;

//    If the line is too long (>250), break it
    details = Utils::breakLine(details, 250);
    if (details.size()>0) {
        qDebug() << "Details string size: " << details.size();
    }

    QTreeWidgetItem *item = ui->treePackages->topLevelItem(row);
    if (item) {
        QString oldStatus = item->text(4);
        item->setText(4, status);
        if (!details.isEmpty()) {
            item->setToolTip(4, details);
        }
        item->setForeground(4, Utils::getColorForStatus(status));

        qDebug() << "Build status" << status << "inserted in" << row
                 << "(Total rows:" << ui->treePackages->topLevelItemCount() << ")";

        //    If the old status is not empty and it is different from latest one,
        //    change the tray icon and enable the "Mark all as read" button
        if (hasBuildStatusChanged(oldStatus, status)) {
            Utils::setItemBoldFont(item, true);
            ui->action_Mark_all_as_read->setEnabled(true);
        }

        if (row == ui->treePackages->topLevelItemCount()-1) {
            emit updateStatusBar(tr("Done"), true);
        }
    } else {
        emit updateStatusBar(details, true);
    }
}

void MainWindow::slotBranchPackage(OBSStatus *obsStatus)
{
    qDebug() << "MainWindow::slotBranchPackage()";

    if (obsStatus->getCode()=="ok") {
        loadProjects();
        trayIcon->showMessage(APP_NAME, tr("The package %1 has been branched").arg(obsStatus->getPackage()));

        // FIXME: Select and scroll to branch after branching. This doesn't work as loading the project list takes some time
    //    QString newBranch = QString("home:%1:branches:%2").arg(obs->getUsername(), obsStatus->getProject());
    //    QModelIndexList itemList = ui->treeProjects->model()->match(ui->treeProjects->model()->index(0, 0),
    //                                                                Qt::DisplayRole, QVariant::fromValue(QString(newBranch)),
    //                                                                1, Qt::MatchExactly);
    //    if (!itemList.isEmpty()) {
    //        auto itemIndex = itemList.at(0);
    //        ui->treeProjects->selectionModel()->select(itemIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    //        ui->treeProjects->scrollTo(itemIndex, QAbstractItemView::PositionAtTop);
    //    }

    } else {
        const QString title = tr("Warning");
        const QString text = obsStatus->getSummary() + "<br>" + obsStatus->getDetails();
        QMessageBox::warning(this, title, text);
    }

    delete obsStatus;
    obsStatus = nullptr;

    emit updateStatusBar(tr("Done"), true);
}

void MainWindow::slotUploadFile(OBSRevision *obsRevision)
{
    qDebug() << "MainWindow::slotUploadFile()";
    QString currentProject = ui->treeProjects->currentIndex().data().toString();
    QString currentPackage = ui->treeBuilds->currentIndex().data().toString();

    // Refresh file list
    if (currentProject == obsRevision->getProject() && currentPackage == obsRevision->getPackage()) {
        getPackageFiles(ui->treeBuilds->currentIndex());
    }
    trayIcon->showMessage(APP_NAME, tr("The file %1 has been uploaded").arg(obsRevision->getFile()));

    delete obsRevision;
    obsRevision = nullptr;

    emit updateStatusBar(tr("Done"), true);
}

void MainWindow::slotUploadFileError(OBSStatus *obsStatus)
{
    qDebug() << "MainWindow::slotUploadFileError()" << obsStatus->getCode();
    QString title = tr("Warning");
    QString text = obsStatus->getSummary() + "<br>" + obsStatus->getDetails();
    QMessageBox::warning(this, title, text);
    QString statusText = tr("Error uploading to %1/%2").arg(obsStatus->getProject(), obsStatus->getPackage());

    delete obsStatus;
    obsStatus = nullptr;

    emit updateStatusBar(statusText, true);
}

void MainWindow::slotDeleteProject(OBSStatus *obsStatus)
{
    qDebug() << "MainWindow::slotDeleteProject()";

    if (obsStatus->getCode()=="ok") {
        QModelIndexList itemList = ui->treeProjects->model()->match(ui->treeProjects->model()->index(0, 0),
                                                                    Qt::DisplayRole, QVariant::fromValue(QString(obsStatus->getProject())),
                                                                    1, Qt::MatchExactly);
        if(!itemList.isEmpty()) {
            auto item = itemList.at(0);
            ui->treeProjects->model()->removeRow(item.row(), item.parent());
        }
        trayIcon->showMessage(APP_NAME, tr("The project %1 has been deleted").arg(obsStatus->getProject()));
    } else {
        const QString title = tr("Warning");
        const QString text = QString("<b>%1</b><br>%2").arg(obsStatus->getSummary(), obsStatus->getDetails());
        QMessageBox::warning(this, title, text);
    }

    delete obsStatus;
    obsStatus = nullptr;

    emit updateStatusBar(tr("Done"), true);
}

void MainWindow::slotDeletePackage(OBSStatus *obsStatus)
{
    qDebug() << "MainWindow::slotDeletePackage()";

    if (obsStatus->getCode()=="ok") {
        QString currentProject = ui->treeProjects->currentIndex().data().toString();

        if (obsStatus->getProject()==currentProject) {
            QModelIndexList itemList = ui->treeBuilds->model()->match(ui->treeBuilds->model()->index(0, 0),
                                                                      Qt::DisplayRole, QVariant::fromValue(QString(obsStatus->getPackage())),
                                                                      1, Qt::MatchExactly);
            if(!itemList.isEmpty()) {
                auto itemIndex = itemList.at(0);
                ui->treeBuilds->model()->removeRow(itemIndex.row(), itemIndex.parent());
            }
        }
        trayIcon->showMessage(APP_NAME, tr("The package %1 has been deleted").arg(obsStatus->getPackage()));
    } else {
        const QString title = tr("Warning");
        const QString text = QString("<b>%1</b><br>%2").arg(obsStatus->getSummary(), obsStatus->getDetails());
        QMessageBox::warning(this, title, text);
    }

    delete obsStatus;
    obsStatus = nullptr;

    emit updateStatusBar(tr("Done"), true);
}

void MainWindow::slotDeleteFile(OBSStatus *obsStatus)
{
    qDebug() << "MainWindow::slotDeleteFile()";

    if (obsStatus->getCode()=="ok") {
        QString currentProject = ui->treeProjects->currentIndex().data().toString();
        QString currentPackage = ui->treeBuilds->currentIndex().data().toString();
        QString fileName = obsStatus->getDetails();

        if (obsStatus->getProject()==currentProject && obsStatus->getPackage()==currentPackage) {
            QModelIndexList itemList = ui->treeFiles->model()->match(ui->treeFiles->model()->index(0, 0),
                                                                      Qt::DisplayRole, QVariant::fromValue(QString(fileName)),
                                                                      1, Qt::MatchExactly);
            if (!itemList.isEmpty()) {
                auto itemIndex = itemList.at(0);
                ui->treeFiles->model()->removeRow(itemIndex.row(), itemIndex.parent());

                // Disable the delete file action if there are no files after a deletion
                QItemSelectionModel *treeFilesSelectionModel = ui->treeFiles->selectionModel();
                QList<QModelIndex> selectedFiles = treeFilesSelectionModel->selectedIndexes();
                bool enable = !selectedFiles.isEmpty();
                actionDelete_file->setEnabled(enable);
            }

        }
        trayIcon->showMessage(APP_NAME, tr("The file %1 has been deleted").arg(obsStatus->getDetails()));
    } else {
        const QString title = tr("Warning");
        const QString text = QString("<b>%1</b><br>%2").arg(obsStatus->getSummary(), obsStatus->getDetails());
        QMessageBox::warning(this, title, text);
    }

    delete obsStatus;
    obsStatus = nullptr;

    emit updateStatusBar(tr("Done"), true);
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
    ui->actionChange_request_state->setEnabled(true);
}

void MainWindow::setNotify(bool notify)
{
    qDebug() << "MainWindow::setNotify()" << notify;
    if (notify != m_notify) {
        m_notify = notify;
        emit notifyChanged(notify);
    }
}

void MainWindow::slotUpdateStatusBar(const QString &message, bool progressBarHidden)
{
    qDebug() << "MainWindow::slotUpdateStatusBar()";
    ui->statusbar->showMessage(message);
    progressBar->setHidden(progressBarHidden);
}

void MainWindow::loginSlot(const QString &username, const QString &password)
{
    qDebug() << "MainWindow::loginSlot()";
    obs->setCredentials(username, password);
    obs->login();
}

void MainWindow::on_action_Quit_triggered()
{
    qApp->quit();
}

void MainWindow::on_action_About_triggered()
{
    QMessageBox::about(this,tr("About") + " " + QCoreApplication::applicationName(),
                       "<h2 align=\"left\">" + QCoreApplication::applicationName() + "</h2>" +
                       tr("A Qt-based OBS notifier") + "<br>" +
                       tr("Version:") + " " + QCoreApplication::applicationVersion() +
                       "<br><a href='https://github.com/javierllorente/qactus'>https://github.com/javierllorente/qactus</a>" +
                       "<div align=\"left\">" +
                          "<p>" +
                          "&copy; 2010-2018 Javier Llorente (Main developer)<br>"
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
    connect(ui->action_About_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    // New button actions
    newButton = new QToolButton(this);
    newButton->setPopupMode(QToolButton::MenuButtonPopup);
    newButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    newButton->setText(tr("&New"));
    newButton->setIcon(QIcon::fromTheme("document-new"));

    newMenu = new QMenu(newButton);
    actionNew_package = new QAction(tr("New p&ackage"), this);
    actionNew_package->setIcon(QIcon::fromTheme("application-x-source-rpm"));
    connect(actionNew_package, SIGNAL(triggered(bool)), this, SLOT(newPackage()));

    actionNew_project = new QAction(tr("New pr&oject"), this);
    actionNew_project->setIcon(QIcon::fromTheme("project-development"));
    connect(actionNew_project, SIGNAL(triggered(bool)), this, SLOT(newProject()));

    newMenu->addAction(actionNew_package);
    newMenu->addAction(actionNew_project);
    newButton->setMenu(newMenu);

    actionNew = ui->toolBar->insertWidget(ui->action_Branch_package, newButton);
    connect(newButton, SIGNAL(clicked(bool)), this, SLOT(newPackage()));

    // Reload actions
    action_ReloadProjects = new QAction(tr("&Reload project list"), this);
    action_ReloadProjects->setIcon(QIcon::fromTheme("view-refresh"));
    connect(action_ReloadProjects, SIGNAL(triggered(bool)), this, SLOT(loadProjects()));

    action_ReloadPackages = new QAction(tr("&Reload package list"), this);
    action_ReloadPackages->setIcon(QIcon::fromTheme("view-refresh"));
    connect(action_ReloadPackages, SIGNAL(triggered(bool)), this, SLOT(reloadPackages()));

    action_ReloadFiles = new QAction(tr("&Reload file list"), this);
    action_ReloadFiles->setIcon(QIcon::fromTheme("view-refresh"));
    connect(action_ReloadFiles, SIGNAL(triggered(bool)), this, SLOT(reloadFiles()));

    action_ReloadResults = new QAction(tr("&Reload result list"), this);
    action_ReloadResults->setIcon(QIcon::fromTheme("view-refresh"));
    connect(action_ReloadResults, SIGNAL(triggered(bool)), this, SLOT(reloadResults()));

    // Delete actions
    actionDelete_project = new QAction(tr("Delete pro&ject"), this);
    actionDelete_project->setIcon(QIcon::fromTheme("trash-empty"));
    connect(actionDelete_project, SIGNAL(triggered(bool)), this, SLOT(deleteProject()));

    actionDelete_package = new QAction(tr("Delete pac&kage"), this);
    actionDelete_package->setIcon(QIcon::fromTheme("trash-empty"));
    connect(actionDelete_package, SIGNAL(triggered(bool)), this, SLOT(deletePackage()));

    actionDelete_file = new QAction(tr("Delete &file"), this);
    actionDelete_file->setIcon(QIcon::fromTheme("trash-empty"));
    connect(actionDelete_file, SIGNAL(triggered(bool)), this, SLOT(deleteFile()));

    // Browser filter actions
    QWidget *filterSpacer = new QWidget(this);
    filterSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    filterSpacer->setVisible(true);
    actionFilterSpacer = ui->toolBar->addWidget(filterSpacer);

    browserFilter = new BrowserFilter(this);
    actionFilter = ui->toolBar->addWidget(browserFilter);

    // Tray icon actions
    action_Restore = new QAction(tr("&Minimise"), trayIcon);
    connect(action_Restore, SIGNAL(triggered()), this, SLOT(toggleVisibility()));
    trayIcon->trayIconMenu->addAction(action_Restore);

    action_Quit = new QAction(tr("&Quit"), trayIcon);
    action_Quit->setIcon(QIcon::fromTheme("application-exit"));
    connect(action_Quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    trayIcon->trayIconMenu->addAction(action_Quit);
}

void MainWindow::setupIconBar()
{
    ui->iconBar->setMaximumWidth(ui->iconBar->sizeHintForColumn(0)+4);

    QListWidgetItem *browserItem = ui->iconBar->item(0);
    QIcon browserIcon(":/icons/32x32/browser.png");
    browserItem->setIcon(browserIcon);

    QListWidgetItem *monitorItem = ui->iconBar->item(1);
    QIcon monitorIcon(":/icons/32x32/monitor.png");
    monitorItem->setIcon(monitorIcon);

    QListWidgetItem *requestsItem = ui->iconBar->item(2);
    QIcon requestsIcon(":/icons/32x32/requests.png");
    requestsItem->setIcon(requestsIcon);
}

void MainWindow::createStatusBar()
{
    connect(this, SIGNAL(updateStatusBar(QString,bool)), this, SLOT(slotUpdateStatusBar(QString,bool)));

    ui->statusbar->showMessage(tr("Offline"));
    progressBar = new QProgressBar(ui->statusbar);
    progressBar->setHidden(true);
    progressBar->setTextVisible(false);
    progressBar->setMaximum(0);
    progressBar->setMaximum(0);
    progressBar->setMaximumSize(50, ui->statusbar->height());
    ui->statusbar->addPermanentWidget(progressBar, 10);
}

void MainWindow::createTimer()
{
    timer = new QTimer(this);
    interval = 0;
    connect(obs, SIGNAL(isAuthenticated(bool)), this, SLOT(startTimer(bool)));
    connect(timer, SIGNAL(timeout()), this, SLOT(on_action_Refresh_triggered()));
}

void MainWindow::setTimerInterval(int interval)
{
    qDebug() << "MainWindow::setTimerInterval()" << interval;
    if (interval >= 5) {
        this->interval = interval;
    } else {
        qDebug() << "Error starting timer: Wrong timer interval (smaller than 5)";
    }
}

void MainWindow::startTimer(bool authenticated)
{
    QSettings settings;
    settings.beginGroup("Timer");
    bool enableTimer = settings.value("Active").toBool();
    settings.endGroup();

    if (authenticated && enableTimer && !timer->isActive()) {
        qDebug() << "MainWindow::startTimer()" << interval;
        timer->start(interval*60000);
    } else {
        timer->stop();
    }
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
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("pos", pos());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("Row", ui->iconBar->currentRow());
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
    qDebug() << "MainWindow::readSettings()";
    readMWSettings();
    readProxySettings();
    readAuthSettings();
    readBrowserSettings();
    readMonitorSettings();
}

void MainWindow::readMWSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    restoreGeometry(settings.value("geometry").toByteArray());
    int row = settings.value("Row").toInt();
    ui->iconBar->setCurrentRow(row);
    settings.endGroup();

    on_iconBar_currentRowChanged(row);
}

void MainWindow::readMonitorSettings()
{
    QSettings settings;
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

void MainWindow::readAuthSettings()
{
    qDebug() << "MainWindow::readAuthSettings()";
    QSettings settings;
    settings.beginGroup("Auth");
    obs->setApiUrl(settings.value("ApiUrl").toString());
    if (settings.value("AutoLogin").toBool()) {
        Credentials *credentials = new Credentials(this);
        connect(credentials, SIGNAL(errorReadingPassword(QString)),
                this, SLOT(errorReadingPasswordSlot(QString)));
        connect(credentials, SIGNAL(credentialsRestored(QString, QString)),
                this, SLOT(credentialsRestoredSlot(QString, QString)));
        credentials->readPassword(settings.value("Username").toString());
        delete credentials;
    }
    settings.endGroup();
}

void MainWindow::readBrowserSettings()
{
    QSettings settings;
    settings.beginGroup("Browser");
    includeHomeProjects = settings.value("IncludeHomeProjects").toBool();
    settings.endGroup();
}

void MainWindow::readProxySettings()
{
    qDebug() << "MainWindow::readProxySettings()";

    QSettings settings;
    settings.beginGroup("Proxy");
    QNetworkProxy::ProxyType proxyType = static_cast<QNetworkProxy::ProxyType>(settings.value("Type").toInt());
    bool systemProxy = (proxyType == QNetworkProxy::DefaultProxy);
    bool manualProxy = (proxyType == QNetworkProxy::Socks5Proxy || proxyType == QNetworkProxy::HttpProxy);

    QNetworkProxyFactory::setUseSystemConfiguration(systemProxy);

    if (!systemProxy) {
        QNetworkProxy proxy;
        proxy.setType(proxyType);
        if (manualProxy) {
            proxy.setHostName(settings.value("Server").toString());
            proxy.setPort(settings.value("Port").toInt());
            proxy.setUser(settings.value("Username").toString());
            proxy.setPassword(settings.value("Password").toString());
        }
        QNetworkProxy::setApplicationProxy(proxy);
    }

    settings.endGroup();
}

void MainWindow::readTimerSettings()
{
    QSettings settings;
    settings.beginGroup("Timer");
    if (settings.value("Active").toBool()) {
        qDebug() << "MainWindow::readSettingsTimer() Timer Active = true";
        qDebug() << "MainWindow::readSettingsTimer() Interval:" << settings.value("Value").toString() << "minutes";
        setTimerInterval(settings.value("Value").toInt());
        startTimer(obs->isAuthenticated());
    } else {
        qDebug() << "MainWindow::readSettingsTimer() Timer Active = false";
        if (timer->isActive()) {
            timer->stop();
            qDebug() << "MainWindow::readSettingsTimer() Timer has been stopped";
        }
    }
    settings.endGroup();
}

void MainWindow::showLoginDialog()
{
    qDebug() << "MainWindow::showLoginDialog()";
    if (loginDialog==nullptr) {
        loginDialog = new Login(this);
        connect(loginDialog, SIGNAL(login(QString,QString)), this, SLOT(loginSlot(QString,QString)));
        loginDialog->exec();
//        delete loginDialog;
//        loginDialog = nullptr;
    } else {
        loginDialog->show();
    }

}

void MainWindow::on_action_Configure_Qactus_triggered()
{
    qDebug() << "MainWindow Launching Configure...";
    Configure *configure = new Configure(this, obs);
    connect(configure, SIGNAL(apiChanged()), this, SLOT(apiChangedSlot()));
    connect(configure, SIGNAL(proxyChanged()), this, SLOT(readProxySettings()));
    connect(configure, SIGNAL(includeHomeProjectsChanged()), this, SLOT(refreshProjectFilter()));
    connect(configure, SIGNAL(timerChanged()), this, SLOT(readTimerSettings()));
    configure->exec();
    delete configure;
}

void MainWindow::on_action_Login_triggered()
{
    showLoginDialog();
}

void MainWindow::on_actionAPI_information_triggered()
{
    obs->about();
}

void MainWindow::slotAbout(OBSAbout *obsAbout)
{
    const QString title = obsAbout->getTitle();
    const QString text = QString("%1<br>Revision: %2<br>Last deployment: %3").arg(obsAbout->getDescription(),
                                         obsAbout->getRevision(), obsAbout->getLastDeployment());
    QMessageBox::information(this, title, text);

    delete obsAbout;
    obsAbout = nullptr;
}

void MainWindow::on_iconBar_currentRowChanged(int index)
{
    // Enable/disable the branch/delete button if there is a file/package/project selected
    QItemSelectionModel *treeFilesSelectionModel = ui->treeFiles->selectionModel();
    if (treeFilesSelectionModel) {
        QList<QModelIndex> selectedFiles = treeFilesSelectionModel->selectedIndexes();
        bool enable = !selectedFiles.isEmpty();
        actionDelete_file->setEnabled(enable);
    } else {
        actionDelete_file->setEnabled(false);
    }

    QItemSelectionModel *treeBuildsSelectionModel = ui->treeBuilds->selectionModel();
    if (treeBuildsSelectionModel) {
        QList<QModelIndex> selectedBuilds = treeBuildsSelectionModel->selectedIndexes();
        bool enable = !selectedBuilds.isEmpty();
        ui->action_Branch_package->setEnabled(enable);
        ui->action_Upload_file->setEnabled(enable);
        actionDelete_package->setEnabled(enable);
    } else {
        ui->action_Branch_package->setEnabled(false);
        ui->action_Upload_file->setEnabled(false);
        actionDelete_package->setEnabled(false);
    }

    QItemSelectionModel *treeProjectsSelectionModel = ui->treeProjects->selectionModel();
    if (treeProjectsSelectionModel) {
        QList<QModelIndex> selectedProjects = treeProjectsSelectionModel->selectedIndexes();
        bool enable = !selectedProjects.isEmpty();
        actionDelete_project->setEnabled(enable);
    } else {
        actionDelete_project->setEnabled(false);
    }

    bool browserTabVisible = (index==0);
    actionNew->setVisible(browserTabVisible);
    ui->action_Branch_package->setVisible(browserTabVisible);
    ui->action_Upload_file->setVisible(browserTabVisible);
    ui->action_Delete->setVisible(browserTabVisible);
    actionFilterSpacer->setVisible(browserTabVisible);
    actionFilter->setVisible(browserTabVisible);

    bool monitorTabVisible = (index==1);
    ui->action_Add->setVisible(monitorTabVisible);
    ui->action_Remove->setVisible(monitorTabVisible);
    ui->action_Mark_all_as_read->setVisible(monitorTabVisible);

    bool requestsTabVisible = (index==2);
    ui->actionChange_request_state->setVisible(requestsTabVisible);

    ui->action_Refresh->setVisible(monitorTabVisible || requestsTabVisible);
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
