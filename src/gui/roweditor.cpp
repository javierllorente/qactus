/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2015-2018 Javier Llorente <javier@opensuse.org>
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

#include "roweditor.h"
#include "ui_roweditor.h"

RowEditor::RowEditor(QWidget *parent, OBS *obs) :
    QDialog(parent),
    ui(new Ui::RowEditor),
    mOBS(obs)
{
    ui->setupUi(this);
    projectModel = NULL;
    projectCompleter = NULL;
    packageModel = NULL;
    packageCompleter = NULL;
    repositoryModel = NULL;
    repositoryCompleter = NULL;
    archModel = NULL;
    archCompleter = NULL;
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
    connect(mOBS, SIGNAL(finishedParsingProjectList(QStringList)), this, SLOT(insertProjectList(QStringList)));
    mOBS->getProjects();
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

    connect(mOBS, SIGNAL(finishedParsingPackageList(QStringList)), this, SLOT(insertPackageList(QStringList)));
    mOBS->getPackages(projectName);
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

    connect(mOBS, SIGNAL(finishedParsingProjectMetadata(QStringList)), this, SLOT(insertProjectMetadata(QStringList)));
    QString project = ui->lineEditProject->text();
    mOBS->getProjectMetadata(project);
}

void RowEditor::insertProjectMetadata(const QStringList &list)
{
    qDebug() << "RowEditor::insertProjectMetadata()";
    repositoryList = list;
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

    archList = mOBS->getRepositoryArchs(repository);
    archModel = new QStringListModel(archList);
    archCompleter = new QCompleter(archModel, this);

    ui->lineEditArch->setCompleter(archCompleter);

    connect(ui->lineEditArch, SIGNAL(textEdited(const QString&)),
            this, SLOT(refreshArchAutocompleter(const QString&)));
}

void RowEditor::refreshArchAutocompleter(const QString&)
{
    archModel->setStringList(archList);
}
