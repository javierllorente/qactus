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

#ifndef BROWSER_H
#define BROWSER_H

#include <QWidget>
#include <QItemSelectionModel>
#include <QMenu>
#include "obs.h"

namespace Ui {
class Browser;
}

class Browser : public QWidget
{
    Q_OBJECT

public:
    explicit Browser(QWidget *parent = nullptr, OBS *obs = nullptr);
    ~Browser();
    void readSettings();
    void addProjectList(const QStringList &projectList);
    void createProjectsContextMenu(QMenu *projectsMenu);
    void createPackagesContextMenu(QMenu *packagesMenu);
    void createFilesContextMenu(QMenu *filesMenu);
    void createResultsContextMenu(QMenu *resultsMenu);
    bool hasProjectSelection();
    bool hasPackageSelection();
    bool hasFileSelection();

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
    void setCurrentProject(const QString &project);
    void downloadFile();
    void uploadSelectedFile();
    void createRequest();
    void copyPackage();
    void deleteProject();
    void deletePackage();
    void deleteFile();

private:
    Ui::Browser *ui;
    OBS *m_obs;
    QItemSelectionModel *packagesSelectionModel;
    QItemSelectionModel *filesSelectionModel;
    bool firstTimeFileListDisplayed;
    bool firstTimeBuildResultsDisplayed;
    QString currentProject;
    QString currentPackage;
    QMenu *m_projectsMenu;
    QMenu *m_packagesMenu;
    QMenu *m_filesMenu;
    QMenu *m_resultsMenu;
    void setupModels();
    void getPackages(const QString &project);

private slots:
    void slotContextMenuProjects(const QPoint &point);
    void slotProjectSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void slotContextMenuPackages(const QPoint &point);
    void slotPackageSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
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
    void getPackages(const QModelIndex &project);
    void updateStatusBar(QString message, bool progressBarHidden);
    void showTrayMessage(const QString &title, const QString &message);
    void toggleBookmarkActions(const QString &project);
};

#endif // BROWSER_H
