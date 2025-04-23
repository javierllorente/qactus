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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProgressDialog>
#include <QScopedPointer>
#include "obsxmlwriter.h"

const QString defaultApiUrl = "https://api.opensuse.org";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    obs(new OBS()),
    trayIcon(new TrayIcon(this)),
    m_notify(false),
    locationBar(new LocationBar(this)),
    searchBar(new SearchBar(this)),
    browser(new Browser(this, locationBar, searchBar, obs)),
    monitor(new Monitor(this, obs)),
    requestBox(new RequestBox(this, obs)),
    errorBox(nullptr),
    loginDialog(nullptr)
{
    ui->setupUi(this);

    createActions();
    setupTreeMonitor();
    connect(this, &MainWindow::updateStatusBar, this, &MainWindow::onUpdateStatusBar);
    connect(browser, &Browser::showTrayMessage, trayIcon, [=](const QString &title, const QString &message){
        trayIcon->showMessage(title, message);
    });
    connect(trayIcon, &TrayIcon::activated, this, &MainWindow::onTrayIconClicked);
    connect(this, &MainWindow::notifyChanged, trayIcon, &TrayIcon::toggleIcon);
    connect(browser, &Browser::projectSelectionChanged, this, &MainWindow::setupActions);
    connect(browser, &Browser::projectSelectionChanged, this, &MainWindow::setupProjectShortcuts);
    connect(browser, &Browser::packageSelectionChanged, this, &MainWindow::setupActions);
    connect(browser, &Browser::packageSelectionChanged, this, &MainWindow::setupPackageShortcuts);
    connect(browser, &Browser::fileSelectionChanged, this, &MainWindow::setupActions);
    connect(browser, &Browser::fileSelectionChanged, this, &MainWindow::setupFileShortcuts);
    connect(browser, &Browser::buildResultSelectionChanged, this, &MainWindow::setupActions);
    connect(browser, &Browser::finishedLoadingProjects, [this](){
        ui->action_Home->setEnabled(true);
        bookmarkButton->setEnabled(true);
        action_ReloadProjects->setEnabled(true);
    });

    createStatusBar();
    createTimer();

    connect(obs, &OBS::apiNotFound, this, &MainWindow::onApiNotFound);
    connect(obs, &OBS::authenticated, this, &MainWindow::onAuthenticated);
    connect(obs, &OBS::selfSignedCertificateError,
            this, &MainWindow::handleSelfSignedCertificates);
    connect(obs, &OBS::networkError, this, &MainWindow::showNetworkError);
    connect(obs, &OBS::finishedParsingAbout, this, &MainWindow::onAbout);

    ui->stackedWidget->addWidget(browser);
    ui->stackedWidget->addWidget(monitor);
    ui->stackedWidget->addWidget(requestBox);

    readSettings();
    readTimerSettings();
}

