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
#include "utils.h"
#include "credentials.h"

namespace Ui {
    class MainWindow;
}

class OBS;
class Login;
class TrayIcon;
class Configure;
class OBSFile;
class OBSResult;
class OBSPackage;
class OBSRequest;

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

    QToolBar *toolBar;
    QProgressBar *progressBar;

    TrayIcon *trayIcon;
    bool m_notify;
    void createActions();
    void setupIconBar();
    QAction *action_About;
    QAction *action_Quit;
    QAction *action_Restore;

    void setupBrowser();
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
    void showContextMenu(const QPoint&);
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
    void on_action_Remove_triggered();
    void on_action_Refresh_triggered();
    void on_action_Mark_all_as_read_triggered();
    void markRead(QTreeWidgetItem*, int);
    void loginSlot(const QString &username, const QString &password);
    void on_action_About_triggered();
    void on_action_Quit_triggered();
    void trayIconClicked(QSystemTrayIcon::ActivationReason);
    void toggleVisibility();
    void on_action_Configure_Qactus_triggered();
    void on_action_Login_triggered();
    void on_iconBar_currentRowChanged(int index);
    void insertProjectList();
    void insertPackageList();
    void insertFile(OBSFile*);
    void insertResult(OBSResult*);
    void insertBuildStatus(OBSPackage*, const int&);
    void insertRequest(OBSRequest*);
    void removeRequest(const QString& id);
    void updateStatusBarSlot(const QString &message, bool progressBarHidden);
    void refreshProjectFilter();
    void projectSelectionChanged(const QItemSelection &, const QItemSelection &);
    void buildSelectionChanged(const QItemSelection &, const QItemSelection &);
    void srStatusSlot(const QString &status);
};


#endif // MAINWINDOW_H
