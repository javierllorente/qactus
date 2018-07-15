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
#include "roweditor.h"
#include "requeststateeditor.h"
#include "obsstatus.h"
#include "autotooltipdelegate.h"
#include "requesttreewidgetitem.h"
#include "utils.h"
#include "credentials.h"
#include "browserfilter.h"
#include "createdialog.h"

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

    QToolButton *deleteButton;
    QMenu *deleteMenu;
    QAction *actionDelete_project;
    QAction *actionDelete_package;
    QAction *actionDelete_file;
    QAction *actionDelete;

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
    void loadProjects();
    QStringListModel *sourceModelProjects;
    QStringListModel *sourceModelBuilds;
    QSortFilterProxyModel *proxyModelProjects;
    QSortFilterProxyModel *proxyModelBuilds;
    QStandardItemModel *sourceModelFiles;
    QStandardItemModel *sourceModelBuildResults;
    void getBuildResults();
    void filterProjects(const QString &item);
    void filterBuilds(const QString &item);
    void createTreePackages();
    void createTreeRequests();

    void createStatusBar();

    QTimer *timer;
    void createTimer();
    void setTimerInterval(int interval);
    int interval;

    void writeSettings();
    void readSettings();
    void readMWSettings();
    void readMonitorSettings();
    void readAuthSettings();
    void readBrowserSettings();
    bool includeHomeProjects;

    QMessageBox *errorBox;

    void closeEvent(QCloseEvent*);
    bool event(QEvent *event);

    Login *loginDialog;
    void showLoginDialog();
    QItemSelectionModel *projectsSelectionModel;
    QItemSelectionModel *buildsSelectionModel;
    QItemSelectionModel *filesSelectionModel;
    bool hasBuildStatusChanged(const QString &oldStatus, const QString &newStatus);

signals:
    void updateStatusBar(QString message, bool progressBarHidden);
    void notifyChanged(bool notify);

private slots:
    void readProxySettings();
    void startTimer(bool authenticated);
    void readTimerSettings();
    void showNetworkError(const QString &networkError);
    void handleSelfSignedCertificates(QNetworkReply*);
    void apiChangedSlot();
    void errorReadingPasswordSlot(const QString &error);
    void credentialsRestoredSlot(const QString &username, const QString &password);
    void isAuthenticated(bool authenticated);
    void slotContextMenuRequests(const QPoint &point);
    void slotContextMenuProjects(const QPoint &point);
    void slotContextMenuPackages(const QPoint &point);
    void slotContextMenuFiles(const QPoint &point);
    void changeRequestState();
    void getPackages(QModelIndex index);
    void getPackageFiles(QModelIndex index);
    void filterResults(const QString &item);
    void filterRadioButtonClicked(bool);
    void getRequestDescription(QTreeWidgetItem*, int);
    void setNotify(bool notify);
    void on_action_Add_triggered();
    void finishedResultListSlot();
    void editRow(QTreeWidgetItem*, int);
    void slotEnableRemoveRow();
    void on_action_Remove_triggered();
    void on_action_Refresh_triggered();
    void on_action_Branch_package_triggered();
    void on_action_Upload_file_triggered();
    void newProject();
    void newPackage();
    void deleteProject();
    void deletePackage();
    void deleteFile();
    void on_action_Mark_all_as_read_triggered();
    void markRead(QTreeWidgetItem*, int);
    void loginSlot(const QString &username, const QString &password);
    void on_action_About_triggered();
    void on_action_Quit_triggered();
    void trayIconClicked(QSystemTrayIcon::ActivationReason);
    void toggleVisibility();
    void on_action_Configure_Qactus_triggered();
    void on_action_Login_triggered();
    void on_actionAPI_information_triggered();
    void slotAbout(OBSAbout *obsAbout);
    void on_iconBar_currentRowChanged(int index);
    void insertProjectList();
    void insertPackageList();
    void insertFile(OBSFile*);
    void slotFileListAdded();
    void insertResult(OBSResult*);
    void insertBuildStatus(OBSStatus *obsStatus, int row);
    void slotBranchPackage(OBSStatus *obsStatus);
    void slotUploadFile(OBSRevision *obsRevision);
    void slotUploadFileError(OBSStatus *obsStatus);
    void slotDeleteProject(OBSStatus *obsStatus);
    void slotDeletePackage(OBSStatus *obsStatus);
    void slotDeleteFile(OBSStatus *obsStatus);
    void insertRequest(OBSRequest*);
    void removeRequest(const QString& id);
    void updateStatusBarSlot(const QString &message, bool progressBarHidden);
    void refreshProjectFilter();
    void projectSelectionChanged(const QItemSelection &, const QItemSelection &);
    void buildSelectionChanged(const QItemSelection &, const QItemSelection &);
    void fileSelectionChanged(const QItemSelection &, const QItemSelection &);
    void srStatusSlot(const QString &status);
};


#endif // MAINWINDOW_H