MainWindow::~MainWindow()
{
    writeSettings();
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::onReadingPasswordError(const QString &error)
{
    qDebug() << Q_FUNC_INFO << error;
    showLoginDialog();
}

void MainWindow::onCredentialsRestored(const QString &username, const QString &password)
{
    qDebug() << Q_FUNC_INFO;
    login(username, password);
    QProgressDialog progress(tr("Logging in..."), nullptr, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
}

void MainWindow::showNetworkError(const QString &networkError)
{
    qDebug() << Q_FUNC_INFO;
    progressBar->setHidden(true);

    // The QMessageBox is only displayed once if there are
    // repeated errors (queued requests, probably same error)
    if (!errorBox) {
        errorBox = new QMessageBox(this);
    }

    if (!errorBox->isVisible()) {
        errorBox->setWindowTitle(tr("Network Error"));
        errorBox->setText(networkError);
        errorBox->setIcon(QMessageBox::Critical);
        emit updateStatusBar(tr("Network error"), true);
        int ret = errorBox->exec();
        if (ret) {
            delete errorBox;
            errorBox = nullptr;
        }
    }
}

void MainWindow::handleSelfSignedCertificates(QNetworkReply *reply)
{
    QSslConfiguration sslConfig;
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
            "/data/" + QCoreApplication::applicationName();
    QDir::setCurrent(dataDir);

    QString apiUrldomain = obs->getApiUrl().section('.', -2);
    apiUrldomain.replace(".", "");
    apiUrldomain.replace("/", "");
    QString filename =  apiUrldomain + ".pem";

    if (QFile::exists(filename)) {
        qDebug() << "Reading self-signed certificate" << filename;
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QByteArray byteArray = file.readAll();
        file.close();
        QSslCertificate sslCertificate(byteArray);
        auto sslConfiguration = QSslConfiguration::defaultConfiguration();
        sslConfiguration.addCaCertificate(sslCertificate);
        reply->ignoreSslErrors();
        return;
    } else {
        sslConfig = reply->sslConfiguration();
        QSslCertificate sslCertificate = sslConfig.peerCertificate();
//        qDebug() << sslCertificate.toText();

        QStringList certInfo = sslCertificate.subjectInfo("CN");
        const QString title = QString(tr("Warning"));
        const QString text = QString(tr("Do you want to accept this self-signed certificate from %1?")
                                     .arg(certInfo.at(0)));
        QMessageBox::StandardButton result = QMessageBox::warning(this, title, text,
                                                                  QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes)  {
            qDebug() << "Saving self-signed certificate as" << filename;

            QFile file(filename);
            file.open(QIODevice::WriteOnly);
            file.write(sslCertificate.toPem());
            file.close();

            auto sslConfiguration = QSslConfiguration::defaultConfiguration();
            sslConfiguration.addCaCertificate(sslCertificate);
            obs->request(reply);
            reply->ignoreSslErrors();
        }
    }
}

void MainWindow::onApiChanged()
{
    qDebug() << Q_FUNC_INFO;
    showLoginDialog();
}

void MainWindow::onAuthenticated(bool authenticated)
{
    qDebug() << Q_FUNC_INFO << authenticated;
    ui->action_Refresh->setEnabled(authenticated);
    if (authenticated) {
        browser->getProjects();
        emit updateStatusBar(tr("Getting projects..."), false);
        toggleAddRow(ui->stackedWidget->currentIndex());
        obs->getPerson();
        emit updateStatusBar(tr("Getting bookmarks..."), false);
        on_action_Refresh_triggered();
        delete loginDialog;
        loginDialog = nullptr;
        emit updateStatusBar(tr("Done"), true);
    } else {
        emit updateStatusBar(tr("Authentication is required"), true);
        showLoginDialog();
    }

    apiInformationAction->setEnabled(authenticated);
}

void MainWindow::setupActions()
{
    qDebug() << __PRETTY_FUNCTION__;

    bool projectSelected = browser->hasProjectSelection();
    actionNew_project->setEnabled(projectSelected);
    actionDelete_project->setEnabled(projectSelected);
    actionProperties_project->setEnabled(projectSelected);
    actionNew_package->setEnabled(projectSelected);
    action_ReloadPackages->setEnabled(projectSelected);
    action_MonitorProject->setEnabled(projectSelected);

    bool packageSelected = browser->hasPackageSelection();
    action_createRequest->setEnabled(packageSelected);
    action_Branch_package->setEnabled(packageSelected);
    action_linkPackage->setEnabled(packageSelected);
    action_copyPackage->setEnabled(packageSelected);
    action_UploadFile->setEnabled(packageSelected);
    action_MonitorPackage->setEnabled(packageSelected);
    actionDelete_package->setEnabled(packageSelected);
    actionProperties_package->setEnabled(packageSelected);
    action_ReloadFiles->setEnabled(packageSelected);
    action_ReloadResults->setEnabled(packageSelected);

    bool fileSelected = browser->hasFileSelection();
    action_DownloadFile->setEnabled(fileSelected);
    actionDelete_file->setEnabled(fileSelected);

    bool buildResultSelected = browser->hasBuildResultSelection();
    action_getBuildLog->setEnabled(buildResultSelected);
}

void MainWindow::setupProjectShortcuts()
{
    qDebug() << __PRETTY_FUNCTION__;
    actionDelete_package->setShortcut(QKeySequence());
    actionDelete_file->setShortcut(QKeySequence());
    actionDelete_project->setShortcut(QKeySequence::Delete);
    actionNew_project->setShortcut(QKeySequence::New);
    actionNew_package->setShortcut(QKeySequence());
}

void MainWindow::setupPackageShortcuts()
{
    qDebug() << __PRETTY_FUNCTION__;
    actionDelete_project->setShortcut(QKeySequence());
    actionDelete_file->setShortcut(QKeySequence());
    actionDelete_package->setShortcut(QKeySequence::Delete);
    actionNew_project->setShortcut(QKeySequence());
    actionNew_package->setShortcut(QKeySequence::New);
}

void MainWindow::setupFileShortcuts()
{
    qDebug() << __PRETTY_FUNCTION__;
    actionDelete_project->setShortcut(QKeySequence());
    actionDelete_package->setShortcut(QKeySequence());
    actionDelete_file->setShortcut(QKeySequence::Delete);
    actionNew_project->setShortcut(QKeySequence());
    actionNew_package->setShortcut(QKeySequence());
}

void MainWindow::getPackages(QModelIndex index)
{
    QString project = index.data().toString();
    qDebug() << "MainWindow::getPackages()" << project;
    if (!project.isEmpty()) {
        emit updateStatusBar(tr("Getting packages..."), false);
        obs->getPackages(project);
    }
}

void MainWindow::toggleAddRow(int index)
{
    // Enable if current tab is "My packages"
    ui->action_Add->setEnabled(index == 0 && obs->isAuthenticated());
}

void MainWindow::enableRemoveRow()
{
    qDebug() << Q_FUNC_INFO;
    ui->action_Remove->setEnabled(monitor->hasPackageSelection());
}

void MainWindow::on_action_Refresh_triggered()
{
    qDebug() << Q_FUNC_INFO;
    monitor->refresh();

    emit updateStatusBar(tr("Getting incoming requests..."), false);
    obs->getIncomingRequests();
    emit updateStatusBar(tr("Getting outgoing requests..."), false);
    obs->getOutgoingRequests();
}

void MainWindow::setupTreeMonitor()
{
    connect(ui->action_Add, &QAction::triggered, monitor, &Monitor::addRow);
    connect(monitor, &Monitor::currentTabChanged, this, &MainWindow::toggleAddRow);
    connect(monitor, &Monitor::itemSelectionChanged, this, &MainWindow::enableRemoveRow);
    connect(ui->action_Remove, &QAction::triggered, monitor, &Monitor::removeRow);
    connect(ui->action_Mark_all_as_read, &QAction::triggered, monitor, &Monitor::markAllRead);
    connect(monitor, &Monitor::notifyChanged, this, &MainWindow::setNotify);
    connect(monitor, &Monitor::updateStatusBar, this, &MainWindow::onUpdateStatusBar);
}

void MainWindow::setNotify(bool notify)
{
    qDebug() << "MainWindow::setNotify()" << notify;

    if (notify) {
        ui->action_Mark_all_as_read->setEnabled(true);
    }

    if (notify != m_notify) {
        m_notify = notify;
        emit notifyChanged(notify);
    }
}

void MainWindow::onUpdateStatusBar(const QString &message, bool progressBarHidden)
{
    qDebug() << Q_FUNC_INFO << "Running tasks:" << runningTasks.loadAcquire();
    qDebug() << Q_FUNC_INFO << message << progressBarHidden;

    if (progressBarHidden) {
        // Note: 1 means there are no tasks runnings. A very cute API :-)
        if (runningTasks.fetchAndSubRelaxed(1) == 1) {
            ui->statusbar->showMessage(message);
            progressBar->setHidden(progressBarHidden);
        }
    } else {
        runningTasks.fetchAndAddRelaxed(1);
        ui->statusbar->showMessage(message);
        progressBar->setHidden(progressBarHidden);
    }

}

void MainWindow::login(const QString &username, const QString &password)
{
    qDebug() << Q_FUNC_INFO;
    obs->setCredentials(username, password);
    obs->login();
    emit updateStatusBar(tr("Logging in..."), false);
}

void MainWindow::on_quitActionTriggered()
{
    qApp->quit();
}

void MainWindow::showAbout()
{
    QString title = tr("About ").arg(QCoreApplication::applicationName());
    QString aboutHtml;
    aboutHtml += QString("<big>%1</big>").arg(QCoreApplication::applicationName()) + "<br>";
    aboutHtml += "<b>" + tr("Version %1").arg(QCoreApplication::applicationVersion()) + "</b><br><br>";
    aboutHtml += tr("A Qt-based Open Build Service client")+ "<br>";
    aboutHtml += "<div align=\"left\">&copy; 2010-2025 Javier Llorente</div>";
    aboutHtml += "<br><a href='https://github.com/javierllorente/qactus'>https://github.com/javierllorente/qactus</a> </p>";
    aboutHtml += "<p><b>" + tr("License") + "</b><br>";
    aboutHtml += "<nobr>" + tr("This program is under the Apache License 2.0") + "</nobr></p></div>";
    aboutHtml += "<small>" + tr("Qt version: %1").arg(qVersion()) + "</small>";

    QMessageBox::about(this, title, aboutHtml);
}

void MainWindow::createActions()
{
    qDebug() << __PRETTY_FUNCTION__;

    // New actions
    actionNew_package = new QAction(tr("New p&ackage"), this);
    actionNew_package->setIcon(QIcon::fromTheme("package"));
    connect(actionNew_package, &QAction::triggered, browser, &Browser::newPackage);

    actionNew_project = new QAction(tr("New pr&oject"), this);
    actionNew_project->setIcon(QIcon::fromTheme("project-development"));
    connect(actionNew_project, &QAction::triggered, browser, &Browser::newProject);

    connect(ui->action_Home, &QAction::triggered, browser, &Browser::goHome);

    // WatchList actions
    bookmarkButton = new QToolButton(this);
    bookmarkButton->setPopupMode(QToolButton::InstantPopup);
    bookmarkButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    bookmarkButton->setText(tr("&Bookmarks"));
    bookmarkButton->setIcon(QIcon::fromTheme("bookmarks"));

    bookmarks = new Bookmarks(bookmarkButton);
    bookmarkButton->setMenu(bookmarks);
    bookmarkButton->setEnabled(false);
    connect(obs, &OBS::finishedParsingPerson, bookmarks, &Bookmarks::loadBookmarks);
    connect(bookmarks, &Bookmarks::clicked, browser, &Browser::goTo);
    connect(browser, &Browser::toggleBookmarkActions, bookmarks, &Bookmarks::toggleActions);
    connect(bookmarks, &Bookmarks::aboutToShow, [=]() {
        bookmarks->toggleActions(locationBar->text());
    });
    connect(bookmarks, &Bookmarks::bookmarkAdded, this, &MainWindow::updatePerson);
    connect(bookmarks, &Bookmarks::bookmarkDeleted, this, &MainWindow::updatePerson);
    connect(bookmarks, &Bookmarks::addBookmarkClicked, bookmarks, [=](){
        bookmarks->addBookmark(locationBar->text());
    });
    connect(bookmarks, &Bookmarks::deleteBookmarkClicked, bookmarks, [=](){
        bookmarks->deleteBookmark(locationBar->text());
    });
    connect(bookmarks, &Bookmarks::updateStatusBar, this, &MainWindow::onUpdateStatusBar);
    actionBookmarks = ui->toolBar->addWidget(bookmarkButton);

    // Location bar
    locationBarAction = ui->toolBar->addWidget(locationBar);

    // Search bar
    searchBarAction = ui->toolBar->addWidget(searchBar);

    // Menu button spacer
    QWidget *toolButtonSpacer = new QWidget();
    toolButtonSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QSizePolicy toolButtonSpacerPolicy = toolButtonSpacer->sizePolicy();
    toolButtonSpacer->setSizePolicy(toolButtonSpacerPolicy);
    ui->toolBar->addWidget(toolButtonSpacer);

    // Menu button
    QToolButton *toolButton = new QToolButton();
    toolButton->setFixedWidth(32);
    toolButton->setPopupMode(QToolButton::InstantPopup);
    toolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_M));
    toolButton->setIcon(QIcon::fromTheme("application-menu"));

    QAction *signInAction = new QAction(tr("Sign in"), this);
    signInAction->setIcon(QIcon::fromTheme("unlock"));
    connect(signInAction, &QAction::triggered, this, &MainWindow::showLoginDialog);

    QAction *configureAction = new QAction(tr("Configure"), this);
    configureAction->setIcon(QIcon::fromTheme("configure"));
    connect(configureAction, &QAction::triggered, this, &MainWindow::showConfigureDialog);

    apiInformationAction = new QAction(tr("API information"), this);
    apiInformationAction->setIcon(QIcon::fromTheme("help-about"));
    connect(apiInformationAction, &QAction::triggered, this, [&]() {
        obs->about();
    });

    QAction *aboutAction = new QAction(tr("About"), this);
    aboutAction->setIcon(QIcon::fromTheme("help-about"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

    QAction *quitAction = new QAction(tr("Quit"), this);
    quitAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    toolButton->addAction(signInAction);
    toolButton->addAction(configureAction);
    toolButton->addAction(apiInformationAction);
    toolButton->addAction(aboutAction);
    toolButton->addAction(quitAction);

    ui->toolBar->addWidget(toolButton);

    // Project actions
    QMenu *projectsMenu = new QMenu(this);

    // Reload actions
    action_ReloadProjects = new QAction(tr("&Reload project list"), this);
    action_ReloadProjects->setIcon(QIcon::fromTheme("view-refresh"));
    action_ReloadProjects->setEnabled(false);
    connect(action_ReloadProjects, &QAction::triggered, browser, &Browser::getProjects);

    action_ReloadPackages = new QAction(tr("&Reload package list"), this);
    action_ReloadPackages->setIcon(QIcon::fromTheme("view-refresh"));
    connect(action_ReloadPackages, &QAction::triggered, browser, &Browser::reloadPackages);

    action_ReloadFiles = new QAction(tr("&Reload file list"), this);
    action_ReloadFiles->setIcon(QIcon::fromTheme("view-refresh"));
    connect(action_ReloadFiles, &QAction::triggered, browser, &Browser::reloadFiles);

    action_ReloadResults = new QAction(tr("&Reload result list"), this);
    action_ReloadResults->setIcon(QIcon::fromTheme("view-refresh"));
    connect(action_ReloadResults, &QAction::triggered, browser, &Browser::reloadResults);

    // Upload action
    action_UploadFile = new QAction(tr("&Upload file"), this);
    action_UploadFile->setIcon(QIcon::fromTheme("cloud-upload"));
    connect(action_UploadFile, &QAction::triggered, browser, &Browser::uploadSelectedFile);

    // Download action
    action_DownloadFile = new QAction(tr("&Download file"), this);
    action_DownloadFile->setIcon(QIcon::fromTheme("download"));
    connect(action_DownloadFile, &QAction::triggered, browser, &Browser::downloadFile);

    // Monitor actions
    action_MonitorProject = new QAction(tr("&Monitor project"), this);
    action_MonitorProject->setIcon(QIcon::fromTheme("mail-thread-watch"));
    connect(action_MonitorProject, &QAction::triggered, this, &MainWindow::monitorProject);

    action_MonitorPackage = new QAction(tr("&Monitor package"), this);
    action_MonitorPackage->setIcon(QIcon::fromTheme("mail-thread-watch"));
    connect(action_MonitorPackage, &QAction::triggered, this, &MainWindow::monitorPackage);


    // Get build log action
    action_getBuildLog = new QAction(tr("&Get build log"), this);
    action_getBuildLog->setIcon(QIcon::fromTheme("text-x-log"));
    connect(action_getBuildLog, &QAction::triggered, browser, &Browser::getBuildLog);

    // Create request action
    action_createRequest = new QAction(tr("&Submit package"), this);
    action_createRequest->setIcon(QIcon::fromTheme("cloud-upload"));
    connect(action_createRequest, &QAction::triggered, browser, &Browser::createRequest);

    // Branch package action
    action_Branch_package = new QAction(tr("&Branch package"), this);
    action_Branch_package->setIcon(QIcon::fromTheme("branch"));
    connect(action_Branch_package, &QAction::triggered, browser, &Browser::branchSelectedPackage);

    // Link package action
    action_linkPackage = new QAction(tr("&Link package"), this);
    action_linkPackage->setIcon(QIcon::fromTheme("edit-link"));
    connect(action_linkPackage, &QAction::triggered, browser, &Browser::linkPackage);

    // Copy package action
    action_copyPackage = new QAction(tr("&Copy package"), this);
    action_copyPackage->setIcon(QIcon::fromTheme("edit-copy"));
    connect(action_copyPackage, &QAction::triggered, browser, &Browser::copyPackage);

    // Delete actions
    actionDelete_project = new QAction(tr("Delete pro&ject"), this);
    actionDelete_project->setIcon(QIcon::fromTheme("trash-empty"));
    connect(actionDelete_project, &QAction::triggered, browser, &Browser::deleteProject);

    actionDelete_package = new QAction(tr("Delete pac&kage"), this);
    actionDelete_package->setIcon(QIcon::fromTheme("trash-empty"));
    connect(actionDelete_package, &QAction::triggered, browser, &Browser::deletePackage);

    actionDelete_file = new QAction(tr("Delete &file"), this);
    actionDelete_file->setIcon(QIcon::fromTheme("trash-empty"));
    connect(actionDelete_file, &QAction::triggered, browser, &Browser::deleteFile);

    // Properties Action
    actionProperties_project = new QAction(tr("&Properties"), this);
    actionProperties_project->setIcon(QIcon::fromTheme("document-properties"));
    connect(actionProperties_project, &QAction::triggered, browser, &Browser::editProject);

    actionProperties_package = new QAction(tr("&Properties"), this);
    actionProperties_package->setIcon(QIcon::fromTheme("document-properties"));
    connect(actionProperties_package, &QAction::triggered, browser, &Browser::editPackage);

    // Tray icon actions
    action_Restore = new QAction(tr("&Minimise"), trayIcon);
    connect(action_Restore, &QAction::triggered, this, &MainWindow::toggleVisibility);
    trayIcon->trayIconMenu->addAction(action_Restore);

    action_Quit = new QAction(tr("&Quit"), trayIcon);
    action_Quit->setIcon(QIcon::fromTheme("application-exit"));
    connect(action_Quit, &QAction::triggered, qApp, &QApplication::quit);
    trayIcon->trayIconMenu->addAction(action_Quit);

    projectsMenu->addAction(actionNew_project);
    projectsMenu->addAction(action_ReloadProjects);
    projectsMenu->addAction(action_MonitorProject);
    projectsMenu->addAction(actionDelete_project);
    projectsMenu->addAction(actionProperties_project);

    QMenu *treePackagesMenu = new QMenu(this);
    treePackagesMenu->addAction(actionNew_package);
    treePackagesMenu->addAction(action_Branch_package);
    treePackagesMenu->addAction(action_createRequest);
    treePackagesMenu->addAction(action_linkPackage);
    treePackagesMenu->addAction(action_copyPackage);
    treePackagesMenu->addAction(action_ReloadPackages);
    treePackagesMenu->addAction(action_MonitorPackage);
    treePackagesMenu->addAction(actionDelete_package);
    treePackagesMenu->addAction(actionProperties_package);

    QMenu *treeFilesMenu = new QMenu(this);
    treeFilesMenu->addAction(action_UploadFile);
    treeFilesMenu->addAction(action_DownloadFile);
    treeFilesMenu->addAction(action_ReloadFiles);
    treeFilesMenu->addAction(actionDelete_file);

    QMenu *treeResultsMenu = new QMenu(this);
    treeResultsMenu->addAction(action_getBuildLog);
    treeResultsMenu->addAction(action_ReloadResults);

    browser->addProjectActions(projectsMenu->actions());
    browser->createPackagesContextMenu(treePackagesMenu);
    browser->createFilesContextMenu(treeFilesMenu);
    browser->setResultsMenu(treeResultsMenu);

    QAction *locationAction = new QAction(tr("Quick Project"), this);
    locationAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(locationAction, &QAction::triggered, this, [&](){
        locationBar->setFocus();
    });
    addAction(locationAction);

    QAction *quickSearchAction = new QAction(tr("Quick search"), this);
    quickSearchAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    connect(quickSearchAction, &QAction::triggered, this, [&](){
        searchBar->setFocus();
    });
    addAction(quickSearchAction);

    QAction *actionFilterPackages = new QAction(tr("Filter packages"), this);
    actionFilterPackages->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Q));
    connect(actionFilterPackages, &QAction::triggered, this, [&](){
        browser->setPackageFilterFocus();
    });
    addAction(actionFilterPackages);
}

