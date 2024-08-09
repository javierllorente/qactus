/*
 * Copyright (C) 2018-2023 Javier Llorente <javier@opensuse.org>
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
#include "metaconfigeditor.h"
#include "ui_metaconfigeditor.h"
#include <QTreeWidget>
#include <QHeaderView>
#include <QTimer>
#include <QStandardItemModel>
#include "focusfilter.h"

MetaConfigEditor::MetaConfigEditor(QWidget *parent, OBS *obs, const QString &project, const QString &package, MCEMode mode) :
    QDialog(parent),
    ui(new Ui::MetaConfigEditor),
    m_obs(obs),
    m_project(project),
    m_package(package),
    m_mode(mode),
    packageLineEdit(nullptr),
    urlLineEdit(nullptr),
    tableRepositories(nullptr),
    buildFlagTree(nullptr),
    debugInfoFlagTree(nullptr),
    publishFlagTree(nullptr),
    useForFlagTree(nullptr),
    repositoryCompleter(new RepositoryCompleter),
    repositoryFlagsCompleter(new RepositoryFlagsCompleter),
    usersTree(nullptr),
    groupsTree(nullptr)
{
    ui->setupUi(this);
    QString windowTitle;

    switch (m_mode) {
    case MCEMode::CreateProject: {
        windowTitle = tr("Create project");
        ui->projectLineEdit->setText(project + ":");
        ui->projectLineEdit->setFocus(); // setFocus() selects text!
        QTimer::singleShot(0, ui->projectLineEdit, [this](){
            ui->projectLineEdit->deselect();
        });

        OBSPrjMetaConfig *prjMetaConfig = new OBSPrjMetaConfig();
        addDefaultMaintainer(prjMetaConfig);
        addDefaultRepositories(prjMetaConfig);
        slotFetchedProjectMetaConfig(prjMetaConfig);
        m_obs->getDistributions();

        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        break;
    }
    case MCEMode::EditProject:
        windowTitle = tr("Edit project");
        ui->projectLineEdit->setText(m_project);
        ui->projectLineEdit->setDisabled(true);
        ui->titleLineEdit->setFocus();

        connect(m_obs, &OBS::finishedParsingProjectMetaConfig, this, &MetaConfigEditor::slotFetchedProjectMetaConfig);
        m_obs->getProjectMetaConfig(m_project);
        m_obs->getDistributions();
        break;
    case MCEMode::CreatePackage: {
        windowTitle = tr("Create package");
        ui->projectLineEdit->setText(m_project);
        packageLineEdit->setFocus();
        ui->projectLineEdit->setDisabled(true);
        createUrlField();

        OBSPkgMetaConfig *pkgMetaConfig = new OBSPkgMetaConfig();
        addDefaultMaintainer(pkgMetaConfig);
        slotFetchedPackageMetaConfig(pkgMetaConfig);

        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        break;
    }
    case MCEMode::EditPackage:
        windowTitle = tr("Edit package");
        ui->projectLineEdit->setText(m_project);
        ui->projectLineEdit->setDisabled(true);
        packageLineEdit->setText("");
        packageLineEdit->setDisabled(true);
        ui->titleLineEdit->setFocus();
        createUrlField();

        connect(m_obs, &OBS::finishedParsingPackageMetaConfig, this, &MetaConfigEditor::slotFetchedPackageMetaConfig);
        m_obs->getPackageMetaConfig(m_project, m_package);
        connect(m_obs, &OBS::finishedParsingProjectMetaConfig, this, &MetaConfigEditor::slotSetupRepositoryFlagsCompleter);
        m_obs->getProjectMetaConfig(m_project);
        break;
    }

    setWindowTitle(windowTitle);

    connect(this, &MetaConfigEditor::createProject, m_obs, &OBS::createProject);
    connect(this, &MetaConfigEditor::createPackage, m_obs, &OBS::createPackage);
    connect(m_obs, &OBS::finishedParsingCreatePrjStatus, this, &MetaConfigEditor::slotCreateResult);
    connect(m_obs, &OBS::finishedParsingCreatePkgStatus, this, &MetaConfigEditor::slotCreateResult);
    connect(m_obs, &OBS::cannotCreateProject, this, &MetaConfigEditor::slotCreateResult);
    connect(m_obs, &OBS::cannotCreatePackage, this, &MetaConfigEditor::slotCreateResult);
}

MetaConfigEditor::~MetaConfigEditor()
{
    delete ui;
}

void MetaConfigEditor::on_buttonBox_accepted()
{
    QProgressDialog progress(tr("Creating..."), nullptr, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    OBSXmlWriter *xmlWriter = new OBSXmlWriter(this);
    QByteArray data;

    switch (m_mode) {
    case MCEMode::CreateProject:
    case MCEMode::EditProject: {
        OBSPrjMetaConfig *prjMetaConfig = new OBSPrjMetaConfig();
        prjMetaConfig->setName(ui->projectLineEdit->text());
        prjMetaConfig->setTitle(ui->titleLineEdit->text());
        prjMetaConfig->setDescription(ui->descriptionTextEdit->toPlainText());

        fillMetaConfigRoles(usersTree, prjMetaConfig, "userid");
        fillMetaConfigRoles(groupsTree, prjMetaConfig, "groupid");

        fillMetaConfigRepositoryFlags(buildFlagTree, prjMetaConfig, RepositoryFlag::Build);
        fillMetaConfigRepositoryFlags(debugInfoFlagTree, prjMetaConfig, RepositoryFlag::DebugInfo);
        fillMetaConfigRepositoryFlags(publishFlagTree, prjMetaConfig, RepositoryFlag::Publish);
        fillMetaConfigRepositoryFlags(useForFlagTree, prjMetaConfig, RepositoryFlag::UseForBuild);

        fillMetaConfigRepositories(tableRepositories, prjMetaConfig);

        data = xmlWriter->createProjectMeta(prjMetaConfig);
        delete prjMetaConfig;
        emit createProject(ui->projectLineEdit->text(), data);
        break;
    }
    case MCEMode::CreatePackage:
    case MCEMode::EditPackage: {
        OBSPkgMetaConfig *pkgMetaConfig = new OBSPkgMetaConfig();
        pkgMetaConfig->setName(packageLineEdit->text());
        pkgMetaConfig->setProject(ui->projectLineEdit->text());
        pkgMetaConfig->setTitle(ui->titleLineEdit->text());
        pkgMetaConfig->setUrl(QUrl(urlLineEdit->text()));
        pkgMetaConfig->setDescription(ui->descriptionTextEdit->toPlainText());

        fillMetaConfigRoles(usersTree, pkgMetaConfig, "userid");
        fillMetaConfigRoles(groupsTree, pkgMetaConfig, "groupid");

        fillMetaConfigRepositoryFlags(buildFlagTree, pkgMetaConfig, RepositoryFlag::Build);
        fillMetaConfigRepositoryFlags(debugInfoFlagTree, pkgMetaConfig, RepositoryFlag::DebugInfo);
        fillMetaConfigRepositoryFlags(publishFlagTree, pkgMetaConfig, RepositoryFlag::Publish);
        fillMetaConfigRepositoryFlags(useForFlagTree, pkgMetaConfig, RepositoryFlag::UseForBuild);

        data = xmlWriter->createPackageMeta(pkgMetaConfig);
        delete pkgMetaConfig;
        emit createPackage(ui->projectLineEdit->text(), packageLineEdit->text(), data);
        break;
    }
    }

    delete xmlWriter;
}

void MetaConfigEditor::on_buttonBox_rejected()
{
    close();
}

void MetaConfigEditor::slotCreateResult(OBSStatus *obsStatus)
{
   qDebug() << __PRETTY_FUNCTION__ << obsStatus->getCode();
   const QString title = tr("Warning");
   const QString text = QString("<b>%1</b><br>%2").arg(obsStatus->getSummary(), obsStatus->getDetails());

   if (obsStatus->getCode() == "ok") {
       close();
   } else {
       QMessageBox::warning(this, title, text);
   }
}

void MetaConfigEditor::slotFetchedProjectMetaConfig(OBSPrjMetaConfig *prjMetaConfig)
{
    fillRepositoryTab(prjMetaConfig);
    fillTabs(prjMetaConfig);
}

void MetaConfigEditor::slotSetupRepositoryFlagsCompleter(OBSPrjMetaConfig *prjMetaConfig)
{
    for (auto repository : prjMetaConfig->getRepositories()) {
        for (auto arch : repository->getArchs()) {
            repositoryFlagsCompleter->appendRepository(repository->getName());
        }
    }
}

void MetaConfigEditor::slotFetchedPackageMetaConfig(OBSPkgMetaConfig *pkgMetaConfig)
{
    packageLineEdit->setText(pkgMetaConfig->getName());
    urlLineEdit->setText(pkgMetaConfig->getUrl().toString());
    fillTabs(pkgMetaConfig);
}

void MetaConfigEditor::on_projectLineEdit_textChanged(const QString &project)
{
    bool enable = false;

    switch (m_mode) {
    case MCEMode::CreateProject:
    case MCEMode::EditProject:
        enable = !project.isEmpty();
        break;
    case MCEMode::CreatePackage:
    case MCEMode::EditPackage:
        createPackageField();
        enable = !project.isEmpty() && !packageLineEdit->text().isEmpty();
        break;
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

void MetaConfigEditor::packageTextChanged(const QString &package)
{
    bool enable = (!ui->projectLineEdit->text().isEmpty() && !package.isEmpty());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

void MetaConfigEditor::fillRepositoryTab(OBSPrjMetaConfig *prjMetaConfig)
{
    tableRepositories = createRepositoryTable();

    connect(tableRepositories, &RepositoryTreeWidget::repositoryRemoved, repositoryFlagsCompleter, &RepositoryFlagsCompleter::slotRepositoryRemoved);
    connect(tableRepositories, &RepositoryTreeWidget::itemChanged, repositoryFlagsCompleter, &RepositoryFlagsCompleter::slotItemChanged);
    connect(tableRepositories->selectionModel(), &QItemSelectionModel::selectionChanged, repositoryFlagsCompleter, &RepositoryFlagsCompleter::slotSelectionChanged);

    tableRepositories->setItemDelegate(repositoryCompleter);
    connect(m_obs, &OBS::finishedParsingDistribution, repositoryCompleter, &RepositoryCompleter::slotFetchedDistribution);
    connect(tableRepositories, &RepositoryTreeWidget::itemChanged, repositoryCompleter, &RepositoryCompleter::slotItemChanged);
    connect(tableRepositories->selectionModel(), &QItemSelectionModel::currentRowChanged, repositoryCompleter, &RepositoryCompleter::slotCurrentChanged);

    for (auto repository : prjMetaConfig->getRepositories()) {
        tableRepositories->addRepository(repository);

        for (auto arch : repository->getArchs()) {
            repositoryFlagsCompleter->appendRepository(repository->getName());
        }
    }
}

void MetaConfigEditor::fillTabs(OBSMetaConfig *metaConfig)
{
    ui->titleLineEdit->setText(metaConfig->getTitle());
    ui->descriptionTextEdit->setText(metaConfig->getDescription());

    QWidget *repositoryFlags = new QWidget();
    QGridLayout *layoutRepositoryFlags = new QGridLayout();
    repositoryFlags->setLayout(layoutRepositoryFlags);
    ui->tabWidget->insertTab(2, repositoryFlags, "Repository flags");
    buildFlagTree = createRepositoryFlagsTable("Build repository", metaConfig->getBuildFlag());
    debugInfoFlagTree = createRepositoryFlagsTable("DebugInfo repository", metaConfig->getDebugInfoFlag());
    publishFlagTree = createRepositoryFlagsTable("Publish repository", metaConfig->getPublishFlag());
    useForFlagTree = createRepositoryFlagsTable("UseForBuild repository", metaConfig->getUseForBuildFlag());

    QWidget *buildFlagWidget = createSideBar(buildFlagTree);
    QWidget *debugInfoFlagWidget = createSideBar(debugInfoFlagTree);
    QWidget *publishFlagWidget = createSideBar(publishFlagTree);
    QWidget *useForFlagWidget = createSideBar(useForFlagTree);

    layoutRepositoryFlags->setSpacing(0);
    layoutRepositoryFlags->setContentsMargins(0, 0, 0, 0);
    layoutRepositoryFlags->addWidget(buildFlagWidget);
    layoutRepositoryFlags->addWidget(debugInfoFlagWidget);
    layoutRepositoryFlags->addWidget(publishFlagWidget);
    layoutRepositoryFlags->addWidget(useForFlagWidget);


    usersTree = createRoleTable("User", metaConfig->getPersons());
    QWidget *usersWidget = createButtonBar(usersTree);

    groupsTree = createRoleTable("Group", metaConfig->getGroups());
    QWidget *groupsWidget = createButtonBar(groupsTree);

    ui->tabWidget->insertTab(3, usersWidget, "Users");
    ui->tabWidget->insertTab(4, groupsWidget, "Groups");
}

QWidget *MetaConfigEditor::createButtonBar(QTreeWidget *treeWidget)
{
    QWidget *mainWidget = new QWidget(ui->tabWidget);
    QWidget *widgetButtonBar = new QWidget(ui->tabWidget);

    QPushButton *buttonAdd = new QPushButton(treeWidget);
    buttonAdd->setIcon(QIcon::fromTheme("list-add"));
    buttonAdd->setMaximumSize(25, 25);
    connect(buttonAdd, &QPushButton::clicked, treeWidget, [treeWidget]() {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        treeWidget->addTopLevelItem(item);
        treeWidget->scrollToItem(item);
        treeWidget->setCurrentItem(item);
    });
    QPushButton *buttonRemove = new QPushButton(treeWidget);
    buttonRemove->setIcon(QIcon::fromTheme("list-remove"));
    buttonRemove->setMaximumSize(25, 25);
    buttonRemove->setShortcut(QKeySequence::Delete);
    connect(buttonRemove, &QPushButton::clicked, treeWidget, [treeWidget]() {
        QModelIndex modelIndex = treeWidget->currentIndex();
        int index = modelIndex.row();
        treeWidget->takeTopLevelItem(index);
    });

    QHBoxLayout *layoutButtonBar = new QHBoxLayout();
    layoutButtonBar->setSpacing(0);
    layoutButtonBar->setContentsMargins(0, 0, 0, 0);
    layoutButtonBar->setAlignment(Qt::AlignLeft);
    layoutButtonBar->addWidget(buttonAdd);
    layoutButtonBar->addWidget(buttonRemove);
    widgetButtonBar->setLayout(layoutButtonBar);

    QVBoxLayout *layoutTab = new QVBoxLayout();
    layoutTab->setSpacing(0);
    layoutTab->setContentsMargins(0, 0, 0, 2);
    layoutTab->addWidget(treeWidget);
    layoutTab->addWidget(widgetButtonBar);
    mainWidget->setLayout(layoutTab);

    return mainWidget;
}

QWidget *MetaConfigEditor::createSideBar(QTreeWidget *treeWidget)
{
    QWidget *mainWidget = new QWidget(ui->tabWidget);
    QWidget *widgetSideBar = new QWidget(ui->tabWidget);

    QPushButton *buttonAdd = new QPushButton(treeWidget);
    buttonAdd->setIcon(QIcon::fromTheme("list-add"));
    buttonAdd->setMaximumSize(25, 25);
    connect(buttonAdd, &QPushButton::clicked, treeWidget, [treeWidget]() {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        treeWidget->addTopLevelItem(item);
        treeWidget->scrollToItem(item);
        treeWidget->setCurrentItem(item);
        treeWidget->setFocus();
    });
    QPushButton *buttonRemove = new QPushButton(treeWidget);
    buttonRemove->setIcon(QIcon::fromTheme("list-remove"));
    buttonRemove->setMaximumSize(25, 25);

    FocusFilter *focusFilter = new FocusFilter(treeWidget);
    treeWidget->installEventFilter(focusFilter);
    connect(focusFilter, &FocusFilter::hasFocus, this, [buttonRemove](bool focus) {
        buttonRemove->setShortcut(focus ? QKeySequence::Delete : QKeySequence());
    });

    connect(buttonRemove, &QPushButton::clicked, treeWidget, [treeWidget]() {
        QModelIndex modelIndex = treeWidget->selectionModel()->currentIndex();
        int index = modelIndex.row();
        treeWidget->takeTopLevelItem(index);
    });

    QVBoxLayout *layoutButtonBar = new QVBoxLayout();
    layoutButtonBar->setSpacing(0);
    layoutButtonBar->setContentsMargins(0, 0, 0, 0);
    layoutButtonBar->setAlignment(Qt::AlignTop);
    layoutButtonBar->addWidget(buttonAdd);
    layoutButtonBar->addWidget(buttonRemove);
    widgetSideBar->setLayout(layoutButtonBar);

    QHBoxLayout *layoutSection = new QHBoxLayout();
    layoutSection->setSpacing(0);
    layoutSection->setContentsMargins(0, 0, 2, 0);
    layoutSection->addWidget(treeWidget);
    layoutSection->addWidget(widgetSideBar);
    mainWidget->setLayout(layoutSection);

    return mainWidget;
}

RepositoryTreeWidget *MetaConfigEditor::createRepositoryTable()
{
    RepositoryTreeWidget *treeWidget = new RepositoryTreeWidget(ui->tabWidget);
    QWidget *widgetTab = treeWidget->createButtonBar();
    ui->tabWidget->insertTab(1, widgetTab, "Repositories");

    return treeWidget;
}

QTreeWidget *MetaConfigEditor::createRepositoryFlagsTable(const QString &header, const QHash<QString, bool> &flag)
{
    QTreeWidgetItem *item = nullptr;
    QTreeWidget *treeWidget = new QTreeWidget(ui->tabWidget);
    treeWidget->setColumnWidth(0, 308);
    treeWidget->setRootIsDecorated(false);
    QStringList headersRepositoryFlags = QStringList() << header << "Enabled";
    treeWidget->setHeaderLabels(headersRepositoryFlags);
    treeWidget->setItemDelegate(repositoryFlagsCompleter);

    QStringList repositories = flag.keys();

    for (auto repository : repositories) {
        item = new QTreeWidgetItem();
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setText(0, repository);
        bool enabled = flag.value(repository);
        item->setText(1, QVariant(enabled).toString());
        treeWidget->addTopLevelItem(item);
    }

    return treeWidget;
}

QTreeWidget *MetaConfigEditor::createRoleTable(const QString &header, const QMultiHash<QString, QString> &userRoles)
{
    QTreeWidgetItem *item = nullptr;
    QTreeWidget *treeWidget = new QTreeWidget(ui->tabWidget);
    treeWidget->setColumnWidth(0, 150);
    treeWidget->setRootIsDecorated(false);
    treeWidget->setAlternatingRowColors(true);
    QStringList headersPersons = QStringList() << header << "Role";
    treeWidget->setHeaderLabels(headersPersons);

    QStringList users = userRoles.keys();
    QString userAdded;

    for (auto user : users) {
        QStringList roles = userRoles.values(user);

        if (user!=userAdded) {
            for (auto role : roles) {
                item = new QTreeWidgetItem();
                item->setFlags(item->flags() | Qt::ItemIsEditable);
                item->setText(0, user);
                item->setText(1, role);
                treeWidget->addTopLevelItem(item);
            }
            userAdded=user;
        }
    }
    treeWidget->sortItems(1, Qt::AscendingOrder);

    return treeWidget;
}

void MetaConfigEditor::createPackageField()
{
    packageLineEdit = new QLineEdit(ui->tabGeneral);
    QLabel *packageLabel = new QLabel(tr("Package:"), ui->tabGeneral);
    ui->layoutGeneral->insertRow(3, packageLabel, packageLineEdit);
    connect(packageLineEdit, &QLineEdit::textChanged, this, &MetaConfigEditor::packageTextChanged);
}

void MetaConfigEditor::createUrlField()
{
    urlLineEdit = new QLineEdit(ui->tabGeneral);
    QLabel *urlLabel = new QLabel(tr("URL:"), ui->tabGeneral);
    ui->layoutGeneral->insertRow(5, urlLabel, urlLineEdit);
}

void MetaConfigEditor::addDefaultMaintainer(OBSMetaConfig *metaConfig)
{
    metaConfig->insertPerson(m_obs->getUsername(), "maintainer");
}

void MetaConfigEditor::addDefaultRepositories(OBSPrjMetaConfig *prjMetaConfig)
{
    OBSRepository *tumbleweed = new OBSRepository("openSUSE_Tumbleweed", "openSUSE:Factory", "snapshot", "x86_64");
    prjMetaConfig->appendRepository(tumbleweed);

    OBSRepository *leap = new OBSRepository("openSUSE_Current", "openSUSE:Current", "standard", "x86_64");
    prjMetaConfig->appendRepository(leap);
}

void MetaConfigEditor::fillMetaConfigRoles(QTreeWidget *tree, OBSMetaConfig *metaConfig, const QString &type)
{
    int rows = tree->topLevelItemCount();
    for (int i=0; i<rows; i++) {
        if (type == "userid") {
            metaConfig->insertPerson(tree->topLevelItem(i)->text(0), tree->topLevelItem(i)->text(1));
        } else if (type == "groupid") {
            metaConfig->insertGroup(tree->topLevelItem(i)->text(0), tree->topLevelItem(i)->text(1));
        }
    }
}

void MetaConfigEditor::fillMetaConfigRepositoryFlags(QTreeWidget *tree, OBSMetaConfig *metaConfig, RepositoryFlag flag)
{
    int rows = tree->topLevelItemCount();
    for (int i=0; i<rows; i++) {
        bool enabled =  QVariant(tree->topLevelItem(i)->text(1)).toBool();
        switch (flag) {
        case RepositoryFlag::Build:
            metaConfig->insertBuildFlag(tree->topLevelItem(i)->text(0), enabled);
            break;
        case RepositoryFlag::DebugInfo:
            metaConfig->insertDebugInfoFlag(tree->topLevelItem(i)->text(0), enabled);
            break;
        case RepositoryFlag::Publish:
            metaConfig->insertPublishFlag(tree->topLevelItem(i)->text(0), enabled);
            break;
        case RepositoryFlag::UseForBuild:
            metaConfig->insertUseForBuildFlag(tree->topLevelItem(i)->text(0), enabled);
            break;
        }
    }
}

void MetaConfigEditor::fillMetaConfigRepositories(RepositoryTreeWidget *tree, OBSPrjMetaConfig *prjMetaConfig)
{
    for (auto repository : tree->getRepositories()) {
        prjMetaConfig->appendRepository(repository);
    }
}
