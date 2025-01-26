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
#include "obsxmlwriter.h"

const QString defaultApiUrl = "https://api.opensuse.org";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    obs(new OBS()),
    trayIcon(new TrayIcon(this)),
    m_notify(false),
    locationBar(new LocationBar(this)),
    browser(new Browser(this, locationBar, obs)),
    monitor(new Monitor(this, obs)),
    requestBox(new RequestBox(this, obs)),
    errorBox(nullptr),
    loginDialog(nullptr)
{
    ui->setupUi(this);

    createActions();
    setupTreeMonitor();
    connect(this, &MainWindow::updateStatusBar, this, &MainWindow::slotUpdateStatusBar);
    connect(browser, &Browser::showTrayMessage, trayIcon, [=](const QString &title, const QString &message){
        trayIcon->showMessage(title, message);
    });
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

    connect(ui->actionChange_request_state, &QAction::triggered, requestBox, &RequestBox::changeRequestState);
    connect(requestBox, &RequestBox::descriptionFetched, this, [=](){
        ui->actionChange_request_state->setEnabled(true);
    });

    createStatusBar();
    createTimer();

    connect(obs, SIGNAL(apiNotFound(QUrl)), this, SLOT(slotApiNotFound(QUrl)));
    connect(obs, SIGNAL(isAuthenticated(bool)), this, SLOT(isAuthenticated(bool)));
    connect(obs, SIGNAL(selfSignedCertificate(QNetworkReply*)),
            this, SLOT(handleSelfSignedCertificates(QNetworkReply*)));
    connect(obs, SIGNAL(networkError(QString)), this, SLOT(showNetworkError(QString)));
    connect(obs, &OBS::finishedParsingAbout, this, &MainWindow::slotAbout);

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

void MainWindow::slotErrorReadingPassword(const QString &error)
{
    qDebug() << "MainWindow::slotErrorReadingPassword()" << error;
    showLoginDialog();
}

void MainWindow::slotCredentialsRestored(const QString &username, const QString &password)
{
    qDebug() << "MainWindow::slotCredentialsRestored()";
    slotLogin(username, password);
    QProgressDialog progress(tr("Logging in..."), nullptr, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    emit updateStatusBar(tr("Logging in..."), false);
}

void MainWindow::showNetworkError(const QString &networkError)
{
    qDebug() << "MainWindow::showNetworkError()";
    progressBar->setHidden(true);

    // The QMessageBox is only displayed once if there are
    // repeated errors (queued requests, probably same error)
    if(!errorBox) {
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

void MainWindow::slotApiChanged()
{
    qDebug() << "MainWindow::slotApiChanged()";
    showLoginDialog();
}

void MainWindow::isAuthenticated(bool authenticated)
{
    qDebug() << "MainWindow::isAuthenticated()" << authenticated;
    ui->action_Refresh->setEnabled(authenticated);
    if (authenticated) {
        browser->getProjects();
        slotToggleAddRow(ui->stackedWidget->currentIndex());
        obs->getPerson();
        on_action_Refresh_triggered();
        delete loginDialog;
        loginDialog = nullptr;
    } else {
        emit updateStatusBar(tr("Authentication is required"), true);
        showLoginDialog();
    }

    ui->actionAPI_information->setEnabled(authenticated);
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

void MainWindow::slotToggleAddRow(int index)
{
    // Enable if current tab is "My packages"
    ui->action_Add->setEnabled(index == 0 && obs->isAuthenticated());
}

void MainWindow::slotEnableRemoveRow()
{
    qDebug() << __PRETTY_FUNCTION__;
    ui->action_Remove->setEnabled(monitor->hasPackageSelection());
}

void MainWindow::on_action_Refresh_triggered()
{
    qDebug() << "MainWindow::refreshView()";
    emit updateStatusBar(tr("Getting build statuses..."), false);
    monitor->refresh();

    emit updateStatusBar(tr("Getting requests..."), false);
    obs->getIncomingRequests();
    obs->getOutgoingRequests();
}

void MainWindow::setupTreeMonitor()
{
    connect(ui->action_Add, &QAction::triggered, monitor, &Monitor::addRow);
    connect(monitor, &Monitor::currentTabChanged, this, &MainWindow::slotToggleAddRow);
    connect(monitor, &Monitor::itemSelectionChanged, this, &MainWindow::slotEnableRemoveRow);
    connect(ui->action_Remove, &QAction::triggered, monitor, &Monitor::removeRow);
    connect(ui->action_Mark_all_as_read, &QAction::triggered, monitor, &Monitor::markAllRead);
    connect(monitor, &Monitor::notifyChanged, this, &MainWindow::setNotify);
    connect(monitor, &Monitor::updateStatusBar, this, &MainWindow::slotUpdateStatusBar);
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

void MainWindow::slotUpdateStatusBar(const QString &message, bool progressBarHidden)
{
    qDebug() << "MainWindow::slotUpdateStatusBar()";
    ui->statusbar->showMessage(message);
    progressBar->setHidden(progressBarHidden);
}

void MainWindow::slotLogin(const QString &username, const QString &password)
{
    qDebug() << "MainWindow::slotLogin()";
    obs->setCredentials(username, password);
    obs->login();
    emit updateStatusBar(tr("Logging in..."), false);
}

void MainWindow::on_action_Quit_triggered()
{
    qApp->quit();
}

void MainWindow::on_action_About_triggered()
{
    QString title = tr("About ").arg(QCoreApplication::applicationName());
    QString aboutHtml;
    aboutHtml += QString("<big>%1</big>").arg(QCoreApplication::applicationName()) + "<br>";
    aboutHtml += "<b>" + tr("Version %1").arg(QCoreApplication::applicationVersion()) + "</b><br><br>";
    aboutHtml += tr("A Qt-based Open Build Service client")+ "<br>";
    aboutHtml += "<div align=\"left\">&copy; 2010-2024 Javier Llorente</div>";
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
    connect(obs, &OBS::finishedParsingPerson, bookmarks, &Bookmarks::slotLoadBookmarks);
    connect(bookmarks, &Bookmarks::clicked, browser, &Browser::goTo);
    connect(browser, &Browser::toggleBookmarkActions, bookmarks, &Bookmarks::toggleActions);
    connect(bookmarks, &Bookmarks::bookmarkAdded, this, &MainWindow::slotUpdatePerson);
    connect(bookmarks, &Bookmarks::bookmarkDeleted, this, &MainWindow::slotUpdatePerson);
    connect(bookmarks, &Bookmarks::addBookmarkClicked, bookmarks, [=](){
        bookmarks->addBookmark(browser->getCurrentProject());
    });
    connect(bookmarks, &Bookmarks::deleteBookmarkClicked, bookmarks, [=](){
        bookmarks->deleteBookmark(browser->getCurrentProject());
    });
    actionBookmarks = ui->toolBar->addWidget(bookmarkButton);

    // Browser filter actions
    actionFilter = ui->toolBar->addWidget(locationBar);
    QWidget *filterSpacer = new QWidget(this);
    filterSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    filterSpacer->setFixedWidth(25);
    filterSpacer->setVisible(true);
    actionFilterSpacer = ui->toolBar->addWidget(filterSpacer);

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
    action_UploadFile = new QAction(tr("&Upload file"));
    action_UploadFile->setIcon(QIcon::fromTheme("cloud-upload"));
    connect(action_UploadFile, &QAction::triggered, browser, &Browser::uploadSelectedFile);

    // Download action
    action_DownloadFile = new QAction(tr("&Download file"));
    action_DownloadFile->setIcon(QIcon::fromTheme("download"));
    connect(action_DownloadFile, &QAction::triggered, browser, &Browser::downloadFile);

    // Monitor project action
    action_MonitorProject = new QAction(tr("&Monitor project"), this);
    action_MonitorProject->setIcon(QIcon::fromTheme("mail-thread-watch"));
    connect(action_MonitorProject, &QAction::triggered, this, &MainWindow::monitorProject);

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
    connect(action_Restore, SIGNAL(triggered()), this, SLOT(toggleVisibility()));
    trayIcon->trayIconMenu->addAction(action_Restore);

    action_Quit = new QAction(tr("&Quit"), trayIcon);
    action_Quit->setIcon(QIcon::fromTheme("application-exit"));
    connect(action_Quit, SIGNAL(triggered()), qApp, SLOT(quit()));
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

    QAction *actionQuickSearch = new QAction(tr("Quick Project"), this);
    actionQuickSearch->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(actionQuickSearch, &QAction::triggered, this, [&](){
        locationBar->setFocus();
    });
    addAction(actionQuickSearch);

    QAction *actionFilterPackages = new QAction(tr("Filter packages"), this);
    actionFilterPackages->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Q));
    connect(actionFilterPackages, &QAction::triggered, this, [&](){
        browser->setPackageFilterFocus();
    });
    addAction(actionFilterPackages);
}

void MainWindow::monitorProject()
{
    qDebug() << __PRETTY_FUNCTION__;

    if (!monitor->tabWidgetContains(browser->getCurrentProject())) {
        monitor->addTab(browser->getCurrentProject());
    }

    obs->getProjectResults(browser->getCurrentProject());
    emit updateStatusBar(tr("Adding project to monitor..."), false);
}

void MainWindow::createStatusBar()
{
    connect(browser, &Browser::updateStatusBar, this, &MainWindow::slotUpdateStatusBar);

    ui->statusbar->showMessage(tr("Offline"));
    progressBar = new QProgressBar(ui->statusbar);
    progressBar->setHidden(true);
    progressBar->setTextVisible(false);
    progressBar->setMaximum(0);
    progressBar->setMaximum(0);
    progressBar->setMaximumSize(50, ui->statusbar->height());
    ui->statusbar->addPermanentWidget(progressBar, 10);
}

void MainWindow::createTimer()
{
    timer = new QTimer(this);
    interval = 0;
    connect(obs, SIGNAL(isAuthenticated(bool)), this, SLOT(startTimer(bool)));
    connect(timer, SIGNAL(timeout()), this, SLOT(on_action_Refresh_triggered()));
}

void MainWindow::setTimerInterval(int interval)
{
    qDebug() << "MainWindow::setTimerInterval()" << interval;
    if (interval >= 5) {
        this->interval = interval;
    } else {
        qDebug() << "Error starting timer: Wrong timer interval (smaller than 5)";
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

void MainWindow::trayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason==QSystemTrayIcon::Trigger) {
        toggleVisibility();
        setNotify(false);
    }

    qDebug() << "MainWindow::trayIconClicked()";
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
    qDebug() << "MainWindow::readAuthSettings()";
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
        connect(credentials, SIGNAL(errorReadingPassword(QString)),
                this, SLOT(slotErrorReadingPassword(QString)));
        connect(credentials, SIGNAL(credentialsRestored(QString, QString)),
                this, SLOT(slotCredentialsRestored(QString, QString)));
        credentials->readPassword(settings.value("Username").toString());
        delete credentials;
    }
    settings.endGroup();
}

void MainWindow::readProxySettings()
{
    qDebug() << "MainWindow::readProxySettings()";

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

void MainWindow::slotApiNotFound(QUrl url)
{
    qDebug() << " MainWindow::slotApiNotFound()";
    const QString title = tr("Error");
    const QString text = QString(tr("OBS API not found at<br>%1<br>"
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
    qDebug() << "MainWindow::showLoginDialog()";
    if (!loginDialog) {
        loginDialog = new Login(this);
        connect(loginDialog, SIGNAL(login(QString,QString)), this, SLOT(slotLogin(QString,QString)));
    }
    loginDialog->show();
}

void MainWindow::on_action_Configure_Qactus_triggered()
{
    qDebug() << __PRETTY_FUNCTION__;
    Configure *configure = new Configure(this, obs);
    connect(configure, &Configure::apiChanged, this, &MainWindow::slotApiChanged);
    connect(configure, &Configure::proxyChanged, this, &MainWindow::readProxySettings);
    connect(configure, &Configure::includeHomeProjectsChanged, this, [=](){
        browser->readSettings();
        browser->getProjects();
    });
    connect(configure, &Configure::timerChanged, this, &MainWindow::readTimerSettings);
    configure->exec();
    delete configure;
}

void MainWindow::on_action_Login_triggered()
{
    showLoginDialog();
}

void MainWindow::on_actionAPI_information_triggered()
{
    obs->about();
}

void MainWindow::slotAbout(QSharedPointer<OBSAbout> about)
{
    const QString title = about->getTitle();
    const QString text = QString("%1<br>Revision: %2<br>Last deployment: %3").arg(about->getDescription(),
                                                                                  about->getRevision(), about->getLastDeployment());
    QMessageBox::information(this, title, text);
}

void MainWindow::slotUpdatePerson(QSharedPointer<OBSPerson> obsPerson)
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

    actionFilterSpacer->setVisible(browserTabVisible);
    actionFilter->setVisible(browserTabVisible);

    bool monitorTabVisible = (index==1);
    ui->action_Add->setVisible(monitorTabVisible);
    ui->action_Remove->setVisible(monitorTabVisible);
    ui->action_Remove->setShortcut(monitorTabVisible ? QKeySequence::Delete : QKeySequence());
    ui->action_Mark_all_as_read->setVisible(monitorTabVisible);

    bool requestsTabVisible = (index==2);
    ui->actionChange_request_state->setVisible(requestsTabVisible);

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
        qDebug() << "Window activated";
        setNotify(false);
        break;
    default:
        break;
    }
    return QMainWindow::event(event);
}
