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
    connect(ui->treeRequests, SIGNAL(customContextMenuRequested(const QPoint&)),this,
            SLOT(showContextMenu(const QPoint&)));
    ui->treeRequests->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(obs, SIGNAL(isAuthenticated(bool)), this, SLOT(isAuthenticated(bool)));
    connect(obs, SIGNAL(selfSignedCertificate(QNetworkReply*)),
            this, SLOT(handleSelfSignedCertificates(QNetworkReply*)));
    connect(obs, SIGNAL(networkError(QString)), this, SLOT(showNetworkError(QString)));

    connect(obs, SIGNAL(finishedParsingAbout(OBSAbout*)), this, SLOT(slotAbout(OBSAbout*)));

    connect(obs, SIGNAL(projectListIsReady()), this, SLOT(insertProjectList()));
    connect(obs, SIGNAL(packageListIsReady()), this, SLOT(insertPackageList()));
    connect(obs, SIGNAL(finishedParsingFile(OBSFile*)), this, SLOT(insertFile(OBSFile*)));
    connect(obs, SIGNAL(finishedParsingPackage(OBSPackage*,int)),
            this, SLOT(insertBuildStatus(OBSPackage*, const int)));

    connect(obs, SIGNAL(finishedParsingStatus(OBSStatus*)),
            this, SLOT(slotReceivedStatus(OBSStatus*)));
    connect(obs, SIGNAL(finishedParsingDeletePrjStatus(OBSStatus*)),
            this, SLOT(slotDeleteProject(OBSStatus*)));
    connect(obs, SIGNAL(finishedParsingDeletePkgStatus(OBSStatus*)),
            this, SLOT(slotDeletePackage(OBSStatus*)));

    connect(obs, SIGNAL(cannotDeleteProject(OBSStatus*)),
            this, SLOT(slotDeleteProject(OBSStatus*)));
    connect(obs, SIGNAL(cannotDeletePackage(OBSStatus*)),
            this, SLOT(slotDeletePackage(OBSStatus*)));

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

    sourceModelProjects = new QStringListModel(ui->treeProjects);
    proxyModelProjects = new QSortFilterProxyModel(ui->treeProjects);
    sourceModelBuilds = new QStringListModel(ui->treeBuilds);
    proxyModelBuilds = nullptr;
    sourceModelFiles = nullptr;
    sourceModelBuildResults = nullptr;

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

    ui->action_Branch_package->setEnabled(false);
    actionDelete_package->setEnabled(false);
    actionDelete_project->setEnabled(true);
    actionDelete->setEnabled(true);
}

void MainWindow::buildSelectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
    qDebug() << "MainWindow::buildSelectionChanged()";
    getPackageFiles(ui->treeBuilds->currentIndex());

    ui->action_Branch_package->setEnabled(true);
    actionDelete_package->setEnabled(true);
    actionDelete->setEnabled(true);
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
    actionChangeRequestState->setIcon(QIcon::fromTheme("mail-reply-sender"));
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

    if (ui->iconBar->currentRow()==1) {
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

void MainWindow::slotReceivedStatus(OBSStatus *obsStatus)
{
    qDebug() << "MainWindow::slotReceivedStatus()";

    if (obsStatus->getCode()!="ok") {
        const QString title = tr("Warning");
        const QString text = obsStatus->getSummary() + "<br>" + obsStatus->getDetails();
        QMessageBox::warning(this, title, text);
    }

    loadProjects();
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

    delete obsStatus;
    obsStatus = nullptr;

    emit updateStatusBar(tr("Done"), true);
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

void MainWindow::updateStatusBarSlot(const QString &message, bool progressBarHidden)
{
    qDebug() << "MainWindow::updateStatusBarSlot()";
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

    // Delete button actions
    deleteButton = new QToolButton(this);
    deleteButton->setPopupMode(QToolButton::InstantPopup);
    deleteButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    deleteButton->setText(tr("&Delete"));
    deleteButton->setIcon(QIcon::fromTheme("trash-empty"));

    deleteMenu = new QMenu(deleteButton);
    actionDelete_project = new QAction(tr("Pro&ject"), this);
    actionDelete_project->setIcon(QIcon::fromTheme("project-development"));
    connect(actionDelete_project, SIGNAL(triggered(bool)), this, SLOT(deleteProject()));

    actionDelete_package = new QAction(tr("Pac&kage"), this);
    actionDelete_package->setIcon(QIcon::fromTheme("application-x-source-rpm"));
    connect(actionDelete_package, SIGNAL(triggered(bool)), this, SLOT(deletePackage()));

    deleteMenu->addAction(actionDelete_project);
    deleteMenu->addAction(actionDelete_package);
    deleteButton->setMenu(deleteMenu);

    actionDelete = ui->toolBar->addWidget(deleteButton);
    actionDelete->setVisible(false);

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
    // Enable/Disable the branch package button if there is a build selected
    QItemSelectionModel *treeBuildsSelectionModel = ui->treeBuilds->selectionModel();
    if (treeBuildsSelectionModel) {
        QList<QModelIndex> selectedBuilds = treeBuildsSelectionModel->selectedIndexes();
        bool enable = !selectedBuilds.isEmpty();
        ui->action_Branch_package->setEnabled(enable);
        actionDelete_package->setEnabled(enable);
    } else {
        ui->action_Branch_package->setEnabled(false);
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

    bool enableactionDelete = actionDelete_project->isEnabled() || actionDelete_package->isEnabled();
    actionDelete->setEnabled(enableactionDelete);

    bool browserTabVisible = (index==0);
    ui->action_Branch_package->setVisible(browserTabVisible);
    actionDelete->setVisible(browserTabVisible);
    actionFilterSpacer->setVisible(browserTabVisible);
    actionFilter->setVisible(browserTabVisible);

    bool monitorTabVisible = (index==1);
    ui->action_Add->setVisible(monitorTabVisible);
    ui->action_Remove->setVisible(monitorTabVisible);
    ui->action_Mark_all_as_read->setVisible(monitorTabVisible);

    bool requestsTabVisible = (index==2);
    ui->actionChange_request_state->setVisible(requestsTabVisible);
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
