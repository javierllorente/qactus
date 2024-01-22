/*
 * Copyright (C) 2019-2024 Javier Llorente <javier@opensuse.org>
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
#include "browser.h"
#include "ui_browser.h"
#include <QFileDialog>
#include <QSettings>
#include "metaconfigeditor.h"
#include "createrequestdialog.h"
#include "packageactiondialog.h"
#include "buildlogviewer.h"

Browser::Browser(QWidget *parent, BrowserFilter *browserFilter, OBS *obs) :
    QWidget(parent),
    ui(new Ui::Browser),
    m_browserFilter(browserFilter),
    m_obs(obs),
    m_packagesMenu(nullptr),
    m_filesMenu(nullptr),
    m_resultsMenu(nullptr),
    m_packagesToolbar(new QToolBar(this)),
    m_filesToolbar(new QToolBar(this)),
    m_resultsToolbar(new QToolBar(this))
{
    ui->setupUi(this);

    ui->hSplitterBrowser->setSizes((QList<int>({160, 400})));
    ui->hSplitterBrowser->setStretchFactor(1, 1);
    ui->hSplitterBrowser->setStretchFactor(0, 1);

    QIcon filterIcon(QIcon::fromTheme("view-filter"));
    ui->lineEditFilter->addAction(filterIcon, QLineEdit::LeadingPosition);

    m_packagesToolbar->setIconSize(QSize(15, 15));
    m_filesToolbar->setIconSize(QSize(15, 15));
    m_resultsToolbar->setIconSize(QSize(15, 15));

    ui->verticalLayout_6->insertWidget(2, m_packagesToolbar);
    ui->verticalLayout_4->addWidget(m_filesToolbar);
    ui->verticalLayout_2->addWidget(m_resultsToolbar);

    connect(m_obs, &OBS::finishedParsingProjectList, this, &Browser::addProjectList);
    connect(m_browserFilter, &BrowserFilter::setCurrentProject, this, &Browser::setCurrentProject);

    connect(m_obs, &OBS::finishedParsingFile, this, &Browser::addFile);
    connect(m_obs, &OBS::finishedParsingFileList, ui->treeFiles, &FileTreeWidget::filesAdded);
    connect(m_obs, &OBS::finishedParsingUploadFileRevision, this, &Browser::slotUploadFile);
    connect(m_obs, &OBS::cannotUploadFile, this, &Browser::slotUploadFileError);
    connect(m_obs, &OBS::fileFetched, this, &Browser::slotFileFetched);
    connect(ui->treeFiles, &FileTreeWidget::updateStatusBar, this, &Browser::updateStatusBar);

    connect(m_obs, &OBS::finishedParsingCreateRequest, this, &Browser::slotCreateRequest);
    connect(m_obs, &OBS::finishedParsingCreateRequestStatus, this, &Browser::slotCreateRequestStatus);
    connect(m_obs, &OBS::finishedParsingBranchPackage, this, &Browser::slotBranchPackage);

    connect(m_obs, &OBS::buildLogFetched, this, &Browser::slotBuildLogFetched);
    connect(m_obs, &OBS::buildLogNotFound, this, &Browser::slotBuildLogNotFound);
    connect(m_obs, &OBS::projectNotFound, this, &Browser::slotProjectNotFound);
    connect(m_obs, &OBS::packageNotFound, this, &Browser::slotPackageNotFound);

    connect(m_obs, &OBS::finishedParsingCreatePrjStatus, this, [=](OBSStatus *status) {
        if (status->getCode() == "ok") {
            m_browserFilter->addProject(status->getProject());
            setCurrentProject(status->getProject());
            showTrayMessage(APP_NAME, tr("Project %1 has been created").arg(status->getProject()));
        }
    });

    connect(m_obs, &OBS::finishedParsingCreatePkgStatus, this, [=](OBSStatus *status) {
        if (status->getCode()=="ok") {
            ui->treePackages->addPackage(status->getPackage());
            ui->treePackages->setCurrentPackage(status->getPackage());
            showTrayMessage(APP_NAME, tr("Package %1 has been created").arg(status->getPackage()));
        }
    });

    connect(m_obs, &OBS::finishedParsingDeletePrjStatus, this, &Browser::slotDeleteProject);
    connect(m_obs, &OBS::finishedParsingDeletePkgStatus, this, &Browser::slotDeletePackage);
    connect(m_obs, &OBS::finishedParsingDeleteFileStatus, this, &Browser::slotDeleteFile);

    connect(m_obs, &OBS::cannotDeleteProject, this, &Browser::slotDeleteProject);
    connect(m_obs, &OBS::cannotDeletePackage, this, &Browser::slotDeletePackage);
    connect(m_obs, &OBS::cannotDeleteFile, this, &Browser::slotDeleteFile);

    connect(ui->treePackages, &PackageTreeWidget::customContextMenuRequested, this, &Browser::slotContextMenuPackages);
    connect(ui->treeFiles, &FileTreeWidget::customContextMenuRequested, this, &Browser::slotContextMenuFiles);
    connect(ui->treeBuildResults, &BuildResultTreeWidget::customContextMenuRequested, this,&Browser::slotContextMenuResults);

    connect(ui->treeFiles, &FileTreeWidget::droppedFile, this, &Browser::uploadFile);

    connect(m_obs, &OBS::finishedParsingPackageList, ui->treePackages, &PackageTreeWidget::addPackageList);
    connect(ui->treePackages, &PackageTreeWidget::updateStatusBar, this, &Browser::updateStatusBar);

    connect(m_obs, &OBS::finishedParsingResult, this, &Browser::addResult);
    connect(m_obs, &OBS::finishedParsingResultList, ui->treeBuildResults, &BuildResultTreeWidget::finishedAddingResults);
    connect(m_obs, &OBS::finishedParsingResultList, this, &Browser::finishedAddingResults);

    // Model selection's signals-slots
    packagesSelectionModel = ui->treePackages->selectionModel();
    connect(packagesSelectionModel, &QItemSelectionModel::selectionChanged, this, &Browser::slotPackageSelectionChanged);
    connect(packagesSelectionModel, &QItemSelectionModel::selectionChanged, this, &Browser::packageSelectionChanged);

    connect(m_obs, &OBS::finishedParsingPackageMetaConfig, this, &Browser::slotPkgMetaConfigFetched);

    filesSelectionModel = ui->treeFiles->selectionModel();
    connect(filesSelectionModel, &QItemSelectionModel::selectionChanged, this, &Browser::fileSelectionChanged);

    QItemSelectionModel *buildResultsSelectionModel = ui->treeBuildResults->selectionModel();
    connect(buildResultsSelectionModel, &QItemSelectionModel::selectionChanged, this, &Browser::buildResultSelectionChanged);

    connect(ui->lineEditFilter, &QLineEdit::textChanged, ui->treePackages, &PackageTreeWidget::filterPackages);

    readSettings();
}

Browser::~Browser()
{
    writeSettings();
    delete ui;
}

void Browser::readSettings()
{
    QSettings settings;
    settings.beginGroup("Browser");
    bool includeHomeProjects = settings.value("IncludeHomeProjects").toBool();
    m_obs->setIncludeHomeProjects(includeHomeProjects);
    ui->hSplitterBrowser->restoreState(settings.value("horizontalSplitterSizes").toByteArray());
    ui->vSplitterBrowser->restoreState(settings.value("verticalSplitterSizes").toByteArray());
    settings.endGroup();
}

void Browser::addProjectList(const QStringList &projectList)
{ 
    qDebug() << __PRETTY_FUNCTION__;
    m_browserFilter->addProjectList(projectList);

    emit toggleBookmarkActions("");
    emit finishedLoadingProjects();
    emit updateStatusBar(tr("Done"), true);
}

void Browser::createPackagesContextMenu(QMenu *packagesMenu)
{
    m_packagesMenu = packagesMenu;
    m_packagesToolbar->addActions(m_packagesMenu->actions());
}

void Browser::createFilesContextMenu(QMenu *filesMenu)
{
    m_filesMenu = filesMenu;
    m_filesToolbar->addActions(m_filesMenu->actions());
}

void Browser::createResultsContextMenu(QMenu *resultsMenu)
{
    m_resultsMenu = resultsMenu;
    m_resultsToolbar->addActions(m_resultsMenu->actions());
}

bool Browser::hasProjectSelection()
{
    return (!currentProject.isEmpty());
}

bool Browser::hasPackageSelection()
{
    QItemSelectionModel *treePackagesSelectionModel = ui->treePackages->selectionModel();
    if (treePackagesSelectionModel) {
        return treePackagesSelectionModel->hasSelection();
    } else {
        return false;
    }
}

bool Browser::hasFileSelection()
{
    QItemSelectionModel *treeFilesSelectionModel = ui->treeFiles->selectionModel();
    if (treeFilesSelectionModel) {
        return treeFilesSelectionModel->hasSelection();
    } else {
        return false;
    }
}

bool Browser::hasBuildResultSelection()
{
    QItemSelectionModel *treeBuildResultsSelectionModel = ui->treeBuildResults->selectionModel();
    if (treeBuildResultsSelectionModel) {
        return treeBuildResultsSelectionModel->hasSelection();
    } else {
        return false;
    }
}

void Browser::setPackageFilterFocus()
{
    ui->lineEditFilter->setFocus();
}

QString Browser::packageFilterText() const
{
    return ui->lineEditFilter->text();
}

void Browser::clearPackageFilter()
{
    ui->lineEditFilter->clear();
}

void Browser::clearOverview()
{
    ui->title->clear();
    ui->link->clear();
    ui->description->clear();
}

void Browser::newProject()
{
    qDebug() << __PRETTY_FUNCTION__;
    MetaConfigEditor *metaConfigEditor = new MetaConfigEditor(this, m_obs, currentProject, "", MCEMode::CreateProject);
    metaConfigEditor->exec();
    delete metaConfigEditor;
}

void Browser::newPackage()
{
    qDebug() << __PRETTY_FUNCTION__;
    MetaConfigEditor *metaConfigEditor = new MetaConfigEditor(this, m_obs, currentProject, "", MCEMode::CreatePackage);
    metaConfigEditor->exec();
    delete metaConfigEditor;
}

void Browser::editProject()
{
    qDebug() << __PRETTY_FUNCTION__;
    MetaConfigEditor *metaConfigEditor = new MetaConfigEditor(this, m_obs, currentProject, "", MCEMode::EditProject);
    metaConfigEditor->exec();
    delete metaConfigEditor;
}

void Browser::editPackage()
{
    qDebug() << __PRETTY_FUNCTION__;
    QString package = ui->treePackages->getCurrentPackage();
    MetaConfigEditor *metaConfigEditor = new MetaConfigEditor(this, m_obs, currentProject, package, MCEMode::EditPackage);
    metaConfigEditor->exec();
    delete metaConfigEditor;
}

void Browser::reloadPackages()
{
    qDebug() << __PRETTY_FUNCTION__;
    getPackages(currentProject);

    // Clean up package files and results
    ui->treeFiles->clearModel();
    ui->treeBuildResults->clearModel();

    emit packageSelectionChanged();
    ui->treeFiles->setAcceptDrops(false);
}

void Browser::reloadFiles()
{
    qDebug() << __PRETTY_FUNCTION__;
    getPackageFiles(ui->treePackages->getCurrentPackage());
    emit packageSelectionChanged();
}

void Browser::addResult(OBSResult *result)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (!result->getStatusList().isEmpty()) {
        currentPackage = ui->treePackages->getCurrentPackage();
        QString resultProject = result->getProject();
        QString resultPackage = result->getStatusList().first()->getPackage();

        if (currentProject == resultProject && currentPackage == resultPackage) {
            ui->treeBuildResults->addResult(result);
        }
    }
}

void Browser::reloadResults()
{
    qDebug() << __PRETTY_FUNCTION__;
    getBuildResults(currentProject, ui->treePackages->getCurrentPackage());
    emit packageSelectionChanged();
}

void Browser::getBuildLog()
{
    qDebug() << __PRETTY_FUNCTION__;
    QString currentBuildRepository = ui->treeBuildResults->getCurrentRepository();
    QString currentBuildArch = ui->treeBuildResults->getCurrentArch();
    QString currentPackage = ui->treePackages->getCurrentPackage();

    m_obs->getBuildLog(currentProject, currentBuildRepository, currentBuildArch, currentPackage);
    emit updateStatusBar(tr("Getting build log..."), false);
}

void Browser::branchSelectedPackage()
{
    QString package = ui->treePackages->getCurrentPackage();

    const QString title = tr("Branch confirmation");
    const QString text = tr("<b>Source</b><br> %1/%2<br><b>Destination</b><br> home:%3:branches")
                           .arg(currentProject, package, m_obs->getUsername());

    QMessageBox::StandardButton result = QMessageBox::question(this, title, text,
                                                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        qDebug() << "Branching package..." << currentProject << "/" << package;
        m_obs->branchPackage(currentProject, package);
        const QString statusText = tr("Branching %1/%2...").arg(currentProject, package);
        emit updateStatusBar(statusText, false);
    }
}

void Browser::getProjects()
{
    qDebug() << __PRETTY_FUNCTION__;
    setupModels();
    ui->treeFiles->setAcceptDrops(false);
    m_browserFilter->clear();
    clearOverview();
    currentProject = "";
    emit projectSelectionChanged();

    emit updateStatusBar(tr("Getting projects..."), false);
    m_obs->getProjects();
}

void Browser::goHome()
{
    QString userHomeProject = QString("home:%1").arg(m_obs->getUsername());
    setCurrentProject(userHomeProject);
}

QString Browser::getCurrentProject() const
{
    return currentProject;
}

void Browser::setCurrentProject(const QString &project)
{
    qDebug() << __PRETTY_FUNCTION__;
    clearOverview();
    if (!project.isEmpty()) {
        m_browserFilter->setText(project);
        getPackages(project);
        emit toggleBookmarkActions(project);
    }

    emit projectSelectionChanged();
}

void Browser::downloadFile()
{
    QString currentPackage = ui->treePackages->getCurrentPackage();
    QString currentFile = ui->treeFiles->getCurrentFile();
    m_obs->downloadFile(currentProject, currentPackage, currentFile);
}

void Browser::uploadSelectedFile()
{
    QStringList pathList = QFileDialog::getOpenFileNames(this, tr("Upload file"));

    foreach (QString path, pathList) {
        qDebug() << __PRETTY_FUNCTION__  << "path:" << path;
        uploadFile(path);
    }
}

void Browser::createRequest()
{
//    FIXME: If there is a _link, set target to project/package from _link
    QString currentPackage = ui->treePackages->getCurrentPackage();

    OBSRequest *request = new OBSRequest();
    request->setActionType("submit");
    request->setSourceProject(currentProject);
    request->setSourcePackage(currentPackage);

    if (ui->treeFiles->hasLink()) {
        m_obs->getLink(currentProject, currentPackage);
    }

    CreateRequestDialog *createRequestDialog = new CreateRequestDialog(request, m_obs, this);
    createRequestDialog->addProjectList(m_browserFilter->getProjectList());
    disconnect(m_obs, &OBS::finishedParsingPackageList,
               ui->treePackages, &PackageTreeWidget::addPackageList);

    int result = createRequestDialog->exec();
    if (result) {
        QString statusText = tr("Creating request...");
        emit updateStatusBar(statusText, false);
    }
    delete createRequestDialog;
    delete request;

    connect(m_obs, &OBS::finishedParsingPackageList,
            ui->treePackages, &PackageTreeWidget::addPackageList);
}

void Browser::linkPackage()
{
    qDebug() << __PRETTY_FUNCTION__;
    PackageActionDialog *packageActionDialog = new PackageActionDialog(this, m_obs,
                                                                       currentProject,
                                                                       ui->treePackages->getCurrentPackage(),
                                                                       PackageAction::LinkPackage);
    packageActionDialog->addProjectList(m_browserFilter->getProjectList());
    connect(packageActionDialog, &PackageActionDialog::showTrayMessage, this, &Browser::showTrayMessage);
    connect(packageActionDialog, &PackageActionDialog::updateStatusBar, this, &Browser::updateStatusBar);
    packageActionDialog->exec();
    delete packageActionDialog;
}

void Browser::copyPackage()
{
    qDebug() << __PRETTY_FUNCTION__;
    PackageActionDialog *packageActionDialog = new PackageActionDialog(this, m_obs,
                                                                       currentProject,
                                                                       ui->treePackages->getCurrentPackage(),
                                                                       PackageAction::CopyPackage);
    packageActionDialog->addProjectList(m_browserFilter->getProjectList());
    connect(packageActionDialog, &PackageActionDialog::showTrayMessage, this, &Browser::showTrayMessage);
    connect(packageActionDialog, &PackageActionDialog::updateStatusBar, this, &Browser::updateStatusBar);
    packageActionDialog->exec();
    delete packageActionDialog;
}

void Browser::deleteProject()
{
    qDebug() << __PRETTY_FUNCTION__;

    const QString title = tr("Delete confirmation");
    const QString text = tr("Do you really want to delete project<br> %1?")
                           .arg(currentProject);

    QMessageBox::StandardButton result = QMessageBox::question(this, title, text,
                                                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        qDebug() << __PRETTY_FUNCTION__ << "Deleting project" << currentProject << "...";
        m_obs->deleteProject(currentProject);
        const QString statusText = tr("Deleting %1...").arg(currentProject);
        emit updateStatusBar(statusText, false);
    }
}

void Browser::deletePackage()
{
    qDebug() << __PRETTY_FUNCTION__;

    QString package = ui->treePackages->getCurrentPackage();

    const QString title = tr("Delete confirmation");
    const QString text = tr("Do you really want to delete package<br> %1/%2?")
                           .arg(currentProject, package);

    QMessageBox::StandardButton result = QMessageBox::question(this, title, text,
                                                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        qDebug() << "Deleting package" << package << "...";
        m_obs->deletePackage(currentProject, package);
        const QString statusText = tr("Deleting %1...").arg(package);
        emit updateStatusBar(statusText, false);
    }
}

void Browser::deleteFile()
{
    qDebug() << __PRETTY_FUNCTION__;

    QString package = ui->treePackages->getCurrentPackage();
    QString fileName = ui->treeFiles->getCurrentFile();

    const QString title = tr("Delete confirmation");
    const QString text = tr("Do you really want to delete file<br> %1/%2/%3?")
                           .arg(currentProject, package, fileName);

    QMessageBox::StandardButton result = QMessageBox::question(this, title, text,
                                                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        qDebug() << "Deleting file" << fileName << "...";
        m_obs->deleteFile(currentProject, package, fileName);
        const QString statusText = tr("Deleting %1...").arg(fileName);
        emit updateStatusBar(statusText, false);
    }
}

void Browser::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Browser");
    settings.setValue("horizontalSplitterSizes", ui->hSplitterBrowser->saveState());
    settings.setValue("verticalSplitterSizes", ui->vSplitterBrowser->saveState());
    settings.endGroup();
}

void Browser::setupModels()
{
    qDebug() << __PRETTY_FUNCTION__;
    // Clean up package list, files and results
    ui->treePackages->deleteModel();
    ui->treeFiles->clearModel();
    ui->treeBuildResults->clearModel();

    ui->treePackages->createModel();
    packagesSelectionModel = ui->treePackages->selectionModel();
    connect(packagesSelectionModel, &QItemSelectionModel::selectionChanged, this, &Browser::slotPackageSelectionChanged);
}

void Browser::slotContextMenuPackages(const QPoint &point)
{
    QModelIndex index = ui->treePackages->indexAt(point);
    if (index.isValid() && m_packagesMenu) {
        m_packagesMenu->exec(ui->treePackages->mapToGlobal(point));
    }
}

void Browser::getPackages(const QString &project)
{
    qDebug() << __PRETTY_FUNCTION__ << project;
    if (!project.isEmpty()) {
        emit updateStatusBar(tr("Getting packages..."), false);
        m_obs->getPackages(project);
        currentProject = project;
    }
}

void Browser::slotPackageSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_UNUSED(deselected)

    if (!selected.isEmpty()) {
        QModelIndex selectedPackage = selected.indexes().at(0);
        currentPackage = ui->treePackages->getCurrentPackage();

        m_obs->getPackageMetaConfig(currentProject, currentPackage);
        getPackageFiles(selectedPackage.data().toString());
        getBuildResults(currentProject, currentPackage);
        emit packageSelectionChanged();
        ui->treeFiles->setAcceptDrops(true);
    } else {
        // If there is no package selected, clear both the file and build result lists
        ui->treeFiles->clearModel();
        ui->treeBuildResults->clearModel();

        emit projectSelectionChanged();
        ui->treeFiles->setAcceptDrops(false);
    }
}

void Browser::slotPkgMetaConfigFetched(OBSPkgMetaConfig *pkgMetaConfig)
{
    qDebug() << __PRETTY_FUNCTION__;
    QString title = pkgMetaConfig->getTitle();
    if (title.isEmpty()) {
        title = pkgMetaConfig->getName();
    }
    ui->title->setText(title);

    QString url = pkgMetaConfig->getUrl().toString();
    ui->link->setText(!url.isEmpty() ? "<a href=\"" + url + "\">" + url + "</a>" : "");
    ui->link->setVisible(!url.isEmpty());
    QString description = pkgMetaConfig->getDescription();
    if (description.isEmpty()) {
        description = "No description set";
    }
    ui->description->setText(description);
}

void Browser::getPackageFiles(const QString &package)
{
    qDebug() << __PRETTY_FUNCTION__;
    emit updateStatusBar(tr("Getting package files.."), false);

    ui->treeFiles->clearModel();

    m_obs->getFiles(currentProject, package);
    emit updateStatusBar(tr("Getting package data..."), false);
}

void Browser::getBuildResults(const QString &project, const QString &package)
{
    qDebug() << __PRETTY_FUNCTION__;
    emit updateStatusBar(tr("Getting build results..."), false);

    ui->treeBuildResults->clearModel();
    m_obs->getAllBuildStatus(project, package);
}

void Browser::slotContextMenuFiles(const QPoint &point)
{
    QModelIndex index = ui->treeFiles->indexAt(point);
    if (index.isValid() && m_filesMenu) {
        m_filesMenu->exec(ui->treeFiles->mapToGlobal(point));
    }
}

void Browser::addFile(OBSFile *file)
{
    qDebug() << __PRETTY_FUNCTION__;

    currentPackage = ui->treePackages->getCurrentPackage();
    QString fileProject = file->getProject();
    QString filePackage = file->getPackage();

    if (currentProject == fileProject && currentPackage == filePackage) {
        ui->treeFiles->addFile(file);
    }
}

void Browser::uploadFile(const QString &path)
{
    qDebug() <<  __PRETTY_FUNCTION__ << "path:" << path;

    QString package = ui->treePackages->getCurrentPackage();

    if (!currentProject.isEmpty() && !package.isEmpty()) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return;
        }

        QByteArray data = file.readAll();
        qDebug() << __PRETTY_FUNCTION__ << "data.size()" << data.size();

        QFileInfo fi(file.fileName());
        QString fileName = fi.fileName();

        m_obs->uploadFile(currentProject, package, fileName, data);

        QString statusText = tr("Uploading %1 to %2/%3...").arg(fileName, currentProject, package);
        emit updateStatusBar(statusText, false);
    } else {
        qDebug() << __PRETTY_FUNCTION__ << "File" << path << "cannot be uploaded. Project/package is empty!";
    }
}

void Browser::slotUploadFile(OBSRevision *revision)
{
    qDebug() << __PRETTY_FUNCTION__;
    QString currentPackage = ui->treePackages->getCurrentPackage();

    // Refresh file list
    if (currentProject == revision->getProject() && currentPackage == revision->getPackage()) {
        getPackageFiles(currentPackage);
    }
    showTrayMessage(APP_NAME, tr("The file %1 has been uploaded").arg(revision->getFile()));

    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotUploadFileError(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__ << status->getCode();
    QString title = tr("Warning");
    QString text = status->getSummary() + "<br>" + status->getDetails();
    QMessageBox::warning(this, title, text);
    QString statusText = tr("Error uploading to %1/%2").arg(status->getProject(), status->getPackage());

    emit updateStatusBar(statusText, true);
}

void Browser::slotContextMenuResults(const QPoint &point)
{
    QModelIndex index = ui->treeBuildResults->indexAt(point);
    if (index.isValid() && m_resultsMenu) {
        m_resultsMenu->exec(ui->treeBuildResults->mapToGlobal(point));
    }
}

void Browser::slotCreateRequest(OBSRequest *obsRequest)
{
    qDebug() << __PRETTY_FUNCTION__;

    QString message = tr("Request created successfully. %1").arg(obsRequest->getId());
    emit showTrayMessage(APP_NAME, message);
    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotCreateRequestStatus(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__;

    const QString title = tr("Request failed!");
    const QString text = QString("<b>%1</b><br>%2<br>%3").arg(
                status->getCode(), status->getSummary(), status->getDetails());
    QMessageBox::critical(this, title, text);

    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotBranchPackage(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (status->getCode()=="ok") {
        QString newBranch = status->getProject();
        m_browserFilter->addProject(newBranch);
        setCurrentProject(newBranch);
        showTrayMessage(APP_NAME, tr("The package %1 has been branched").arg(status->getPackage()));
    } else {
        const QString title = tr("Warning");
        const QString text = status->getSummary() + "<br>" + status->getDetails();
        QMessageBox::warning(this, title, text);
    }

    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotFileFetched(const QString &fileName, const QByteArray &data)
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save as"), fileName);
    QFile file(path);

    file.open(QIODevice::WriteOnly);
    qint64 bytesWritten = file.write(data);
    file.close();

    if (bytesWritten!=-1) {
        showTrayMessage(APP_NAME, tr("File %1 downloaded successfuly").arg(fileName));
    }
}

void Browser::slotBuildLogFetched(const QString &buildLog)
{
    qDebug() << __PRETTY_FUNCTION__;
    BuildLogViewer *buildLogViewer = new BuildLogViewer(this);
    buildLogViewer->show();
    buildLogViewer->setText(buildLog);
    buildLogViewer->setAttribute(Qt::WA_DeleteOnClose, true);
    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotBuildLogNotFound()
{
    qDebug() << __PRETTY_FUNCTION__;
    QString title = tr("Information");
    QString text = tr("Build log not found");
    QMessageBox::information(this, title, text);
    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotProjectNotFound(OBSStatus *status)
{
    ui->treePackages->clearModel();
    const QString title = tr("Project not found");
    const QString text = QString("<b>%1</b><br>%2").arg(status->getSummary(), status->getCode());
    QMessageBox::information(this, title, text);
    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotPackageNotFound(OBSStatus *status)
{
    const QString title = tr("Package not found");
    const QString text = QString("<b>%1</b><br>%2").arg(status->getSummary(), status->getCode());
    QMessageBox::information(this, title, text);
    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotDeleteProject(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (status->getCode()=="ok") {
        m_browserFilter->removeProject(status->getProject());

        showTrayMessage(APP_NAME, tr("The project %1 has been deleted").arg(status->getProject()));
    } else {
        const QString title = tr("Warning");
        const QString text = QString("<b>%1</b><br>%2").arg(status->getSummary(), status->getDetails());
        QMessageBox::warning(this, title, text);
    }

    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotDeletePackage(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (status->getCode() == "ok") {
        if (status->getProject() == currentProject) {
            ui->treePackages->removePackage(status->getPackage());
        }
        showTrayMessage(APP_NAME, tr("The package %1 has been deleted").arg(status->getPackage()));
    } else {
        const QString title = tr("Warning");
        const QString text = QString("<b>%1</b><br>%2").arg(status->getSummary(), status->getDetails());
        QMessageBox::warning(this, title, text);
    }

    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotDeleteFile(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (status->getCode() == "ok") {
        QString currentPackage = ui->treePackages->getCurrentPackage();
        QString fileName = status->getDetails();

        if (status->getProject() == currentProject && status->getPackage() == currentPackage) {
            bool fileRemoved = ui->treeFiles->removeFile(fileName);

            if (fileRemoved) {
                // Disable the delete file action if there are no files after a deletion
                QItemSelectionModel *treeFilesSelectionModel = ui->treeFiles->selectionModel();
                QList<QModelIndex> selectedFiles = treeFilesSelectionModel->selectedIndexes();
                emit fileSelectionChanged();
            }
        }
        showTrayMessage(APP_NAME, tr("The file %1 has been deleted").arg(status->getDetails()));
    } else {
        const QString title = tr("Warning");
        const QString text = QString("<b>%1</b><br>%2").arg(status->getSummary(), status->getDetails());
        QMessageBox::warning(this, title, text);
    }

    emit updateStatusBar(tr("Done"), true);
}

void Browser::finishedAddingResults()
{
   qDebug() << __PRETTY_FUNCTION__;
   emit updateStatusBar(tr("Done"), true);
}
