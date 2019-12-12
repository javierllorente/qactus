/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
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

#include "browser.h"
#include "ui_browser.h"
#include <QFileDialog>
#include <QSettings>
#include "metaconfigeditor.h"
#include "createrequestdialog.h"
#include "copypackagedialog.h"
#include "buildlogviewer.h"

Browser::Browser(QWidget *parent, OBS *obs) :
    QWidget(parent),
    ui(new Ui::Browser),
    m_obs(obs),
    firstTimeBuildResultsDisplayed(true),
    m_projectsMenu(nullptr),
    m_packagesMenu(nullptr),
    m_filesMenu(nullptr),
    m_resultsMenu(nullptr),
    m_projectsToolbar(new QToolBar(this)),
    m_packagesToolbar(new QToolBar(this)),
    m_filesToolbar(new QToolBar(this)),
    m_resultsToolbar(new QToolBar(this))
{
    ui->setupUi(this);

    ui->hSplitterBrowser->setStretchFactor(1, 1);
    ui->hSplitterBrowser->setStretchFactor(0, 0);

    m_projectsToolbar->setIconSize(QSize(15, 15));
    m_packagesToolbar->setIconSize(QSize(15, 15));
    m_filesToolbar->setIconSize(QSize(15, 15));
    m_resultsToolbar->setIconSize(QSize(15, 15));

    ui->verticalLayout_6->addWidget(m_projectsToolbar);
    ui->verticalLayout_5->insertWidget(1, m_packagesToolbar);
    ui->verticalLayout_4->addWidget(m_filesToolbar);
    ui->verticalLayout_2->addWidget(m_resultsToolbar);

    connect(m_obs, &OBS::finishedParsingProjectList, this, &Browser::addProjectList);

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
        if (status->getCode()=="ok") {
            ui->treeProjects->addProject(status->getProject());
            ui->treeProjects->setCurrentProject(status->getProject());
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

    connect(ui->treeProjects, &ProjectTreeWidget::customContextMenuRequested, this, &Browser::slotContextMenuProjects);
    connect(ui->treePackages, &PackageTreeWidget::customContextMenuRequested, this, &Browser::slotContextMenuPackages);
    connect(ui->treeFiles, &FileTreeWidget::customContextMenuRequested, this, &Browser::slotContextMenuFiles);
    connect(ui->treeBuildResults, &BuildResultTreeWidget::customContextMenuRequested, this,&Browser::slotContextMenuResults);

    connect(ui->treeFiles, &FileTreeWidget::droppedFile, this, &Browser::uploadFile);

    connect(m_obs, &OBS::finishedParsingPackageList, ui->treePackages, &PackageTreeWidget::addPackageList);
    connect(ui->treePackages, &PackageTreeWidget::updateStatusBar, this, &Browser::updateStatusBar);

    connect(m_obs, &OBS::finishedParsingResult, this, &Browser::addResult);
    connect(m_obs, &OBS::finishedParsingResultList, this, &Browser::finishedAddingResults);

    // Model selection's signals-slots
    QItemSelectionModel *projectsSelectionModel = ui->treeProjects->selectionModel();
    connect(projectsSelectionModel, &QItemSelectionModel::selectionChanged, this, &Browser::slotProjectSelectionChanged);
    connect(projectsSelectionModel, &QItemSelectionModel::selectionChanged, this, &Browser::projectSelectionChanged);

    packagesSelectionModel = ui->treePackages->selectionModel();
    connect(packagesSelectionModel, &QItemSelectionModel::selectionChanged, this, &Browser::slotPackageSelectionChanged);
    connect(packagesSelectionModel, &QItemSelectionModel::selectionChanged, this, &Browser::packageSelectionChanged);

    filesSelectionModel = ui->treeFiles->selectionModel();
    connect(filesSelectionModel, &QItemSelectionModel::selectionChanged, this, &Browser::fileSelectionChanged);

    connect(ui->lineEditFilter, &QLineEdit::textChanged, ui->treePackages, &PackageTreeWidget::filterPackages);

    readSettings();
}

Browser::~Browser()
{
    delete ui;
}

void Browser::readSettings()
{
    QSettings settings;
    settings.beginGroup("Browser");
    bool includeHomeProjects = settings.value("IncludeHomeProjects").toBool();
    m_obs->setIncludeHomeProjects(includeHomeProjects);
    settings.endGroup();
}

void Browser::addProjectList(const QStringList &projectList)
{ 
    qDebug() << __PRETTY_FUNCTION__;
    ui->treeProjects->addProjectList(projectList);

    emit toggleBookmarkActions("");
    emit finishedLoadingProjects();
    emit updateStatusBar(tr("Done"), true);
}

void Browser::createProjectsContextMenu(QMenu *projectsMenu)
{
    m_projectsMenu = projectsMenu;
    m_projectsToolbar->addActions(m_projectsMenu->actions());
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
    QItemSelectionModel *treeProjectsSelectionModel = ui->treeProjects->selectionModel();
    if (treeProjectsSelectionModel) {
        return treeProjectsSelectionModel->hasSelection();
    } else {
        return false;
    }
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

void Browser::newProject()
{
    qDebug() << __PRETTY_FUNCTION__;

    QString project = ui->treeProjects->getCurrentProject();

    MetaConfigEditor *metaConfigEditor = new MetaConfigEditor(this, m_obs, project, "", MCEMode::CreateProject);
    metaConfigEditor->exec();
    delete metaConfigEditor;
}

void Browser::newPackage()
{
    qDebug() << __PRETTY_FUNCTION__;

    QString project = ui->treeProjects->getCurrentProject();

    MetaConfigEditor *metaConfigEditor = new MetaConfigEditor(this, m_obs, project, "", MCEMode::CreatePackage);
    metaConfigEditor->exec();
    delete metaConfigEditor;
}

void Browser::editProject()
{
    qDebug() << __PRETTY_FUNCTION__;

    QString project = ui->treeProjects->getCurrentProject();

    MetaConfigEditor *metaConfigEditor = new MetaConfigEditor(this, m_obs, project, "", MCEMode::EditProject);
    metaConfigEditor->exec();
    delete metaConfigEditor;
}

void Browser::editPackage()
{
    qDebug() << __PRETTY_FUNCTION__;

    QString project = ui->treeProjects->getCurrentProject();
    QString package = ui->treePackages->getCurrentPackage();

    MetaConfigEditor *metaConfigEditor = new MetaConfigEditor(this, m_obs, project, package, MCEMode::EditPackage);
    metaConfigEditor->exec();
    delete metaConfigEditor;
}

void Browser::reloadPackages()
{
    qDebug() << __PRETTY_FUNCTION__;
    getPackages(ui->treeProjects->getCurrentProject());

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

    currentProject = ui->treeProjects->getCurrentProject();
    currentPackage = ui->treePackages->getCurrentPackage();
    QString resultProject = result->getProject();
    QString resultPackage = result->getStatus()->getPackage();

    if (currentProject==resultProject && currentPackage==resultPackage) {
        ui->treeBuildResults->addResult(result);
    }

//  FIXME: The last slot connected is in charge of deleting result
//    delete result;
//    result = nullptr;
}

void Browser::reloadResults()
{
    qDebug() << __PRETTY_FUNCTION__;
    getBuildResults(ui->treeProjects->getCurrentProject(), ui->treePackages->getCurrentPackage());
    emit packageSelectionChanged();
}

void Browser::getBuildLog()
{
    qDebug() << __PRETTY_FUNCTION__;
    QString currentProject = ui->treeProjects->getCurrentProject();
    QString currentBuildRepository = ui->treeBuildResults->getCurrentRepository();
    QString currentBuildArch = ui->treeBuildResults->getCurrentArch();
    QString currentPackage = ui->treePackages->getCurrentPackage();

    m_obs->getBuildLog(currentProject, currentBuildRepository, currentBuildArch, currentPackage);
    emit updateStatusBar(tr("Getting build log..."), false);
}

void Browser::branchSelectedPackage()
{
    QString project = ui->treeProjects->getCurrentProject();
    QString package = ui->treePackages->getCurrentPackage();

    const QString title = tr("Branch confirmation");
    const QString text = tr("<b>Source</b><br> %1/%2<br><b>Destination</b><br> home:%3:branches:%4")
                           .arg(project, package, m_obs->getUsername(), project);

    QMessageBox::StandardButton result = QMessageBox::question(this, title, text,
                                                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        qDebug() << "Branching package..." << project << "/" << package;
        m_obs->branchPackage(project, package);
        const QString statusText = tr("Branching %1/%2...").arg(project, package);
        emit updateStatusBar(statusText, false);
    }
}

void Browser::getProjects()
{
    qDebug() << __PRETTY_FUNCTION__;
    setupModels();
    ui->treeFiles->setAcceptDrops(false);

    emit updateStatusBar(tr("Getting projects..."), false);
    m_obs->getProjects();
}

void Browser::goHome()
{
    QString userHomeProject = QString("home:%1").arg(m_obs->getUsername());
    ui->treeProjects->setCurrentProject(userHomeProject);
}

QString Browser::getCurrentProject() const
{
    return ui->treeProjects->getCurrentProject();
}

void Browser::setCurrentProject(const QString &project)
{
    ui->treeProjects->setCurrentProject(project);
}

void Browser::downloadFile()
{
    QString currentProject = ui->treeProjects->getCurrentProject();
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
    QString currentProject = ui->treeProjects->getCurrentProject();
    QString currentPackage = ui->treePackages->getCurrentPackage();

    OBSRequest *request = new OBSRequest();
    request->setActionType("submit");
    request->setSourceProject(currentProject);
    request->setSourcePackage(currentPackage);

    if (ui->treeFiles->hasLink()) {
        m_obs->getLink(currentProject, currentPackage);
    }

    CreateRequestDialog *createRequestDialog = new CreateRequestDialog(request, m_obs, this);
    createRequestDialog->addProjectList(ui->treeProjects->getProjectList());
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

void Browser::copyPackage()
{
    qDebug() << __PRETTY_FUNCTION__;
    CopyPackageDialog *copyPackageDialog = new CopyPackageDialog(this, m_obs,
                                                                 ui->treeProjects->getCurrentProject(),
                                                                 ui->treePackages->getCurrentPackage());
    copyPackageDialog->addProjectList(ui->treeProjects->getProjectList());
    connect(copyPackageDialog, &CopyPackageDialog::showTrayMessage, this, &Browser::showTrayMessage);
    connect(copyPackageDialog, &CopyPackageDialog::updateStatusBar, this, &Browser::updateStatusBar);
    copyPackageDialog->exec();
    delete copyPackageDialog;
}

void Browser::deleteProject()
{
    qDebug() << __PRETTY_FUNCTION__;

    QString project = ui->treeProjects->getCurrentProject();

    const QString title = tr("Delete confirmation");
    const QString text = tr("Do you really want to delete project<br> %1?")
                           .arg(project);

    QMessageBox::StandardButton result = QMessageBox::question(this, title, text,
                                                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        qDebug() << __PRETTY_FUNCTION__ << "Deleting project" << project << "...";
        m_obs->deleteProject(project);
        const QString statusText = tr("Deleting %1...").arg(project);
        emit updateStatusBar(statusText, false);
    }
}

void Browser::deletePackage()
{
    qDebug() << __PRETTY_FUNCTION__;

    QString project = ui->treeProjects->getCurrentProject();
    QString package = ui->treePackages->getCurrentPackage();

    const QString title = tr("Delete confirmation");
    const QString text = tr("Do you really want to delete package<br> %1/%2?")
                           .arg(project, package);

    QMessageBox::StandardButton result = QMessageBox::question(this, title, text,
                                                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        qDebug() << "Deleting package" << package << "...";
        m_obs->deletePackage(project, package);
        const QString statusText = tr("Deleting %1...").arg(package);
        emit updateStatusBar(statusText, false);
    }
}

void Browser::deleteFile()
{
    qDebug() << __PRETTY_FUNCTION__;

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
        m_obs->deleteFile(project, package, fileName);
        const QString statusText = tr("Deleting %1...").arg(fileName);
        emit updateStatusBar(statusText, false);
    }
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

void Browser::slotContextMenuProjects(const QPoint &point)
{
    QModelIndex index = ui->treeProjects->indexAt(point);
    if (index.isValid() && m_projectsMenu) {
        m_projectsMenu->exec(ui->treeProjects->mapToGlobal(point));
    }
}

void Browser::slotProjectSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_UNUSED(deselected)

    QString selectedProjectStr;

    // Clean up files and build results on project click
    ui->treeFiles->clearModel();
    ui->treeBuildResults->clearModel();

    if (!selected.isEmpty()) {
        QModelIndex selectedProject = selected.indexes().at(0);
        selectedProjectStr = selectedProject.data().toString();
        getPackages(selectedProject.data().toString());
        ui->treePackages->filterPackages("");
        emit projectSelectionChanged();
        ui->treeFiles->setAcceptDrops(false);
    }

    emit toggleBookmarkActions(selectedProjectStr);
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
    }
}

void Browser::slotPackageSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_UNUSED(deselected)

    if (!selected.isEmpty()) {
        QModelIndex selectedPackage = selected.indexes().at(0);
        currentProject = ui->treeProjects->getCurrentProject();
        currentPackage = ui->treePackages->getCurrentPackage();

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

void Browser::getPackageFiles(const QString &package)
{
    qDebug() << __PRETTY_FUNCTION__;
    emit updateStatusBar(tr("Getting package files.."), false);

    ui->treeFiles->clearModel();

    QString currentProject = ui->treeProjects->getCurrentProject();
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

    currentProject = ui->treeProjects->getCurrentProject();
    currentPackage = ui->treePackages->getCurrentPackage();
    QString fileProject = file->getProject();
    QString filePackage = file->getPackage();

    if (currentProject==fileProject && currentPackage==filePackage) {
        ui->treeFiles->addFile(file);
    }
    delete file;
    file = nullptr;
}

void Browser::uploadFile(const QString &path)
{
    qDebug() <<  __PRETTY_FUNCTION__ << "path:" << path;

    QString project = ui->treeProjects->getCurrentProject();
    QString package = ui->treePackages->getCurrentPackage();

    if (!project.isEmpty() && !package.isEmpty()) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return;
        }

        QByteArray data = file.readAll();
        qDebug() << __PRETTY_FUNCTION__ << "data.size()" << data.size();

        QFileInfo fi(file.fileName());
        QString fileName = fi.fileName();

        m_obs->uploadFile(project, package, fileName, data);

        QString statusText = tr("Uploading %1 to %2/%3...").arg(fileName, project, package);
        emit updateStatusBar(statusText, false);
    } else {
        qDebug() << __PRETTY_FUNCTION__ << "File" << path << "cannot be uploaded. Project/package is empty!";
    }
}

