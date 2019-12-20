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
    deleteProjectConnected(false),
    deletePackageConnected(false),
    deleteFileConnected(false),
    browser(new Browser(this, obs)),
    monitor(new Monitor(this, obs)),
    requestBox(new RequestBox(this, obs)),
    errorBox(nullptr),
    loginDialog(nullptr)
{
    ui->setupUi(this);

    createActions();
    setupTreeMonitor();
    connect(browser, &Browser::showTrayMessage, trayIcon, [=](const QString &title, const QString &message){
        trayIcon->showMessage(title, message);
    });

    connect(browser, &Browser::projectSelectionChanged, this, &MainWindow::setupActions);
    connect(browser, &Browser::projectSelectionChanged, this, &MainWindow::setupProjectShortcuts);
    connect(browser, &Browser::projectSelectionChanged, this, [&]() {
        if (!browserFilter->text().isEmpty()) {
            browserFilter->clear();
        }
    });
    connect(browser, &Browser::packageSelectionChanged, this, &MainWindow::setupActions);
    connect(browser, &Browser::packageSelectionChanged, this, &MainWindow::setupPackageShortcuts);
    connect(browser, &Browser::fileSelectionChanged, this, &MainWindow::setupActions);
    connect(browser, &Browser::fileSelectionChanged, this, &MainWindow::setupFileShortcuts);
    connect(browser, &Browser::buildResultSelectionChanged, this, &MainWindow::setupActions);
    connect(browser, &Browser::finishedLoadingProjects, [this](){
        newButton->setEnabled(true);
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
    connect(obs, SIGNAL(finishedParsingAbout(OBSAbout*)), this, SLOT(slotAbout(OBSAbout*)));

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
    QProgressDialog progress(tr("Logging in..."), 0, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    emit updateStatusBar(tr("Logging in..."), false);
}

void MainWindow::showNetworkError(const QString &networkError)
{
    qDebug() << "MainWindow::showNetworkError()";

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
            errorBox = NULL;
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
        QSslSocket::addDefaultCaCertificate(sslCertificate);
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

            QSslSocket::addDefaultCaCertificate(sslCertificate);
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

    bool packageSelected = browser->hasPackageSelection();
    ui->action_Branch_package->setEnabled(packageSelected);
    action_createRequest->setEnabled(packageSelected);
    action_copyPackage->setEnabled(packageSelected);
    ui->action_Upload_file->setEnabled(packageSelected);
    actionDelete_package->setEnabled(packageSelected);
    actionProperties_package->setEnabled(packageSelected);
    action_ReloadFiles->setEnabled(packageSelected);
    action_ReloadResults->setEnabled(packageSelected);

    bool fileSelected = browser->hasFileSelection();
    ui->action_Download_file->setEnabled(fileSelected);
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

    ui->action_Delete->setEnabled(browser->hasProjectSelection());
    disconnect(ui->action_Delete, &QAction::triggered, browser, &Browser::deletePackage);
    disconnect(ui->action_Delete, &QAction::triggered, browser, &Browser::deleteFile);

    deletePackageConnected = false;
    deleteFileConnected = false;
    if (!deleteProjectConnected) {
        connect(ui->action_Delete, &QAction::triggered, browser, &Browser::deleteProject);
        deleteProjectConnected = true;
    }

    actionNew_project->setShortcut(QKeySequence::New);
    actionNew_package->setShortcut(QKeySequence());
}

void MainWindow::setupPackageShortcuts()
{
    qDebug() << __PRETTY_FUNCTION__;
    actionDelete_project->setShortcut(QKeySequence());
    actionDelete_file->setShortcut(QKeySequence());
    actionDelete_package->setShortcut(QKeySequence::Delete);

    ui->action_Delete->setEnabled(true);
    disconnect(ui->action_Delete, &QAction::triggered, browser, &Browser::deleteProject);
    disconnect(ui->action_Delete, &QAction::triggered, browser, &Browser::deleteFile);

    deleteProjectConnected = false;
    deleteFileConnected = false;
    if (!deletePackageConnected) {
        connect(ui->action_Delete, &QAction::triggered, browser, &Browser::deletePackage);
        deletePackageConnected = true;
    }

    actionNew_project->setShortcut(QKeySequence());
    actionNew_package->setShortcut(QKeySequence::New);
}

void MainWindow::setupFileShortcuts()
{
    qDebug() << __PRETTY_FUNCTION__;
    actionDelete_project->setShortcut(QKeySequence());
    actionDelete_package->setShortcut(QKeySequence());
    actionDelete_file->setShortcut(QKeySequence::Delete);

    ui->action_Delete->setEnabled(true);
    disconnect(ui->action_Delete, &QAction::triggered, browser, &Browser::deleteProject);
    disconnect(ui->action_Delete, &QAction::triggered, browser, &Browser::deletePackage);

    deleteProjectConnected = false;
    deletePackageConnected = false;
    if (!deleteFileConnected) {
        connect(ui->action_Delete, &QAction::triggered, browser, &Browser::deleteFile);
        deleteFileConnected = true;
    }

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

void MainWindow::slotEnableRemoveRow()
{
    if (!monitor->hasSelection()) {
        ui->action_Remove->setEnabled(false);
    } else if (!ui->action_Remove->isEnabled()) {
        ui->action_Remove->setEnabled(true);
    }
}

void MainWindow::on_action_Refresh_triggered()
{
    qDebug() << "MainWindow::refreshView()";
    emit updateStatusBar(tr("Getting build statuses..."), false);
    monitor->getBuildStatus();

    emit updateStatusBar(tr("Getting requests..."), false);
    obs->getIncomingRequests();
    obs->getOutgoingRequests();
}

void MainWindow::setupTreeMonitor()
{
    connect(ui->action_Add, &QAction::triggered, monitor, &Monitor::addRow);
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
    aboutHtml += "<div align=\"left\">&copy; 2010-2019 Javier Llorente</div>";
    aboutHtml += "<br><a href='https://github.com/javierllorente/qactus'>https://github.com/javierllorente/qactus</a> </p>";
    aboutHtml += "<p><b>" + tr("License") + "</b><br>";
    aboutHtml += "<nobr>" + tr("This program is under the GPLv3") + "</nobr></p></div>";
    aboutHtml += "<small>" + tr("Qt version: %1").arg(qVersion()) + "</small>";

    QMessageBox::about(this, title, aboutHtml);
}

void MainWindow::createActions()
{
    // New button actions
    newButton = new QToolButton(this);
    newButton->setPopupMode(QToolButton::MenuButtonPopup);
    newButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    newButton->setText(tr("&New"));
    newButton->setIcon(QIcon::fromTheme("document-new"));

    newMenu = new QMenu(newButton);
    actionNew_package = new QAction(tr("New p&ackage"), this);
    actionNew_package->setIcon(QIcon::fromTheme("package"));
    connect(actionNew_package, &QAction::triggered, browser, &Browser::newPackage);

    actionNew_project = new QAction(tr("New pr&oject"), this);
    actionNew_project->setIcon(QIcon::fromTheme("project-development"));
    connect(actionNew_project, &QAction::triggered, browser, &Browser::newProject);

    newMenu->addAction(actionNew_package);
    newMenu->addAction(actionNew_project);
    newButton->setMenu(newMenu);
    newButton->setEnabled(false);

    actionNew = ui->toolBar->insertWidget(ui->action_Branch_package, newButton);
    connect(newButton, &QToolButton::clicked, browser, &Browser::newPackage);

    connect(ui->action_Branch_package, &QAction::triggered, browser, &Browser::branchSelectedPackage);
    connect(ui->action_Home, &QAction::triggered, browser, &Browser::goHome);
    connect(ui->action_Download_file, &QAction::triggered, browser, &Browser::downloadFile);
    connect(ui->action_Upload_file, &QAction::triggered, browser, &Browser::uploadSelectedFile);

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
    connect(bookmarks, &Bookmarks::clicked, browser, &Browser::setCurrentProject);
    connect(browser, &Browser::toggleBookmarkActions, bookmarks, &Bookmarks::toggleActions);
    connect(bookmarks, &Bookmarks::bookmarkAdded, this, &MainWindow::slotUpdatePerson);
    connect(bookmarks, &Bookmarks::bookmarkDeleted, this, &MainWindow::slotUpdatePerson);
    connect(bookmarks, &Bookmarks::addBookmarkClicked, bookmarks, [=](){
        bookmarks->addBookmark(browser->getCurrentProject());
    });
    connect(bookmarks, &Bookmarks::deleteBookmarkClicked, bookmarks, [=](){
        bookmarks->deleteBookmark(browser->getCurrentProject());
    });

    actionBookmarks = ui->toolBar->insertWidget(ui->action_Upload_file, bookmarkButton);
    separatorHome = ui->toolBar->insertSeparator(ui->action_Home);

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

    // Get build log action
    action_getBuildLog = new QAction(tr("&Get build log"), this);
    action_getBuildLog->setIcon(QIcon::fromTheme("text-x-log"));
    connect(action_getBuildLog, &QAction::triggered, browser, &Browser::getBuildLog);

    // Create request action
    action_createRequest = new QAction(tr("&Submit package"), this);
    action_createRequest->setIcon(QIcon::fromTheme("cloud-upload"));
    connect(action_createRequest, &QAction::triggered, browser, &Browser::createRequest);

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

    // Browser filter actions
    QWidget *filterSpacer = new QWidget(this);
    filterSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    filterSpacer->setVisible(true);
    actionFilterSpacer = ui->toolBar->addWidget(filterSpacer);

    browserFilter = new BrowserFilter(this);
    actionFilter = ui->toolBar->insertWidget(ui->action_Upload_file, browserFilter);
    connect(obs, &OBS::finishedParsingProjectList, browserFilter, &BrowserFilter::addProjectList);
    connect(browserFilter, &BrowserFilter::setCurrentProject, browser, &Browser::setCurrentProject);

    // Tray icon actions
    action_Restore = new QAction(tr("&Minimise"), trayIcon);
    connect(action_Restore, SIGNAL(triggered()), this, SLOT(toggleVisibility()));
    trayIcon->trayIconMenu->addAction(action_Restore);

    action_Quit = new QAction(tr("&Quit"), trayIcon);
    action_Quit->setIcon(QIcon::fromTheme("application-exit"));
    connect(action_Quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    trayIcon->trayIconMenu->addAction(action_Quit);


    QMenu *treeProjectsMenu = new QMenu(this);
    treeProjectsMenu->addAction(actionNew_project);
    treeProjectsMenu->addAction(action_ReloadProjects);
    treeProjectsMenu->addAction(actionDelete_project);
    treeProjectsMenu->addAction(actionProperties_project);

    QMenu *treePackagesMenu = new QMenu(this);
    treePackagesMenu->addAction(actionNew_package);
    treePackagesMenu->addAction(ui->action_Branch_package);
    treePackagesMenu->addAction(action_createRequest);
    treePackagesMenu->addAction(action_copyPackage);
    treePackagesMenu->addAction(action_ReloadPackages);
    treePackagesMenu->addAction(actionDelete_package);
    treePackagesMenu->addAction(actionProperties_package);

    QMenu *treeFilesMenu = new QMenu(this);
    treeFilesMenu->addAction(ui->action_Upload_file);
    treeFilesMenu->addAction(ui->action_Download_file);
    treeFilesMenu->addAction(action_ReloadFiles);
    treeFilesMenu->addAction(actionDelete_file);

    QMenu *treeResultsMenu = new QMenu(this);
    treeResultsMenu->addAction(action_getBuildLog);
    treeResultsMenu->addAction(action_ReloadResults);

    browser->createProjectsContextMenu(treeProjectsMenu);
    browser->createPackagesContextMenu(treePackagesMenu);
    browser->createFilesContextMenu(treeFilesMenu);
    browser->createResultsContextMenu(treeResultsMenu);
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
    readMWSettings();
    readProxySettings();
    readAuthSettings();
}

void MainWindow::readMWSettings()
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
    if (settings.value("AutoLogin").toBool()) {
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

void MainWindow::slotAbout(OBSAbout *obsAbout)
{
    const QString title = obsAbout->getTitle();
    const QString text = QString("%1<br>Revision: %2<br>Last deployment: %3").arg(obsAbout->getDescription(),
                                         obsAbout->getRevision(), obsAbout->getLastDeployment());
    QMessageBox::information(this, title, text);

    delete obsAbout;
    obsAbout = nullptr;
}

void MainWindow::slotUpdatePerson(OBSPerson *obsPerson)
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

    bool browserTabVisible = (index==0);
    actionNew->setVisible(browserTabVisible);
    ui->action_Branch_package->setVisible(browserTabVisible);
    separatorHome->setVisible(browserTabVisible);
    ui->action_Home->setVisible(browserTabVisible);
    actionBookmarks->setVisible(browserTabVisible);
    ui->action_Upload_file->setVisible(browserTabVisible);
    ui->action_Download_file->setVisible(browserTabVisible);
    ui->action_Delete->setVisible(browserTabVisible);
    ui->action_Delete->setShortcut(browserTabVisible ? QKeySequence::Delete : QKeySequence());
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

void MainWindow::on_tabWidgetPackages_currentChanged(int index)
{
    if (index==0 && browser->hasFileSelection()) {
        setupFileShortcuts();
    }
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