void MainWindow::monitorProject()
{
    qDebug() << Q_FUNC_INFO;

    if (!monitor->tabWidgetContains(browser->getCurrentProject())) {
        monitor->addTab(browser->getCurrentProject());
    }

    obs->getProjectResults(browser->getCurrentProject());
}

void MainWindow::monitorPackage()
{
    qDebug() << Q_FUNC_INFO;

    if (!monitor->packagesTabContains(browser->getCurrentProject(),
                                      browser->getLocationPackage())) {
        monitor->addPackage(browser->getLocationPackage(), browser->getBuilds());
        monitor->refresh();
    }
}

void MainWindow::createStatusBar()
{
    connect(browser, &Browser::updateStatusBar, this, &MainWindow::onUpdateStatusBar);
    connect(requestBox, &RequestBox::updateStatusBar, this, &MainWindow::onUpdateStatusBar);

    ui->statusbar->showMessage(tr("Offline"));
    progressBar = new QProgressBar(ui->statusbar);
    progressBar->setHidden(true);
    progressBar->setTextVisible(false);
    progressBar->setMaximum(0);
    progressBar->setMaximum(0);
    progressBar->setMaximumSize(100, 16);
    ui->statusbar->addPermanentWidget(progressBar, 0);
}

void MainWindow::createTimer()
{
    timer = new QTimer(this);
    interval = 0;
    connect(obs, &OBS::authenticated, this, &MainWindow::startTimer);
    connect(timer, &QTimer::timeout, this, &MainWindow::on_action_Refresh_triggered);
}