void Browser::slotUploadFile(OBSRevision *revision)
{
    qDebug() << __PRETTY_FUNCTION__;
    QString currentProject = ui->treeProjects->getCurrentProject();
    QString currentPackage = ui->treePackages->getCurrentPackage();

    // Refresh file list
    if (currentProject == revision->getProject() && currentPackage == revision->getPackage()) {
        getPackageFiles(currentPackage);
    }
    showTrayMessage(APP_NAME, tr("The file %1 has been uploaded").arg(revision->getFile()));

    delete revision;
    revision = nullptr;

    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotUploadFileError(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__ << status->getCode();
    QString title = tr("Warning");
    QString text = status->getSummary() + "<br>" + status->getDetails();
    QMessageBox::warning(this, title, text);
    QString statusText = tr("Error uploading to %1/%2").arg(status->getProject(), status->getPackage());

    delete status;
    status = nullptr;

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
    delete obsRequest;

    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotCreateRequestStatus(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__;

    const QString title = tr("Request failed!");
    const QString text = QString("<b>%1</b><br>%2<br>%3").arg(
                status->getCode(), status->getSummary(), status->getDetails());
    QMessageBox::critical(this, title, text);

    delete status;

    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotBranchPackage(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (status->getCode()=="ok") {
        QString newBranch = QString("home:%1:branches:%2").arg(m_obs->getUsername(), status->getProject());
        ui->treeProjects->addProject(newBranch);
        ui->treeProjects->setCurrentProject(newBranch);
        showTrayMessage(APP_NAME, tr("The package %1 has been branched").arg(status->getPackage()));

    } else {
        const QString title = tr("Warning");
        const QString text = status->getSummary() + "<br>" + status->getDetails();
        QMessageBox::warning(this, title, text);
    }

    delete status;
    status = nullptr;

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
    delete status;
    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotPackageNotFound(OBSStatus *status)
{
    const QString title = tr("Package not found");
    const QString text = QString("<b>%1</b><br>%2").arg(status->getSummary(), status->getCode());
    QMessageBox::information(this, title, text);
    delete status;
    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotDeleteProject(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (status->getCode()=="ok") {
        ui->treeProjects->removeProject(status->getProject());
        showTrayMessage(APP_NAME, tr("The project %1 has been deleted").arg(status->getProject()));
    } else {
        const QString title = tr("Warning");
        const QString text = QString("<b>%1</b><br>%2").arg(status->getSummary(), status->getDetails());
        QMessageBox::warning(this, title, text);
    }

    delete status;
    status = nullptr;

    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotDeletePackage(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (status->getCode()=="ok") {
        QString currentProject = ui->treeProjects->getCurrentProject();

        if (status->getProject()==currentProject) {
            ui->treePackages->removePackage(status->getPackage());
        }
        showTrayMessage(APP_NAME, tr("The package %1 has been deleted").arg(status->getPackage()));
    } else {
        const QString title = tr("Warning");
        const QString text = QString("<b>%1</b><br>%2").arg(status->getSummary(), status->getDetails());
        QMessageBox::warning(this, title, text);
    }

    delete status;
    status = nullptr;

    emit updateStatusBar(tr("Done"), true);
}

void Browser::slotDeleteFile(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (status->getCode()=="ok") {
        QString currentProject = ui->treeProjects->getCurrentProject();
        QString currentPackage = ui->treePackages->getCurrentPackage();
        QString fileName = status->getDetails();

        if (status->getProject()==currentProject && status->getPackage()==currentPackage) {

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

    delete status;
    status = nullptr;

    emit updateStatusBar(tr("Done"), true);
}

void Browser::finishedAddingResults()
{
   qDebug() << __PRETTY_FUNCTION__;
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
