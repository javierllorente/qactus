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

    QString lastUpdateStr = readSettings();
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
        qDebug() << "Downloading project list...";
        OBSaccess *obsAccess = OBSaccess::getInstance();
        QUrl url("https://api.opensuse.org/source");
        obsAccess->setUrl(url);
        obsAccess->makeRequest();
        stringList = obsAccess->getProjectList();
        writeSettings();
    } else {
        qDebug() << "Reading project list...";
        xmlReader = new OBSxmlReader();
        xmlReader->readFile(fileName);
        stringList = xmlReader->getProjectList();
    }

    initAutocomplete(stringList);
}

RowEditor::~RowEditor()
{
    delete ui;
}

QString RowEditor::readSettings()
{
    QSettings settings("Qactus","Qactus");
    settings.beginGroup("ProjectList");
    QString lastUpdateItem = settings.value("lastupdate").toString();
    settings.endGroup();
    return lastUpdateItem;
}

void RowEditor::writeSettings()
{
    QSettings settings("Qactus","Qactus");
    settings.beginGroup("ProjectList");
    settings.setValue("lastupdate",  QDate::currentDate().toString());
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

void RowEditor::initAutocomplete(const QStringList &stringList)
{
    QStringListModel *model = new QStringListModel(stringList);
    completer = new QCompleter(model, this);

    ui->lineEditProject->setCompleter(completer);

    connect(ui->lineEditProject, SIGNAL(textEdited(const QString&)),
            this, SLOT(refreshAutocomplete(const QString&)));
}

void RowEditor::refreshAutocomplete(const QString&)
{
    QStringListModel *model = (QStringListModel*)(completer->model());
    model->setStringList(stringList);
}
