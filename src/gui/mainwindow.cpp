/* 
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2010-2019 Javier Llorente <javier@opensuse.org>
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

const QString defaultApiUrl = "https://api.opensuse.org";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    obs = new OBS();

    trayIcon = new TrayIcon(this);
    m_notify = false;
    createActions();
    setupTreeMonitor();

    connect(ui->actionChange_request_state, &QAction::triggered, this, &MainWindow::changeRequestState);
    connect(ui->treeRequests, &RequestTreeWidget::changeRequestState, this, &MainWindow::changeRequestState);
    connect(ui->treeRequests, &RequestTreeWidget::descriptionFetched, this, &MainWindow::slotDescriptionFetched);
    connect(ui->treeRequests, &RequestTreeWidget::updateStatusBar, this, &MainWindow::slotUpdateStatusBar);

    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::requestTypeChanged, ui->treeRequests, &RequestTreeWidget::requestTypeChanged);
    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::getIncomingRequests, this, &MainWindow::getIncomingRequests);
    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::getOutgoingRequests, this, &MainWindow::getOutgoingRequests);
    connect(ui->treeRequestBoxes, &RequestBoxTreeWidget::getDeclinedRequests, this, &MainWindow::getDeclinedRequests);

    setupBrowser();
    createStatusBar();

    loginDialog = nullptr;
    errorBox = nullptr;

    createTimer();

    ui->treePackages->setOBS(obs);
    connect(ui->treePackages, SIGNAL(updateStatusBar(QString,bool)), this, SLOT(slotUpdateStatusBar(QString,bool)));

    // Model selection's signals-slots
    projectsSelectionModel = ui->treeProjects->selectionModel();
    connect(projectsSelectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::projectSelectionChanged);

    packagesSelectionModel = ui->treePackages->selectionModel();
    connect(packagesSelectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::packageSelectionChanged);

    filesSelectionModel = ui->treeFiles->selectionModel();
    connect(filesSelectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::fileSelectionChanged);

    ui->hSplitterBrowser->setStretchFactor(1, 1);
    ui->hSplitterBrowser->setStretchFactor(0, 0);

    connect(obs, SIGNAL(apiNotFound(QUrl)), this, SLOT(slotApiNotFound(QUrl)));
    connect(obs, SIGNAL(isAuthenticated(bool)), this, SLOT(isAuthenticated(bool)));
    connect(obs, SIGNAL(selfSignedCertificate(QNetworkReply*)),
            this, SLOT(handleSelfSignedCertificates(QNetworkReply*)));
    connect(obs, SIGNAL(networkError(QString)), this, SLOT(showNetworkError(QString)));

    connect(obs, SIGNAL(finishedParsingAbout(OBSAbout*)), this, SLOT(slotAbout(OBSAbout*)));

    connect(obs, SIGNAL(finishedParsingProjectList(QStringList)), this, SLOT(addProjectList(QStringList)));
    connect(obs, SIGNAL(finishedParsingFile(OBSFile*)), this, SLOT(addFile(OBSFile*)));
    connect(obs, SIGNAL(finishedParsingFileList()), this, SLOT(slotFileListAdded()));

    connect(obs, SIGNAL(finishedParsingBranchPackage(OBSStatus*)),
            this, SLOT(slotBranchPackage(OBSStatus*)));
    connect(obs, SIGNAL(finishedParsingCreateRequest(OBSRequest*)),
            this, SLOT(slotCreateRequest(OBSRequest*)));
    connect(obs, SIGNAL(finishedParsingCreateRequestStatus(OBSStatus*)),
            this, SLOT(slotCreateRequestStatus(OBSStatus*)));

    connect(obs, SIGNAL(finishedParsingUploadFileRevision(OBSRevision*)),
            this, SLOT(slotUploadFile(OBSRevision*)));
    connect(obs, SIGNAL(cannotUploadFile(OBSStatus*)),
            this, SLOT(slotUploadFileError(OBSStatus*)));
    connect(obs, SIGNAL(fileFetched(QString,QByteArray)), this, SLOT(slotFileFetched(QString, QByteArray)));
    connect(obs, SIGNAL(buildLogFetched(QString)), this, SLOT(slotBuildLogFetched(QString)));
    connect(obs, SIGNAL(buildLogNotFound()), this, SLOT(slotBuildLogNotFound()));

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

    connect(obs, &OBS::finishedParsingIncomingRequest, ui->treeRequests, &RequestTreeWidget::addIncomingRequest);
    connect(obs, &OBS::finishedParsingIncomingRequestList, ui->treeRequests, &RequestTreeWidget::irListFetched);
    connect(obs, &OBS::finishedParsingOutgoingRequest, ui->treeRequests, &RequestTreeWidget::addOutgoingRequest);
    connect(obs, &OBS::finishedParsingOutgoingRequestList, ui->treeRequests, &RequestTreeWidget::orListFetched);
    connect(obs, &OBS::finishedParsingDeclinedRequest, ui->treeRequests, &RequestTreeWidget::addDeclinedRequest);
    connect(obs, &OBS::finishedParsingDeclinedRequestList, ui->treeRequests, &RequestTreeWidget::orListFetched);
    connect(obs, SIGNAL(srStatus(QString)), this, SLOT(slotSrStatus(QString)));
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

void MainWindow::slotErrorReadingPassword(const QString &error)
{
    qDebug() << "MainWindow::slotErrorReadingPassword()" << error;
    showLoginDialog();
}

void MainWindow::slotCredentialsRestored(const QString &username, const QString &password)
{
    qDebug() << "MainWindow::slotCredentialsRestored()";
    slotLogin(username, password);
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

void MainWindow::slotApiChanged()
{
    qDebug() << "MainWindow::slotApiChanged()";
    showLoginDialog();
}

void MainWindow::isAuthenticated(bool authenticated)
{
    qDebug() << "MainWindow::isAuthenticated()" << authenticated;
    ui->action_Refresh->setEnabled(authenticated);
    if (authenticated) {
        loadProjects();
        obs->getPerson();
        on_action_Refresh_triggered();
        delete loginDialog;
        loginDialog = nullptr;
    } else {
        emit updateStatusBar(tr("Authentication is required"), true);
        showLoginDialog();
    }

    ui->actionAPI_information->setEnabled(authenticated);
    newButton->setEnabled(authenticated);
    ui->action_Home->setEnabled(authenticated);
    bookmarkButton->setEnabled(authenticated);
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

    connect(ui->treePackages, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(slotContextMenuPackages(QPoint)));

    connect(ui->treeFiles, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(slotContextMenuFiles(QPoint)));

    connect(ui->treeBuildResults, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(slotContextMenuResults(QPoint)));

    connect(ui->treeFiles, SIGNAL(droppedFile(QString)), this, SLOT(uploadFile(QString)));

    proxyModelProjects = static_cast<QSortFilterProxyModel *>(ui->treeProjects->model());

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
    ui->action_Download_file->setEnabled(false);
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
    ui->action_Download_file->setEnabled(false);
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

void MainWindow::setupFileActions()
{
    qDebug() << "MainWindow::setupFileActions()";
    ui->action_Download_file->setEnabled(true);
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

void MainWindow::setupModels()
{
    qDebug() << "MainWindow::setupModels()";
    // Clean up package list, files and results
    ui->treePackages->deleteModel();
    ui->treeFiles->clearModel();
    ui->treeBuildResults->clearModel();

    ui->treePackages->createModel();
    packagesSelectionModel = ui->treePackages->selectionModel();
    connect(packagesSelectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::packageSelectionChanged);
}

void MainWindow::loadProjects()
{
    qDebug() << "MainWindow::loadProjects()";

    setupModels();
    ui->treeFiles->setAcceptDrops(false);
    ui->action_Delete->setEnabled(false);

    emit updateStatusBar(tr("Getting projects..."), false);
    obs->getProjects();
}

void MainWindow::filterResults(const QString &item)
{
    qDebug() << "MainWindow::filterResults())";
    browserFilter->isProjectChecked() ? ui->treeProjects->filterProjects(item) : ui->treePackages->filterPackages(item);

    // Delete  treePackages' model rows when filter doesn't match a project
    if (proxyModelProjects->rowCount()==0 && ui->treePackages->model()->hasChildren()) {
        ui->treePackages->clearModel();
    }
}

void MainWindow::filterRadioButtonClicked(bool)
{
    qDebug() << "MainWindow::filterRadioButtonClicked()";

    // Clear project filter on radio button click
    // if there were no matches for the project name
    if (proxyModelProjects->rowCount()==0 && browserFilter->isPackageChecked()) {
        ui->treeProjects->filterProjects("");
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
    obs->setIncludeHomeProjects(includeHomeProjects);
    obs->getProjects();
    if (browserFilter->isProjectChecked()) {
        ui->treeProjects->filterProjects(browserFilter->getText());
    } else {
        browserFilter->clear();
        ui->treeProjects->filterProjects("");
    }
    setupModels();
    ui->action_Delete->setEnabled(false);
}

void MainWindow::projectSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    qDebug() << "MainWindow::projectSelectionChanged()";
    Q_UNUSED(deselected);

    QString selectedProjectStr;

    // Clean up files and build results on project click
    ui->treeFiles->clearModel();
    ui->treeBuildResults->clearModel();

    if (!selected.isEmpty()) {
        QModelIndex selectedProject = selected.indexes().at(0);
        selectedProjectStr = selectedProject.data().toString();
        getPackages(selectedProject);
        ui->treePackages->filterPackages("");
        setupProjectActions();
        ui->treeFiles->setAcceptDrops(false);
    }

    bookmarks->toggleActions(selectedProjectStr);
}

void MainWindow::packageSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    qDebug() << "MainWindow::packageSelectionChanged()";
    Q_UNUSED(deselected);

    if (!selected.isEmpty()) {
        QModelIndex selectedPackage = selected.indexes().at(0);
        getPackageFiles(selectedPackage);
        setupPackageActions();
        ui->treeFiles->setAcceptDrops(true);
    } else {
        // If there is no package selected, clear both the file and build result lists
        ui->treeFiles->clearModel();
        ui->treeBuildResults->clearModel();

        setupProjectActions();
        ui->treeFiles->setAcceptDrops(false);
    }
}

void MainWindow::fileSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    qDebug() << "MainWindow::fileSelectionChanged()";
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
    setupFileActions();
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
    ui->treeFiles->clearModel();
    ui->treeBuildResults->clearModel();

    setupProjectActions();

    ui->treeFiles->setAcceptDrops(false);
}

void MainWindow::getPackageFiles(QModelIndex index)
{
    qDebug() << "MainWindow::getPackageFiles()";
    emit updateStatusBar(tr("Getting package files.."), false);

    ui->treeFiles->clearModel();

    QString currentProject = ui->treeProjects->getCurrentProject();
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
    getPackageFiles(ui->treePackages->currentIndex());
    setupPackageActions();
}

void MainWindow::getBuildResults()
{
    qDebug() << "MainWindow::getBuildResults()";
    emit updateStatusBar(tr("Getting build results..."), false);

    ui->treeBuildResults->clearModel();
    currentProject = ui->treeProjects->getCurrentProject();
    currentPackage = ui->treePackages->getCurrentPackage();
    obs->getAllBuildStatus(currentProject, currentPackage);
}

void MainWindow::reloadResults()
{
    qDebug() << "MainWindow::reloadResults()";
    getBuildResults();
    setupPackageActions();
}

void MainWindow::getBuildLog()
{
    qDebug() << "MainWindow::getBuildLog()";
    QString currentProject = ui->treeProjects->getCurrentProject();
    QString currentBuildRepository = ui->treeBuildResults->getCurrentRepository();
    QString currentBuildArch = ui->treeBuildResults->getCurrentArch();
    QString currentPackage = ui->treePackages->getCurrentPackage();

    obs->getBuildLog(currentProject, currentBuildRepository, currentBuildArch, currentPackage);
    emit updateStatusBar(tr("Getting build log..."), false);
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
    QMenu *treePackagesMenu = new QMenu(ui->treePackages);
    treePackagesMenu->addAction(actionNew_package);
    treePackagesMenu->addAction(ui->action_Branch_package);
    treePackagesMenu->addAction(action_createRequest);
    treePackagesMenu->addAction(action_ReloadPackages);
    treePackagesMenu->addAction(actionDelete_package);

    QModelIndex index = ui->treePackages->indexAt(point);
    if (index.isValid()) {
        treePackagesMenu->exec(ui->treePackages->mapToGlobal(point));
    }
}

void MainWindow::slotContextMenuFiles(const QPoint &point)
{
    QMenu *treeFilesMenu = new QMenu(ui->treeFiles);
    treeFilesMenu->addAction(ui->action_Upload_file);
    treeFilesMenu->addAction(ui->action_Download_file);
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
    treeResultsMenu->addAction(action_getBuildLog);
    treeResultsMenu->addAction(action_ReloadResults);

    QModelIndex index = ui->treeBuildResults->indexAt(point);
    if (index.isValid()) {
        treeResultsMenu->exec(ui->treeBuildResults->mapToGlobal(point));
    }
}

void MainWindow::changeRequestState()
{
    qDebug() << "MainWindow::changeRequestState()";
    OBSRequest *request = ui->treeRequests->currentRequest();
    RequestStateEditor *reqStateEditor = new RequestStateEditor(this, obs, request);

    disconnect(obs, SIGNAL(finishedParsingResult(OBSResult*)), this, SLOT(addResult(OBSResult*)));
    disconnect(obs, SIGNAL(finishedParsingResult(OBSResult*)), ui->treeMonitor, SLOT(addDroppedPackage(OBSResult*)));

    reqStateEditor->exec();

    delete reqStateEditor;
    delete request;

    connect(obs, SIGNAL(finishedParsingResult(OBSResult*)), ui->treeMonitor, SLOT(addDroppedPackage(OBSResult*)));
    connect(obs, SIGNAL(finishedParsingResult(OBSResult*)), this, SLOT(addResult(OBSResult*)));
}

void MainWindow::slotSrStatus(const QString &status)
{
    qDebug() << "MainWindow::slotSrStatus()";
    if (status=="ok") {
        OBSRequest *request = ui->treeRequests->currentRequest();
        ui->treeRequests->removeIncomingRequest(request->getId());
        ui->textBrowser->clear();
        delete request;
    }
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

void MainWindow::slotEnableRemoveRow()
{
    QList<QModelIndex> list = ui->treeMonitor->selectionModel()->selectedIndexes();

    if (list.isEmpty()) {
        ui->action_Remove->setEnabled(false);
    } else if (!ui->action_Remove->isEnabled()) {
        ui->action_Remove->setEnabled(true);
    }

}

void MainWindow::on_action_Branch_package_triggered()
{
    QString project = ui->treeProjects->getCurrentProject();
    QString package = ui->treePackages->getCurrentPackage();

    const QString title = tr("Branch confirmation");
    const QString text = tr("<b>Source</b><br> %1/%2<br><b>Destination</b><br> home:%3:branches:%4")
                           .arg(project, package, obs->getUsername(), project);

    QMessageBox::StandardButton result = QMessageBox::question(this, title, text,
                                                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        qDebug() << "Branching package..." << project << "/" << package;
        obs->branchPackage(project, package);
        const QString statusText = tr("Branching %1/%2...").arg(project, package);
        emit updateStatusBar(statusText, false);
    }
}

void MainWindow::on_action_Home_triggered()
{
    QString userHomeProject = QString("home:%1").arg(obs->getUsername());
    ui->treeProjects->setCurrentProject(userHomeProject);
}

void MainWindow::on_action_Upload_file_triggered()
{
    QStringList pathList = QFileDialog::getOpenFileNames(this, tr("Upload file"));

    foreach (QString path, pathList) {
        qDebug() << "MainWindow::on_action_Upload_file_triggered() Selected file:" << path;
        uploadFile(path);
    }
}

void MainWindow::on_action_Download_file_triggered()
{
    QString currentProject = ui->treeProjects->getCurrentProject();
    QString currentPackage = ui->treePackages->getCurrentPackage();
    QString currentFile = ui->treeFiles->getCurrentFile();
    obs->downloadFile(currentProject, currentPackage, currentFile);
}

void MainWindow::newProject()
{
    qDebug() << "MainWindow:newProject()";

    QString project = ui->treeProjects->getCurrentProject();

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

    QString project = ui->treeProjects->getCurrentProject();

    CreateDialog *createDialog = new CreateDialog(obs, this);
    createDialog->setProject(project);
    createDialog->exec();
    delete createDialog;
    createDialog = nullptr;
}

void MainWindow::uploadFile(const QString &path)
{
    qDebug() << "MainWindow:uploadFile() path:" << path;

    QString project = ui->treeProjects->getCurrentProject();
    QString package = ui->treePackages->getCurrentPackage();

    if (!project.isEmpty() && !package.isEmpty()) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return;
        }

        QByteArray data = file.readAll();
        qDebug() << "MainWindow::uploadFile() data.size()" << data.size();

        QFileInfo fi(file.fileName());
        QString fileName = fi.fileName();

        obs->uploadFile(project, package, fileName, data);

        QString statusText = tr("Uploading %1 to %2/%3...").arg(fileName, project, package);
        emit updateStatusBar(statusText, false);
    } else {
        qDebug() << "MainWindow:uploadFile() File" << path << "cannot be uploaded. Project/package is empty!";
    }
}

void MainWindow::createRequest()
{
//    FIXME: If there is a _link, set target to project/package from _link
    QString currentProject = ui->treeProjects->getCurrentProject();
    QString currentPackage = ui->treePackages->getCurrentPackage();

    OBSRequest *request = new OBSRequest();
    request->setActionType("submit");
    request->setSourceProject(currentProject);
    request->setSourcePackage(currentPackage);

    if (ui->treeFiles->hasLink()) {
        obs->getLink(currentProject, currentPackage);
    }

    CreateRequestDialog *createRequestDialog = new CreateRequestDialog(request, obs, this);
    createRequestDialog->addProjectList(ui->treeProjects->getProjectList());
    disconnect(obs, &OBS::finishedParsingPackageList,
               ui->treePackages, &PackageTreeWidget::addPackageList);

    int result = createRequestDialog->exec();
    if (result) {
        QString statusText = tr("Creating request...");
        emit updateStatusBar(statusText, false);
    }
    delete createRequestDialog;
    delete request;

    connect(obs, &OBS::finishedParsingPackageList,
               ui->treePackages, &PackageTreeWidget::addPackageList);
}

void MainWindow::deleteProject()
{
    qDebug() << "MainWindow:deleteProject()";

    QString project = ui->treeProjects->getCurrentProject();

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

    QString project = ui->treeProjects->getCurrentProject();
    QString package = ui->treePackages->getCurrentPackage();

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

    QString project = ui->treeProjects->getCurrentProject();
    QString package = ui->treePackages->getCurrentPackage();
    QString fileName = ui->treeFiles->getCurrentFile();

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
    emit updateStatusBar(tr("Getting build statuses..."), false);
    ui->treeMonitor->getBuildStatus();

    emit updateStatusBar(tr("Getting requests..."), false);
    obs->getIncomingRequests();
    obs->getOutgoingRequests();
}

void MainWindow::setupTreeMonitor()
{
    ui->treeMonitor->setOBS(obs);
    connect(ui->action_Add, SIGNAL(triggered(bool)), ui->treeMonitor, SLOT(slotAddRow()));
    connect(ui->treeMonitor, SIGNAL(itemSelectionChanged()), this, SLOT(slotEnableRemoveRow()));
    connect(ui->action_Remove, SIGNAL(triggered(bool)), ui->treeMonitor, SLOT(slotRemoveRow()));
    connect(ui->action_Mark_all_as_read, SIGNAL(triggered(bool)), ui->treeMonitor, SLOT(slotMarkAllRead()));
    connect(ui->treeMonitor, SIGNAL(notifyChanged(bool)), this, SLOT(setNotify(bool)));
    connect(ui->treeMonitor, SIGNAL(updateStatusBar(QString,bool)), this, SLOT(slotUpdateStatusBar(QString,bool)));
}

void MainWindow::addProjectList(const QStringList &projectList)
{
    qDebug() << "MainWindow::addProjectList()";
    ui->treeProjects->addProjectList(projectList);

    QString browserFilterText = browserFilter->getText();
    ui->treeProjects->filterProjects(browserFilterText);

    bookmarks->toggleActions("");

    emit updateStatusBar(tr("Done"), true);
}

void MainWindow::addFile(OBSFile *obsFile)
{
    qDebug() << "MainWindow::addFile()";

    currentProject = ui->treeProjects->getCurrentProject();
    currentPackage = ui->treePackages->getCurrentPackage();
    QString fileProject = obsFile->getProject();
    QString filePackage = obsFile->getPackage();

    if (currentProject==fileProject && currentPackage==filePackage) {
        ui->treeFiles->addFile(obsFile);
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

    currentProject = ui->treeProjects->getCurrentProject();
    currentPackage = ui->treePackages->getCurrentPackage();
    QString resultProject = obsResult->getProject();
    QString resultPackage = obsResult->getStatus()->getPackage();

    if (currentProject==resultProject && currentPackage==resultPackage) {
        ui->treeBuildResults->addResult(obsResult);
    }

//  This slot is in charge of deleting result (last one connected)
    delete obsResult;
    obsResult = nullptr;
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

void MainWindow::slotCreateRequest(OBSRequest *obsRequest)
{
    qDebug() << "MainWindow::slotCreateRequest()";

    QString message = tr("Request created successfully. %1").arg(obsRequest->getId());
    trayIcon->showMessage(APP_NAME, message);
    delete obsRequest;

    emit updateStatusBar(tr("Done"), true);
}

void MainWindow::slotCreateRequestStatus(OBSStatus *obsStatus)
{
    qDebug() << "MainWindow::slotCreateRequestStatus()";

    const QString title = tr("Request failed!");
    const QString text = QString("<b>%1</b><br>%2<br>%3").arg(
                obsStatus->getCode(), obsStatus->getSummary(), obsStatus->getDetails());
    QMessageBox::critical(this, title, text);

    delete obsStatus;

    emit updateStatusBar(tr("Done"), true);
}

void MainWindow::slotUploadFile(OBSRevision *obsRevision)
{
    qDebug() << "MainWindow::slotUploadFile()";
    QString currentProject = ui->treeProjects->getCurrentProject();
    QString currentPackage = ui->treePackages->getCurrentPackage();

    // Refresh file list
    if (currentProject == obsRevision->getProject() && currentPackage == obsRevision->getPackage()) {
        getPackageFiles(ui->treePackages->currentIndex());
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

void MainWindow::slotFileFetched(const QString &fileName, const QByteArray &data)
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save as"), fileName);
    QFile file(path);

    file.open(QIODevice::WriteOnly);
    qint64 bytesWritten = file.write(data);
    file.close();

    if (bytesWritten!=-1) {
        trayIcon->showMessage(APP_NAME, tr("File %1 downloaded successfuly").arg(fileName));
    }
}

void MainWindow::slotBuildLogFetched(const QString &buildLog)
{
    qDebug() << "MainWindow::slotBuildLogFetched()";
    BuildLogViewer *buildLogViewer = new BuildLogViewer(this);
    buildLogViewer->show();
    buildLogViewer->setText(buildLog);
    buildLogViewer->setAttribute(Qt::WA_DeleteOnClose, true);
    emit updateStatusBar(tr("Done"), true);
}

void MainWindow::slotBuildLogNotFound()
{
    qDebug() << "MainWindow::slotBuildLogNotFound()";
    QString title = tr("Information");
    QString text = tr("Build log not found");
    QMessageBox::information(this, title, text);
    emit updateStatusBar(tr("Done"), true);
}

void MainWindow::slotDeleteProject(OBSStatus *obsStatus)
{
    qDebug() << "MainWindow::slotDeleteProject()";

    if (obsStatus->getCode()=="ok") {
        ui->treeProjects->removeProject(obsStatus->getProject());
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
        QString currentProject = ui->treeProjects->getCurrentProject();

        if (obsStatus->getProject()==currentProject) {
            ui->treePackages->removePackage(obsStatus->getPackage());
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
        QString currentProject = ui->treeProjects->getCurrentProject();
        QString currentPackage = ui->treePackages->getCurrentPackage();
        QString fileName = obsStatus->getDetails();

        if (obsStatus->getProject()==currentProject && obsStatus->getPackage()==currentPackage) {

            bool fileRemoved = ui->treeFiles->removeFile(fileName);

            if (fileRemoved) {
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

void MainWindow::getIncomingRequests()
{
    qDebug() << "MainWindow::getIncomingRequests()";
    ui->textBrowser->clear();
    obs->getIncomingRequests();
}

void MainWindow::getOutgoingRequests()
{
    qDebug() << "MainWindow::getOutgoingRequests()";
    ui->textBrowser->clear();
    obs->getOutgoingRequests();
}

void MainWindow::getDeclinedRequests()
{
    qDebug() << "MainWindow::getDeclinedRequests()";
    ui->textBrowser->clear();
    obs->getDeclinedRequests();
}

void MainWindow::slotDescriptionFetched(const QString &description)
{
    ui->textBrowser->setText(description);
    ui->actionChange_request_state->setEnabled(true);
}

void MainWindow::setNotify(bool notify)
{
    qDebug() << "MainWindow::setNotify()" << notify;

    if (notify) {
        ui->action_Mark_all_as_read->setEnabled(true);
    }

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

void MainWindow::slotLogin(const QString &username, const QString &password)
{
    qDebug() << "MainWindow::slotLogin()";
    obs->setCredentials(username, password);
    obs->login();
    emit updateStatusBar(tr("Logging in..."), false);
}

void MainWindow::on_action_Quit_triggered()
{
    qApp->quit();
}

void MainWindow::on_action_About_triggered()
{
    QString title = tr("About ").arg(QCoreApplication::applicationName());
    QString aboutHtml;
    aboutHtml += QString("<big>%1</big>").arg(QCoreApplication::applicationName()) + "<br>";
    aboutHtml += "<b>" + tr("Version %1").arg(QCoreApplication::applicationVersion()) + "</b><br><br>";
    aboutHtml += tr("A Qt-based Open Build Service client")+ "<br>";
    aboutHtml += "<div align=\"left\">&copy; 2010-2019 Javier Llorente</div>";
    aboutHtml += "<br><a href='https://github.com/javierllorente/qactus'>https://github.com/javierllorente/qactus</a> </p>";
    aboutHtml += "<p><b>" + tr("License") + "</b><br>";
    aboutHtml += "<nobr>" + tr("This program is under the GPLv3") + "</nobr></p></div>";
    aboutHtml += "<small>" + tr("Qt version: %1").arg(qVersion()) + "</small>";

    QMessageBox::about(this, title, aboutHtml);
}

void MainWindow::createActions()
{
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
    newButton->setEnabled(false);

    actionNew = ui->toolBar->insertWidget(ui->action_Branch_package, newButton);
    connect(newButton, SIGNAL(clicked(bool)), this, SLOT(newPackage()));

    // WatchList actions
    bookmarkButton = new QToolButton(this);
    bookmarkButton->setPopupMode(QToolButton::InstantPopup);
    bookmarkButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    bookmarkButton->setText(tr("&Bookmarks"));
    bookmarkButton->setIcon(QIcon::fromTheme("bookmarks"));

    bookmarks = new Bookmarks(bookmarkButton);
    bookmarkButton->setMenu(bookmarks);
    bookmarkButton->setEnabled(false);
    connect(obs, &OBS::finishedParsingPerson, bookmarks, &Bookmarks::slotLoadBookmarks);
    connect(bookmarks, &Bookmarks::clicked, ui->treeProjects, &ProjectTreeWidget::setCurrentProject);
    connect(bookmarks, &Bookmarks::bookmarkAdded, this, &MainWindow::slotUpdatePerson);
    connect(bookmarks, &Bookmarks::bookmarkDeleted, this, &MainWindow::slotUpdatePerson);
    connect(bookmarks, &Bookmarks::addBookmarkClicked, bookmarks, [=](){
        bookmarks->addBookmark(ui->treeProjects->getCurrentProject());
    });
    connect(bookmarks, &Bookmarks::deleteBookmarkClicked, bookmarks, [=](){
        bookmarks->deleteBookmark(ui->treeProjects->getCurrentProject());
    });

    actionBookmarks = ui->toolBar->insertWidget(ui->action_Upload_file, bookmarkButton);
    separatorHome = ui->toolBar->insertSeparator(ui->action_Home);
    separatorUpload = ui->toolBar->insertSeparator(ui->action_Upload_file);

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

    // Get build log action
    action_getBuildLog = new QAction(tr("&Get build log"), this);
    action_getBuildLog->setIcon(QIcon::fromTheme("text-x-log"));
    connect(action_getBuildLog, SIGNAL(triggered(bool)), this, SLOT(getBuildLog()));

    // Create request action
    action_createRequest = new QAction(tr("&Submit package"), this);
    action_createRequest->setIcon(QIcon::fromTheme("cloud-upload"));
    connect(action_createRequest, SIGNAL(triggered(bool)), this, SLOT(createRequest()));

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
}

void MainWindow::readSettings()
{
    qDebug() << "MainWindow::readSettings()";
    readMWSettings();
    readProxySettings();
    readAuthSettings();
    readBrowserSettings();
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

void MainWindow::readAuthSettings()
{
    qDebug() << "MainWindow::readAuthSettings()";
    QSettings settings;
    settings.beginGroup("Auth");
    QString apiUrl = settings.value("ApiUrl").toString();
    if (apiUrl.isEmpty()) {
        apiUrl = defaultApiUrl;
        settings.setValue("ApiUrl", defaultApiUrl);
    }
    obs->setApiUrl(apiUrl);
    if (settings.value("AutoLogin").toBool()) {
        Credentials *credentials = new Credentials(this);
        connect(credentials, SIGNAL(errorReadingPassword(QString)),
                this, SLOT(slotErrorReadingPassword(QString)));
        connect(credentials, SIGNAL(credentialsRestored(QString, QString)),
                this, SLOT(slotCredentialsRestored(QString, QString)));
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
    obs->setIncludeHomeProjects(includeHomeProjects);
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

void MainWindow::slotApiNotFound(QUrl url)
{
    qDebug() << " MainWindow::slotApiNotFound()";
    const QString title = tr("Error");
    const QString text = QString(tr("OBS API not found at<br>%1<br>"
                                    "Please check the URL and retry")).arg(url.toString());
    QMessageBox::critical(this, title, text);

    emit updateStatusBar(tr("OBS API not found at ") + url.toString(), true);
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
    if (!loginDialog) {
        loginDialog = new Login(this);
        connect(loginDialog, SIGNAL(login(QString,QString)), this, SLOT(slotLogin(QString,QString)));
    }
    loginDialog->show();
}

void MainWindow::on_action_Configure_Qactus_triggered()
{
    qDebug() << "MainWindow Launching Configure...";
    Configure *configure = new Configure(this, obs);
    connect(configure, SIGNAL(apiChanged()), this, SLOT(slotApiChanged()));
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

void MainWindow::slotUpdatePerson(OBSPerson *obsPerson)
{
    OBSXmlWriter *xmlWriter = new OBSXmlWriter(this);
    QByteArray data = xmlWriter->createPerson(obsPerson);
    delete xmlWriter;
    obs->updatePerson(data);
}

void MainWindow::on_iconBar_currentRowChanged(int index)
{
    // Enable/disable the branch/delete button if there is a file/package/project selected
    QItemSelectionModel *treeFilesSelectionModel = ui->treeFiles->selectionModel();
    if (treeFilesSelectionModel) {
        QList<QModelIndex> selectedFiles = treeFilesSelectionModel->selectedIndexes();
        bool enable = !selectedFiles.isEmpty();
        ui->action_Download_file->setEnabled(enable);
        actionDelete_file->setEnabled(enable);
    } else {
        ui->action_Download_file->setEnabled(false);
        actionDelete_file->setEnabled(false);
    }

    QItemSelectionModel *treePackagesSelectionModel = ui->treePackages->selectionModel();
    if (treePackagesSelectionModel) {
        QList<QModelIndex> selectedPackages = treePackagesSelectionModel->selectedIndexes();
        bool enable = !selectedPackages.isEmpty();
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
    separatorHome->setVisible(browserTabVisible);
    ui->action_Home->setVisible(browserTabVisible);
    actionBookmarks->setVisible(browserTabVisible);
    separatorUpload->setVisible(browserTabVisible);
    ui->action_Upload_file->setVisible(browserTabVisible);
    ui->action_Download_file->setVisible(browserTabVisible);
    ui->action_Delete->setVisible(browserTabVisible);
    ui->action_Delete->setShortcut(browserTabVisible ? QKeySequence::Delete : QKeySequence());
    actionFilterSpacer->setVisible(browserTabVisible);
    actionFilter->setVisible(browserTabVisible);

    bool monitorTabVisible = (index==1);
    ui->action_Add->setVisible(monitorTabVisible);
    ui->action_Remove->setVisible(monitorTabVisible);
    ui->action_Remove->setShortcut(monitorTabVisible ? QKeySequence::Delete : QKeySequence());
    ui->action_Mark_all_as_read->setVisible(monitorTabVisible);

    bool requestsTabVisible = (index==2);
    ui->actionChange_request_state->setVisible(requestsTabVisible);

    ui->action_Refresh->setVisible(monitorTabVisible || requestsTabVisible);
}

void MainWindow::on_tabWidgetPackages_currentChanged(int index)
{
    QItemSelectionModel *treeFilesSelectionModel = ui->treeFiles->selectionModel();
    if (treeFilesSelectionModel) {
        QList<QModelIndex> selectedFiles = treeFilesSelectionModel->selectedIndexes();
        if (index==0 && !selectedFiles.isEmpty()) {
            setupFileActions();
        }
    }
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