void MainWindow::setTimerInterval(int interval)
{
    qDebug() << Q_FUNC_INFO << interval;
    if (interval >= 5) {
        this->interval = interval;
    } else {
        qDebug() << Q_FUNC_INFO << "Error starting timer: Wrong timer interval (smaller than 5)";
    }
}

void MainWindow::startTimer(bool authenticated)
{
    QSettings settings;
    settings.beginGroup("Timer");
    bool enableTimer = settings.value("Active").toBool();
    settings.endGroup();

    if (authenticated && enableTimer && !timer->isActive()) {
        qDebug() << "MainWindow::startTimer()" << interval;
        timer->start(interval*60000);
    } else {
        timer->stop();
    }
}

void MainWindow::toggleVisibility()
{
    if (this->isVisible()) {
        hide();
        action_Restore->setText(tr("Restore"));
    } else {
        showNormal();
        action_Restore->setText(tr("Minimise"));
    }
}

void MainWindow::onTrayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        toggleVisibility();
        setNotify(false);
    }

    qDebug() << Q_FUNC_INFO;
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("pos", pos());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("Row", ui->iconBar->currentRow());
    settings.endGroup();
}

void MainWindow::readSettings()
{
    qDebug() << "MainWindow::readSettings()";
    readWindowSettings();
    readProxySettings();
    readAuthSettings();
}

