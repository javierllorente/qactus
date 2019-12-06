/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2018-2019 Javier Llorente <javier@opensuse.org>
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

#include "metaconfigeditor.h"
#include "ui_metaconfigeditor.h"
#include <QTreeWidget>
#include <QHeaderView>
#include <QTimer>

MetaConfigEditor::MetaConfigEditor(QWidget *parent, OBS *obs, const QString &project, const QString &package, MCEMode mode) :
    QDialog(parent),
    ui(new Ui::MetaConfigEditor),
    m_obs(obs),
    m_project(project),
    m_package(package),
    m_mode(mode),
    packageLineEdit(nullptr),
    urlLineEdit(nullptr),
    buildFlagTree(nullptr),
    debugInfoFlagTree(nullptr),
    publishFlagTree(nullptr),
    useForFlagTree(nullptr)
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

        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        break;
    }
    case MCEMode::EditProject:
        windowTitle = tr("Edit project");
        ui->projectLineEdit->setText(m_project);
        ui->projectLineEdit->setDisabled(true);
        ui->titleLineEdit->setFocus();
        m_obs->getProjectMetaConfig(m_project);
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
        m_obs->getPackageMetaConfig(m_project, m_package);
        break;
    }

    setWindowTitle(windowTitle);

    connect(this, &MetaConfigEditor::createProject, m_obs, &OBS::createProject);
    connect(this, &MetaConfigEditor::createPackage, m_obs, &OBS::createPackage);
    connect(m_obs, &OBS::finishedParsingCreatePrjStatus, this, &MetaConfigEditor::slotCreateResult);
    connect(m_obs, &OBS::finishedParsingCreatePkgStatus, this, &MetaConfigEditor::slotCreateResult);
    connect(m_obs, &OBS::cannotCreateProject, this, &MetaConfigEditor::slotCreateResult);
    connect(m_obs, &OBS::cannotCreatePackage, this, &MetaConfigEditor::slotCreateResult);

    connect(m_obs, &OBS::finishedParsingProjectMetaConfig, this, &MetaConfigEditor::slotFetchedProjectMetaConfig);
    connect(m_obs, &OBS::finishedParsingPackageMetaConfig, this, &MetaConfigEditor::slotFetchedPackageMetaConfig);
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

        QTreeWidget *usersTree = static_cast<QTreeWidget *>(ui->tabWidget->widget(3));
        QTreeWidget *groupsTree = static_cast<QTreeWidget *>(ui->tabWidget->widget(4));

        fillMetaConfigRoles(usersTree, prjMetaConfig, "userid");
        fillMetaConfigRoles(groupsTree, prjMetaConfig, "groupid");

        fillMetaConfigRepositoryFlags(buildFlagTree, prjMetaConfig, RepositoryFlag::Build);
        fillMetaConfigRepositoryFlags(debugInfoFlagTree, prjMetaConfig, RepositoryFlag::DebugInfo);
        fillMetaConfigRepositoryFlags(publishFlagTree, prjMetaConfig, RepositoryFlag::Publish);
        fillMetaConfigRepositoryFlags(useForFlagTree, prjMetaConfig, RepositoryFlag::UseForBuild);

        QTreeWidget *repositoryTree = static_cast<QTreeWidget *>(ui->tabWidget->widget(1));
        fillMetaConfigRepositories(repositoryTree, prjMetaConfig);

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

        QTreeWidget *usersTree = static_cast<QTreeWidget *>(ui->tabWidget->widget(2));
        QTreeWidget *groupsTree = static_cast<QTreeWidget *>(ui->tabWidget->widget(3));

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
   delete obsStatus;
   obsStatus = nullptr;
}

void MetaConfigEditor::slotFetchedProjectMetaConfig(OBSPrjMetaConfig *prjMetaConfig)
{
    QTreeWidgetItem *item = nullptr;
    QTreeWidget *tableRepositories = new QTreeWidget(ui->tabWidget);
    tableRepositories->setColumnWidth(0, 180);
    tableRepositories->setRootIsDecorated(false);
    tableRepositories->setAlternatingRowColors(true);
    QStringList headers = QStringList() << "Repository" << "Arch" << "Path";
    tableRepositories->setHeaderLabels(headers);
    ui->tabWidget->insertTab(1, tableRepositories, "Repositories");

    for (auto repository : prjMetaConfig->getRepositories()) {
        for (auto arch : repository->getArchs()) {
            item = new QTreeWidgetItem();
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            item->setText(0, repository->getName());
            item->setText(1, arch);
            item->setText(2, repository->getProject() + "/" + repository->getRepository());
            tableRepositories->addTopLevelItem(item);
        }
    }

    fillTabs(prjMetaConfig);
    delete prjMetaConfig;
}

void MetaConfigEditor::slotFetchedPackageMetaConfig(OBSPkgMetaConfig *pkgMetaConfig)
{
    packageLineEdit->setText(pkgMetaConfig->getName());
    urlLineEdit->setText(pkgMetaConfig->getUrl().toString());
    fillTabs(pkgMetaConfig);
    delete pkgMetaConfig;
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
    layoutRepositoryFlags->addWidget(buildFlagTree);
    layoutRepositoryFlags->addWidget(debugInfoFlagTree);
    layoutRepositoryFlags->addWidget(publishFlagTree);
    layoutRepositoryFlags->addWidget(useForFlagTree);

    ui->tabWidget->insertTab(3, createRoleTable("User", metaConfig->getPersons()), "Users");
    ui->tabWidget->insertTab(4, createRoleTable("Group", metaConfig->getGroups()), "Groups");
}

QTreeWidget *MetaConfigEditor::createRepositoryFlagsTable(const QString &header, const QHash<QString, bool> &flag)
{
    QTreeWidgetItem *item = nullptr;
    QTreeWidget *treeWidget = new QTreeWidget(ui->tabWidget);
    treeWidget->setColumnWidth(0, 325);
    treeWidget->setRootIsDecorated(false);
    QStringList headersRepositoryFlags = QStringList() << header << "Enabled";
    treeWidget->setHeaderLabels(headersRepositoryFlags);

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

void MetaConfigEditor::fillMetaConfigRepositories(QTreeWidget *tree, OBSPrjMetaConfig *prjMetaConfig)
{
    int rows = tree->topLevelItemCount();
    OBSRepository *repository = nullptr;
    QStringList path;

    for (int i=0; i<rows; i++) {
        path = tree->topLevelItem(i)->text(2).split("/");
        if (path.count()>1) {
            repository = new OBSRepository(tree->topLevelItem(i)->text(0),
                                           path.at(0), path.at(1),
                                           tree->topLevelItem(i)->text(1));
            prjMetaConfig->appendRepository(repository);
        }
    }
}
