/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2010-2015 Javier Llorente <javier@opensuse.org>
 *  Copyright (C) 2010-2011 Sivan Greenberg <sivan@omniqueue.com>
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

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QAuthenticator>
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QUrl>
#include <QMainWindow>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QHttpRequestHeader>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QAction>
#include <QTimer>
#include <QSslError>
#include <QCoreApplication>
#include "trayicon.h"

namespace Ui {
    class MainWindow;
}

class Login;
class TrayIcon;
class OBSxmlReader;
class Configure;

class OBSaccess;
class OBSpackage;
class OBSrequest;

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

    OBSaccess *obsaccess;
    OBSpackage *obsPackage;
    QList<OBSrequest*> obsRequests;

    QToolBar *toolBar;
    void createToolbar();
    QAction *action_Add;
    QAction *action_Remove;
    QAction *action_Refresh;
    QAction *action_Timer;
    QAction *action_aboutQt;

    TrayIcon *trayIcon;
    void createActions();
    QAction *action_About;
    QAction *action_Quit;
    QAction *action_Restore;
//    QAction *action_Configure;

    void createTable();
    void createTableRequests();

    void createStatusBar();
    void writeSettings();
    void readSettings();
    void readSettingsTimer();

    QString packageErrors;

    void insertData(OBSpackage*, const int&);
    void insertRequests(QList<OBSrequest*>);
    QString breakLine(QString&, const int&);
    void checkStatus(const QString&, const int&);
    void paintStatus(const QString&);

    QTableWidgetItem *newItem;
    QTableWidgetItem *reqDataItem;
    QTableWidgetItem *reqIDItem;
    QTableWidgetItem *reqSourceProjectItem;
    QTableWidgetItem *reqTargetProjectItem;
    QTableWidgetItem *requesterItem;
    QTableWidgetItem *reqTypeItem;
    QTableWidgetItem *reqStateItem;

    QList<QString> statusList;

    Login *loginDialog;
    Configure *configureDialog;

private slots:
    void enableButtons(bool);
    void getDescription(QTableWidgetItem*);
    void launchRowEditor();
    void addRow();
    void removeRow();
    void refreshView();
    void lineEdit_Password_returnPressed();
    void pushButton_Login_clicked();
    void on_actionAbout_triggered(bool);
    void on_actionQuit_triggered(bool);
//    void on_cellChanged();
    void trayIconClicked(QSystemTrayIcon::ActivationReason);
    void toggleVisibility();
    void about();
    void on_actionConfigure_Qactus_triggered();
    void on_actionLogin_triggered();
    void on_tabWidget_currentChanged(const int&);
};


#endif // MAINWINDOW_H
