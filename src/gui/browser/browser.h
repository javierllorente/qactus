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
#ifndef BROWSER_H
#define BROWSER_H

#include <QWidget>
#include <QItemSelectionModel>
#include <QMenu>
#include <QToolBar>
#include "locationbar.h"
#include "obs.h"

namespace Ui {
class Browser;
}

class Browser : public QWidget
{
    Q_OBJECT

public:
    explicit Browser(QWidget *parent = nullptr, LocationBar *locationBar = nullptr, OBS *obs = nullptr);
    ~Browser();
    void readSettings();
    void addProjectList(const QStringList &projectList);
    void createPackagesContextMenu(QMenu *packagesMenu);
    void createFilesContextMenu(QMenu *filesMenu);
    void createResultsContextMenu(QMenu *resultsMenu);
    QString getLocationProject() const;
    QString getLocationPackage() const;
    bool hasProjectSelection();
    bool hasPackageSelection();
    bool hasFileSelection();
    bool hasBuildResultSelection();
    void setPackageFilterFocus();
    QString packageFilterText() const;
    void clearPackageFilter();
    void clearOverview();

public slots:
    void newProject();
    void newPackage();
    void editProject();
    void editPackage();
    void reloadPackages();
    void reloadFiles();
    void addResult(OBSResult *result);
    void reloadResults();
    void getBuildLog();
    void branchSelectedPackage();
    void getProjects();
    void goHome();
    QString getCurrentProject() const;
    void load(const QString &location);
    void goTo(const QString &location);
    void handleProjectTasks();
    void handlePackageTasks();
    void slotSelectedPackageNotFound(const QString &package);
    void downloadFile();
    void uploadSelectedFile();
    void createRequest();
    void linkPackage();
    void copyPackage();
    void deleteProject();
    void deletePackage();
    void deleteFile();

private:
    Ui::Browser *ui;
    LocationBar *m_locationBar;
    OBS *m_obs;
    QItemSelectionModel *packagesSelectionModel;
    QItemSelectionModel *filesSelectionModel;
    QString currentProject;
    QString currentPackage;
    QString selectPackage;
    QString overviewProject;
    QString overviewPackage;
    QMenu *m_packagesMenu;
    QMenu *m_filesMenu;
    QMenu *m_resultsMenu;
    QToolBar *m_packagesToolbar;
    QToolBar *m_filesToolbar;
    QToolBar *m_resultsToolbar;
    bool m_loaded;
    void writeSettings();
    void setupModels();
    void getPackages(const QString &project);
    void getRevisions(const QString &project, const QString &package);
    void getProjectRequests(const QString &project);
    void getPackageRequests(const QString &project, const QString &package);

private slots:
    void slotContextMenuPackages(const QPoint &point);
    void slotProjectSelectionChanged();
    void slotPackageSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void slotTabIndexChanged(int index);
    void slotSelectPackage();
    void slotMetaConfigFetched(OBSMetaConfig *metaConfig);
    void setLatestRevision(OBSRevision *revision);
    void getPackageFiles(const QString &package);
    void getBuildResults(const QString &project, const QString &package);
    void slotContextMenuFiles(const QPoint &point);
    void addFile(OBSFile *file);
    void uploadFile(const QString &path);
    void slotUploadFile(OBSRevision *revision);
    void slotUploadFileError(OBSStatus *status);
    void finishedAddingResults();
    void slotContextMenuResults(const QPoint &point);
    void slotCreateRequest(OBSRequest *request);
    void slotCreateRequestStatus(OBSStatus *status);
    void slotBranchPackage(OBSStatus *status);
    void slotFileFetched(const QString &fileName, const QByteArray &data);
    void slotBuildLogFetched(const QString &buildLog);
    void slotBuildLogNotFound();
    void slotProjectNotFound(OBSStatus *status);
    void slotPackageNotFound(OBSStatus *status);
    void slotDeleteProject(OBSStatus *status);
    void slotDeletePackage(OBSStatus *status);
    void slotDeleteFile(OBSStatus *status);

signals:
    void projectSelectionChanged();
    void packageSelectionChanged();
    void fileSelectionChanged();
    void buildResultSelectionChanged();
    void getPackages(const QModelIndex &project);
    void updateStatusBar(QString message, bool progressBarHidden);
    void showTrayMessage(const QString &title, const QString &message);
    void finishedLoadingProjects();
    void toggleBookmarkActions(const QString &project);
};

#endif // BROWSER_H
