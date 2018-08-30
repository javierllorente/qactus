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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QTreeWidgetItem>
#include <QtWidgets/QSystemTrayIcon>
#include <QMessageBox>
#include <QNetworkReply>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QProgressBar>
#include <QTimer>
#include <QToolButton>
#include <QFileDialog>
#include "obs.h"
#include "trayicon.h"
#include "configure.h"
#include "login.h"
#include "requeststateeditor.h"
#include "obsstatus.h"
#include "requesttreewidgetitem.h"
#include "utils.h"
#include "credentials.h"
#include "browserfilter.h"
#include "createdialog.h"
#include "buildlogviewer.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;

    OBS *obs;

    QProgressBar *progressBar;

    TrayIcon *trayIcon;
    bool m_notify;

    QToolButton *newButton;
    QMenu *newMenu;
    QAction *actionNew_package;
    QAction *actionNew_project;
    QAction *actionNew;

    QAction *action_ReloadProjects;
    QAction *action_ReloadPackages;
    QAction *action_ReloadFiles;
    QAction *action_ReloadResults;

    QAction *action_getBuildLog;

    QAction *actionDelete_project;
    QAction *actionDelete_package;
    QAction *actionDelete_file;
    bool deleteProjectConnected;
    bool deletePackageConnected;
    bool deleteFileConnected;

    BrowserFilter *browserFilter;
    QAction *actionFilter;
    QAction *actionFilterSpacer;
    void createActions();
    void setupIconBar();
    QAction *action_About;
    QAction *action_Quit;
    QAction *action_Restore;

    void setupBrowser();
    bool firstTimeFileListDisplayed;
    bool firstTimeBuildResultsDisplayed;
    QSortFilterProxyModel *proxyModelProjects;
    QStandardItemModel *sourceModelBuildResults;
    void setupProjectActions();
    void setupPackageActions();
    void setupFileActions();
    void setupModels();
    QStringList readProjectList() const;
    QString currentProject;
    QString currentPackage;
    void setupTreeMonitor();
    void createTreeRequests();

    void createStatusBar();

    QTimer *timer;
    void createTimer();
    void setTimerInterval(int interval);
    int interval;

    void writeSettings();
    void readSettings();
    void readMWSettings();
    void readAuthSettings();
    void readBrowserSettings();
    bool includeHomeProjects;

    QMessageBox *errorBox;

    void closeEvent(QCloseEvent*);
    bool event(QEvent *event);

    Login *loginDialog;
    void showLoginDialog();
    QItemSelectionModel *projectsSelectionModel;
    QItemSelectionModel *packagesSelectionModel;
    QItemSelectionModel *filesSelectionModel;

signals:
    void updateStatusBar(QString message, bool progressBarHidden);
    void notifyChanged(bool notify);

private slots:
    void readProxySettings();
    void slotApiNotFound(QUrl url);
    void startTimer(bool authenticated);
    void readTimerSettings();
    void showNetworkError(const QString &networkError);
    void handleSelfSignedCertificates(QNetworkReply*);
    void slotApiChanged();
    void slotErrorReadingPassword(const QString &error);
    void slotCredentialsRestored(const QString &username, const QString &password);
    void isAuthenticated(bool authenticated);
    void loadProjects();
    void slotContextMenuRequests(const QPoint &point);
    void slotContextMenuProjects(const QPoint &point);
    void slotContextMenuPackages(const QPoint &point);
    void slotContextMenuFiles(const QPoint &point);
    void slotContextMenuResults(const QPoint &point);
    void changeRequestState();
    void getPackages(QModelIndex index);
    void reloadPackages();
    void getPackageFiles(QModelIndex index);
    void reloadFiles();
    void getBuildResults();
    void reloadResults();
    void getBuildLog();
    void filterResults(const QString &item);
    void filterRadioButtonClicked(bool);
    void getRequestDescription(QTreeWidgetItem*, int);
    void setNotify(bool notify);
    void finishedAddingResults();
    void slotEnableRemoveRow();
    void on_action_Refresh_triggered();
    void on_action_Branch_package_triggered();
    void on_action_Home_triggered();
    void on_action_Upload_file_triggered();
    void on_action_Download_file_triggered();
    void newProject();
    void newPackage();
    void uploadFile(QString path);
    void deleteProject();
    void deletePackage();
    void deleteFile();
    void slotLogin(const QString &username, const QString &password);
    void on_action_About_triggered();
    void on_action_Quit_triggered();
    void trayIconClicked(QSystemTrayIcon::ActivationReason);
    void toggleVisibility();
    void on_action_Configure_Qactus_triggered();
    void on_action_Login_triggered();
    void on_actionAPI_information_triggered();
    void slotAbout(OBSAbout *obsAbout);
    void on_iconBar_currentRowChanged(int index);
    void on_tabWidgetPackages_currentChanged(int index);
    void addProjectList();
    void addFile(OBSFile*);
    void slotFileListAdded();
    void addResult(OBSResult*);
    void slotBranchPackage(OBSStatus *obsStatus);
    void slotUploadFile(OBSRevision *obsRevision);
    void slotUploadFileError(OBSStatus *obsStatus);
    void slotFileFetched(const QString &fileName, const QByteArray &data);
    void slotBuildLogFetched(const QString &buildLog);
    void slotBuildLogNotFound();
    void slotDeleteProject(OBSStatus *obsStatus);
    void slotDeletePackage(OBSStatus *obsStatus);
    void slotDeleteFile(OBSStatus *obsStatus);
    void insertRequest(OBSRequest*);
    void removeRequest(const QString& id);
    void slotUpdateStatusBar(const QString &message, bool progressBarHidden);
    void refreshProjectFilter();
    void projectSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void packageSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void fileSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void slotSrStatus(const QString &status);
};


#endif // MAINWINDOW_H
