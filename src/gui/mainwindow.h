/*
 * Copyright (C) 2010-2025 Javier Llorente <javier@opensuse.org>
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
#include <QSharedPointer>
 #include <QAtomicInt>
#include "obs.h"
#include "trayicon.h"
#include "configure.h"
#include "login.h"
#include "obsstatus.h"
#include "utils.h"
#include "credentials.h"
#include "locationbar.h"
#include "searchbar.h"
#include "buildlogviewer.h"
#include "bookmarks.h"
#include "browser.h"
#include "monitor.h"
#include "requestbox.h"

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
    QAtomicInt runningTasks;

    QAction *action_createRequest;
    QAction *action_Branch_package;
    QAction *action_linkPackage;
    QAction *action_copyPackage;
    QAction *actionNew_package;
    QAction *actionNew_project;

    QToolButton *bookmarkButton;
    Bookmarks *bookmarks;
    QAction *actionBookmarks;

    QAction *action_ReloadProjects;
    QAction *action_ReloadPackages;
    QAction *action_ReloadFiles;
    QAction *action_ReloadResults;

    QAction *action_UploadFile;
    QAction *action_DownloadFile;

    QAction *action_MonitorProject;
    QAction *action_MonitorPackage;
    QAction *action_getBuildLog;

    QAction *actionDelete_project;
    QAction *actionDelete_package;
    QAction *actionDelete_file;

    QAction *actionProperties_project;
    QAction *actionProperties_package;

    LocationBar *locationBar;
    QAction *locationBarAction;
    SearchBar *searchBar;
    QAction *searchBarAction;
    void createActions();
    QAction *apiInformationAction;
    QAction *action_About;
    QAction *action_Quit;
    QAction *action_Restore;

    Browser *browser;
    bool firstTimeFileListDisplayed;
    bool firstTimeBuildResultsDisplayed;
    void setupPackageShortcuts();
    void setupFileShortcuts();
    QString currentProject;
    QString currentPackage;
    Monitor *monitor;
    void setupTreeMonitor();
    RequestBox *requestBox;

    void monitorProject();
    void monitorPackage();

    void createStatusBar();

    QTimer *timer;
    void createTimer();
    void setTimerInterval(int interval);
    int interval;

    void writeSettings();
    void readSettings();
    void readWindowSettings();
    void readAuthSettings();

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
    void onApiNotFound(const QUrl &url);
    void startTimer(bool authenticated);
    void readTimerSettings();
    void showNetworkError(const QString &networkError);
    void handleSelfSignedCertificates(QNetworkReply*);
    void onApiChanged();
    void onReadingPasswordError(const QString &error);
    void onCredentialsRestored(const QString &username, const QString &password);
    void onAuthenticated(bool authenticated);
    void setupActions();
    void setupProjectShortcuts();
    void getPackages(QModelIndex index);
    void setNotify(bool notify);
    void toggleAddRow(int index);
    void enableRemoveRow();
    void on_action_Refresh_triggered();
    void login(const QString &username, const QString &password);
    void showAbout();
    void on_quitActionTriggered();
    void onTrayIconClicked(QSystemTrayIcon::ActivationReason);
    void toggleVisibility();
    void showConfigureDialog();
    void on_signInActionTriggered();
    void on_apiinformationActiontTriggered();
    void onAbout(QSharedPointer<OBSAbout> about);
    void updatePerson(QSharedPointer<OBSPerson> obsPerson);
    void on_iconBar_currentRowChanged(int index);
    void onUpdateStatusBar(const QString &message, bool progressBarHidden);
};

#endif // MAINWINDOW_H