void MainWindow::readWindowSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    restoreGeometry(settings.value("geometry").toByteArray());
    int row = settings.value("Row").toInt();
    ui->iconBar->setCurrentRow(row);
    settings.endGroup();

    on_iconBar_currentRowChanged(row);
}

void MainWindow::readAuthSettings()
{
    qDebug() << Q_FUNC_INFO;
    QSettings settings;
    settings.beginGroup("Auth");
    QString apiUrl = settings.value("ApiUrl").toString();
    if (apiUrl.isEmpty()) {
        apiUrl = defaultApiUrl;
        settings.setValue("ApiUrl", defaultApiUrl);
    }
    obs->setApiUrl(apiUrl);
    if (settings.value("AutoLogin", true).toBool()) {
        Credentials *credentials = new Credentials(this);
        connect(credentials, &Credentials::errorReadingPassword,
                this, &MainWindow::onReadingPasswordError);
        connect(credentials, &Credentials::credentialsRestored,
                this, &MainWindow::onCredentialsRestored);
        credentials->readPassword(settings.value("Username").toString());
        delete credentials;
    }
    settings.endGroup();
}

void MainWindow::readProxySettings()
{
    qDebug() << Q_FUNC_INFO;

    QSettings settings;
    settings.beginGroup("Proxy");
    QNetworkProxy::ProxyType proxyType = static_cast<QNetworkProxy::ProxyType>(settings.value("Type").toInt());
    bool systemProxy = (proxyType == QNetworkProxy::DefaultProxy);
    bool manualProxy = (proxyType == QNetworkProxy::Socks5Proxy || proxyType == QNetworkProxy::HttpProxy);

    QNetworkProxyFactory::setUseSystemConfiguration(systemProxy);

    if (!systemProxy) {
        QNetworkProxy proxy;
        proxy.setType(proxyType);
        if (manualProxy) {
            proxy.setHostName(settings.value("Server").toString());
            proxy.setPort(settings.value("Port").toInt());
            proxy.setUser(settings.value("Username").toString());
            proxy.setPassword(settings.value("Password").toString());
        }
        QNetworkProxy::setApplicationProxy(proxy);
    }

    settings.endGroup();
}

