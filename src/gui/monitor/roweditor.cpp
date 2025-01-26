/*
 * Copyright (C) 2015-2025 Javier Llorente <javier@opensuse.org>
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
#include "roweditor.h"
#include "ui_roweditor.h"

RowEditor::RowEditor(QWidget *parent, OBS *obs) :
    QDialog(parent),
    ui(new Ui::RowEditor),
    m_obs(obs),
    projectModel(nullptr),
    projectCompleter(nullptr),
    packageModel(nullptr),
    packageCompleter(nullptr),
    repositoryModel(nullptr),
    repositoryCompleter(nullptr),
    archModel(nullptr),
    archCompleter(nullptr)
{
    ui->setupUi(this);

    initProjectAutocompleter();
}

RowEditor::~RowEditor()
{
    delete projectModel;
    delete projectCompleter;
    delete packageModel;
    delete packageCompleter;
    delete repositoryModel;
    delete repositoryCompleter;
    delete archModel;
    delete archCompleter;
    delete ui;
}

QString RowEditor::getLastUpdateDate()
{
    QSettings settings("Qactus","Qactus");
    settings.beginGroup("LastUpdate");
    QString lastUpdateItem = settings.value("ProjectList").toString();
    settings.endGroup();
    return lastUpdateItem;
}

void RowEditor::setLastUpdateDate(const QString &date)
{
    QSettings settings("Qactus","Qactus");
    settings.beginGroup("LastUpdate");
    settings.setValue("ProjectList", date);
    settings.endGroup();
}

QString RowEditor::getProject()
{
    return ui->lineEditProject->text();
}

QString RowEditor::getPackage()
{
    return ui->lineEditPackage->text();
}

QString RowEditor::getRepository()
{
    return ui->lineEditRepository->text();
}

QString RowEditor::getArch()
{
    return ui->lineEditArch->text();
}

void RowEditor::setProject(const QString &project)
{
    ui->lineEditProject->setText(project);
}

void RowEditor::setPackage(const QString &package)
{
    ui->lineEditPackage->setText(package);
}

void RowEditor::setRepository(const QString &repository)
{
    ui->lineEditRepository->setText(repository);
}

void RowEditor::setArch(const QString &arch)
{
    ui->lineEditArch->setText(arch);
}

void RowEditor::initProjectAutocompleter()
{
    qDebug() << "RowEditor::initProjectAutocompleter()";
    connect(m_obs, SIGNAL(finishedParsingProjectList(QStringList)), this, SLOT(insertProjectList(QStringList)));
    m_obs->getProjects();
}

void RowEditor::insertProjectList(const QStringList &list)
{
    qDebug() << "RowEditor::insertProjectList()";
    projectList = list;
    projectModel = new QStringListModel(projectList);
    projectCompleter = new QCompleter(projectModel, this);

    ui->lineEditProject->setCompleter(projectCompleter);

    connect(ui->lineEditProject, SIGNAL(textEdited(const QString&)),
            this, SLOT(refreshProjectAutocompleter(const QString&)));
    connect(projectCompleter, SIGNAL(activated(const QString&)),
            this, SLOT(autocompletedProjectName_clicked(const QString&)));
}

void RowEditor::refreshProjectAutocompleter(const QString&)
{
    projectModel->setStringList(projectList);
}

void RowEditor::autocompletedProjectName_clicked(const QString &projectName)
{
    ui->lineEditPackage->setFocus();

    connect(m_obs, SIGNAL(finishedParsingPackageList(QStringList)), this, SLOT(insertPackageList(QStringList)));
    m_obs->getPackages(projectName);
}

void RowEditor::insertPackageList(const QStringList &list)
{
    qDebug() << "RowEditor::insertPackageList()";
    packageList = list;
    packageModel = new QStringListModel(packageList);
    packageCompleter = new QCompleter(packageModel, this);

    ui->lineEditPackage->setCompleter(packageCompleter);

    connect(ui->lineEditPackage, SIGNAL(textEdited(const QString&)),
            this, SLOT(refreshPackageAutocompleter(const QString&)));
    connect(packageCompleter, SIGNAL(activated(const QString&)),
            this, SLOT(autocompletedPackageName_clicked(const QString&)));
}

void RowEditor::refreshPackageAutocompleter(const QString&)
{
    packageModel->setStringList(packageList);
}

void RowEditor::autocompletedPackageName_clicked(const QString&)
{
    ui->lineEditRepository->setFocus();

    connect(m_obs, &OBS::finishedParsingProjectMetaConfig, this, &RowEditor::insertProjectMetaConfig);
    QString project = ui->lineEditProject->text();
    m_obs->getProjectMetaConfig(project);
}

void RowEditor::insertProjectMetaConfig(QSharedPointer<OBSPrjMetaConfig> prjMetaConfig)
{
    qDebug() << __PRETTY_FUNCTION__;
    repositories = prjMetaConfig->getRepositories();

    for (auto repository : repositories) {
        repositoryList.append(repository->getName());
    }

    repositoryModel = new QStringListModel(repositoryList);
    repositoryCompleter = new QCompleter(repositoryModel, this);

    ui->lineEditRepository->setCompleter(repositoryCompleter);

    connect(ui->lineEditRepository, SIGNAL(textEdited(const QString&)),
            this, SLOT(refreshRepositoryAutocompleter(const QString&)));
    connect(repositoryCompleter, SIGNAL(activated(const QString&)),
            this, SLOT(autocompletedRepositoryName_clicked(const QString&)));
}

void RowEditor::refreshRepositoryAutocompleter(const QString&)
{
    repositoryModel->setStringList(repositoryList);
}

void RowEditor::autocompletedRepositoryName_clicked(const QString &repository)
{
    ui->lineEditArch->setFocus();

    for (auto r : repositories) {
        if (r->getName()==repository) {
            archList = r->getArchs();
        }
    }

    archModel = new QStringListModel(archList);
    archCompleter = new QCompleter(archModel, this);

    ui->lineEditArch->setCompleter(archCompleter);

//    connect(ui->lineEditArch, SIGNAL(textEdited(const QString&)),
//            this, SLOT(refreshArchAutocompleter(const QString&)));
}

void RowEditor::refreshArchAutocompleter(const QString&)
{
//    archModel->setStringList(archList);
}
