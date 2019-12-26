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
#include "obsstatus.h"
#include "utils.h"
#include "credentials.h"
#include "browserfilter.h"
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

    QToolButton *newButton;
    QMenu *newMenu;
    QAction *action_createRequest;
    QAction *action_linkPackage;
    QAction *action_copyPackage;
    QAction *actionNew_package;
    QAction *actionNew_project;
    QAction *actionNew;

    QToolButton *bookmarkButton;
    Bookmarks *bookmarks;
    QAction *actionBookmarks;

    QAction *separatorHome;

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

    QAction *actionProperties_project;
    QAction *actionProperties_package;

    BrowserFilter *browserFilter;
    QAction *actionFilter;
    QAction *actionFilterSpacer;
    void createActions();
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

    void createStatusBar();

    QTimer *timer;
    void createTimer();
    void setTimerInterval(int interval);
    int interval;

    void writeSettings();
    void readSettings();
    void readMWSettings();
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
    void slotApiNotFound(QUrl url);
    void startTimer(bool authenticated);
    void readTimerSettings();
    void showNetworkError(const QString &networkError);
    void handleSelfSignedCertificates(QNetworkReply*);
    void slotApiChanged();
    void slotErrorReadingPassword(const QString &error);
    void slotCredentialsRestored(const QString &username, const QString &password);
    void isAuthenticated(bool authenticated);
    void setupActions();
    void setupProjectShortcuts();
    void getPackages(QModelIndex index);
    void setNotify(bool notify);
    void slotEnableRemoveRow();
    void on_action_Refresh_triggered();
    void slotLogin(const QString &username, const QString &password);
    void on_action_About_triggered();
    void on_action_Quit_triggered();
    void trayIconClicked(QSystemTrayIcon::ActivationReason);
    void toggleVisibility();
    void on_action_Configure_Qactus_triggered();
    void on_action_Login_triggered();
    void on_actionAPI_information_triggered();
    void slotAbout(OBSAbout *obsAbout);
    void slotUpdatePerson(OBSPerson *obsPerson);
    void on_iconBar_currentRowChanged(int index);
    void on_tabWidgetPackages_currentChanged(int index);
    void slotUpdateStatusBar(const QString &message, bool progressBarHidden);
};

#endif // MAINWINDOW_H