void MainWindow::onApiNotFound(const QUrl &url)
{
    qDebug() << Q_FUNC_INFO;
    QString title = tr("Error");
    QString text = QString(tr("OBS API not found at<br>%1<br>"
                                    "Please check the URL and retry")).arg(url.toString());
    QMessageBox::critical(this, title, text);

    emit updateStatusBar(tr("OBS API not found at ") + url.toString(), true);
}

void MainWindow::readTimerSettings()
{
    QSettings settings;
    settings.beginGroup("Timer");
    if (settings.value("Active").toBool()) {
        qDebug() << "MainWindow::readSettingsTimer() Timer Active = true";
        qDebug() << "MainWindow::readSettingsTimer() Interval:" << settings.value("Value").toString() << "minutes";
        setTimerInterval(settings.value("Value").toInt());
        startTimer(obs->isAuthenticated());
    } else {
        qDebug() << "MainWindow::readSettingsTimer() Timer Active = false";
        if (timer->isActive()) {
            timer->stop();
            qDebug() << "MainWindow::readSettingsTimer() Timer has been stopped";
        }
    }
    settings.endGroup();
}

void MainWindow::showLoginDialog()
{
    if (!loginDialog) {
        loginDialog = new Login(this);
        connect(loginDialog, &Login::login, this, [this] (const QString &username, const QString &password) {
            if (obs->isAuthenticated()) {
                obs->logout();
            }
            login(username, password);
        });
    }
    loginDialog->show();
}

