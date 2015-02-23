/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2015 Javier Llorente <javier@opensuse.org>
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

RowEditor::RowEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RowEditor)
{
    ui->setupUi(this);

    QString lastUpdateStr = getLastUpdateDate();
    QDate lastUpdateDate = QDate::fromString(lastUpdateStr);
    QString dataDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    QDir::setCurrent(dataDir);
    QString fileName = "projects.xml";

    /* projects.xml is downloaded if
     * it doesn't exist or
     * there is no lastupdate entry in settings file or
     * 7 days have passed since projects.xml was downloaded
     */
    if (!QFile::exists(fileName) ||
            lastUpdateStr.isEmpty() ||
            lastUpdateDate.daysTo(QDate::currentDate()) == -7) {
        OBSaccess *obsAccess = OBSaccess::getInstance();
        if (obsAccess->isAuthenticated()) {
            qDebug() << "Downloading project list...";
            stringList = obsAccess->getProjectList();
            setLastUpdateDate(QDate::currentDate().toString());
        }
    } else {
        qDebug() << "Reading project list...";
        xmlReader = new OBSxmlReader();
        xmlReader->readFile(fileName);
        stringList = xmlReader->getProjectList();
    }

    initProjectAutocompleter(stringList);
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

void RowEditor::initProjectAutocompleter(const QStringList &stringList)
{
    QStringListModel *model = new QStringListModel(stringList);
    projectCompleter = new QCompleter(model, this);

    ui->lineEditProject->setCompleter(projectCompleter);

    connect(ui->lineEditProject, SIGNAL(textEdited(const QString&)),
            this, SLOT(refreshProjectAutocompleter(const QString&)));
    connect(projectCompleter, SIGNAL(activated(const QString&)),
            this, SLOT(autocompletedProjectName_clicked(const QString&)));
}

void RowEditor::refreshProjectAutocompleter(const QString&)
{
    QStringListModel *model = (QStringListModel*)(projectCompleter->model());
    model->setStringList(stringList);
}

void RowEditor::autocompletedProjectName_clicked(const QString&)
{
    ui->lineEditPackage->setFocus();
}
