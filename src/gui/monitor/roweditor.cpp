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
    m_obs(obs)
{
    ui->setupUi(this);

    initProjectAutocompleter();
}

RowEditor::~RowEditor()
{
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
    qDebug() << Q_FUNC_INFO;
    connect(m_obs, &OBS::finishedParsingProjectList, this, &RowEditor::insertProjectList);
    m_obs->getProjects();
}

void RowEditor::insertProjectList(const QStringList &list)
{
    qDebug() << Q_FUNC_INFO;
    projectList = list;
    projectModel.reset(new QStringListModel(projectList));
    projectCompleter.reset(new QCompleter(projectModel.get(), this));

    ui->lineEditProject->setCompleter(projectCompleter.get());

    connect(ui->lineEditProject, &QLineEdit::textEdited,
            this, &RowEditor::refreshProjectAutocompleter);
    connect(projectCompleter.get(), QOverload<const QString &>::of(&QCompleter::activated),
            this, &RowEditor::autocompletedProjectName_clicked);
}

void RowEditor::refreshProjectAutocompleter(const QString&)
{
    projectModel->setStringList(projectList);
}

void RowEditor::autocompletedProjectName_clicked(const QString &projectName)
{
    ui->lineEditPackage->setFocus();

    connect(m_obs, &OBS::finishedParsingPackageList, this, &RowEditor::insertPackageList);
    m_obs->getPackages(projectName);
}

void RowEditor::insertPackageList(const QStringList &list)
{
    qDebug() << Q_FUNC_INFO;
    packageList = list;
    packageModel.reset(new QStringListModel(packageList));
    packageCompleter.reset(new QCompleter(packageModel.get(), this));

    ui->lineEditPackage->setCompleter(packageCompleter.get());

    connect(ui->lineEditPackage, &QLineEdit::textEdited,
            this, &RowEditor::refreshPackageAutocompleter);
    connect(packageCompleter.get(), QOverload<const QString &>::of(&QCompleter::activated),
            this, &RowEditor::autocompletedPackageName_clicked);
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
    qDebug() << Q_FUNC_INFO;
    repositories = prjMetaConfig->getRepositories();

    for (auto repository : repositories) {
        repositoryList.append(repository->getName());
    }

    repositoryModel.reset(new QStringListModel(repositoryList));
    repositoryCompleter.reset(new QCompleter(repositoryModel.get(), this));

    ui->lineEditRepository->setCompleter(repositoryCompleter.get());

    connect(ui->lineEditRepository, &QLineEdit::textEdited,
            this, &RowEditor::refreshRepositoryAutocompleter);
    connect(repositoryCompleter.get(), QOverload<const QString &>::of(&QCompleter::activated),
            this, &RowEditor::autocompletedRepositoryName_clicked);
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

    archModel.reset(new QStringListModel(archList));
    archCompleter.reset(new QCompleter(archModel.get(), this));

    ui->lineEditArch->setCompleter(archCompleter.get());

//    connect(ui->lineEditArch, SIGNAL(textEdited(const QString&)),
//            this, SLOT(refreshArchAutocompleter(const QString&)));
}

void RowEditor::refreshArchAutocompleter(const QString&)
{
//    archModel->setStringList(archList);
}