void MainWindow::showConfigureDialog()
{
    QScopedPointer<Configure> configure(new Configure(this, obs));
    connect(configure.data(), &Configure::apiChanged, this, &MainWindow::onApiChanged);
    connect(configure.data(), &Configure::proxyChanged, this, &MainWindow::readProxySettings);
    connect(configure.data(), &Configure::includeHomeProjectsChanged, this, [=](){
        browser->readSettings();
        browser->getProjects();
    });
    connect(configure.data(), &Configure::timerChanged, this, &MainWindow::readTimerSettings);
    configure->exec();
}

void MainWindow::on_signInActionTriggered()
{
    showLoginDialog();
}

void MainWindow::on_apiinformationActiontTriggered()
{
    obs->about();
}

void MainWindow::onAbout(QSharedPointer<OBSAbout> about)
{
    const QString title = about->getTitle();
    const QString text = QString("%1<br>Revision: %2<br>Last deployment: %3").arg(about->getDescription(),
                                                                                  about->getRevision(), about->getLastDeployment());
    QMessageBox::information(this, title, text);
}

void MainWindow::updatePerson(QSharedPointer<OBSPerson> obsPerson)
{
    OBSXmlWriter *xmlWriter = new OBSXmlWriter(this);
    QByteArray data = xmlWriter->createPerson(obsPerson);
    delete xmlWriter;
    obs->updatePerson(data);
}

void MainWindow::on_iconBar_currentRowChanged(int index)
{
    // Enable/disable the branch/delete button if there is a file/package/project selected
    setupActions();

    bool browserTabVisible = (index == 0);
    ui->action_Home->setVisible(browserTabVisible);
    actionBookmarks->setVisible(browserTabVisible);
    action_UploadFile->setVisible(browserTabVisible);
    action_DownloadFile->setVisible(browserTabVisible);

    locationBarAction->setVisible(browserTabVisible);
    searchBarAction->setVisible(browserTabVisible);

    bool monitorTabVisible = (index==1);
    ui->action_Add->setVisible(monitorTabVisible);
    ui->action_Remove->setVisible(monitorTabVisible);
    ui->action_Remove->setShortcut(monitorTabVisible ? QKeySequence::Delete : QKeySequence());
    ui->action_Mark_all_as_read->setVisible(monitorTabVisible);

    bool requestsTabVisible = (index==2);

    ui->action_Refresh->setVisible(monitorTabVisible || requestsTabVisible);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    toggleVisibility();
}

bool MainWindow::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::WindowActivate:
        setNotify(false);
        break;
    default:
        break;
    }
    return QMainWindow::event(event);
}
