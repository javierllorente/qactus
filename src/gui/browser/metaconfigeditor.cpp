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

MetaConfigEditor::MetaConfigEditor(QWidget *parent, OBS *obs, const QString &project, const QString &package, MCEMode mode) :
    QDialog(parent),
    ui(new Ui::MetaConfigEditor),
    m_obs(obs),
    m_project(project),
    m_package(package),
    m_mode(mode)
{
    ui->setupUi(this);
    QString windowTitle;

    switch (m_mode) {
    case MCEMode::CreateProject:
        windowTitle = tr("Create project");
        ui->projectLineEdit->setText(project + ":");
        ui->packageLabel->hide();
        ui->packageLineEdit->hide();
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        break;
    case MCEMode::EditProject:
        windowTitle = tr("Edit project");
        ui->projectLineEdit->setText(m_project);
        ui->projectLineEdit->setDisabled(true);
        ui->packageLabel->hide();
        ui->packageLineEdit->hide();
        ui->titleLineEdit->setFocus();
        m_obs->getProjectMetaConfig(m_project);
        break;
    case MCEMode::CreatePackage:
        windowTitle = tr("Create package");
        ui->projectLineEdit->setText(m_project);
        ui->packageLineEdit->setFocus();
        ui->projectLineEdit->setDisabled(true);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        break;
    case MCEMode::EditPackage:
        windowTitle = tr("Edit package");
        ui->projectLineEdit->setText(m_project);
        ui->projectLineEdit->setDisabled(true);
        ui->packageLineEdit->setText("");
        ui->packageLineEdit->setDisabled(true);
        ui->titleLineEdit->setFocus();
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
    case MCEMode::EditProject:
        data = xmlWriter->createProjectMeta(ui->projectLineEdit->text(),
                                            ui->titleLineEdit->text(),
                                            ui->descriptionTextEdit->toPlainText(),
                                            m_obs->getUsername());
        emit createProject(ui->projectLineEdit->text(), data);
        break;
    case MCEMode::CreatePackage:
    case MCEMode::EditPackage:
        data = xmlWriter->createPackageMeta(ui->projectLineEdit->text(),
                                            ui->packageLineEdit->text(),
                                            ui->titleLineEdit->text(),
                                            ui->descriptionTextEdit->toPlainText(),
                                            m_obs->getUsername());
        emit createPackage(ui->projectLineEdit->text(), ui->packageLineEdit->text(), data);
        break;
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
    ui->titleLineEdit->setText(prjMetaConfig->getTitle());
    ui->descriptionTextEdit->setText(prjMetaConfig->getDescription());

    delete prjMetaConfig;
}

void MetaConfigEditor::slotFetchedPackageMetaConfig(OBSPkgMetaConfig *pkgMetaConfig)
{
    ui->titleLineEdit->setText(pkgMetaConfig->getTitle());
    ui->descriptionTextEdit->setText(pkgMetaConfig->getDescription());
    ui->packageLineEdit->setText(pkgMetaConfig->getName());

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
        enable = !project.isEmpty() && !ui->packageLineEdit->text().isEmpty();
        break;
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

void MetaConfigEditor::on_packageLineEdit_textChanged(const QString &package)
{
    bool enable = (!ui->projectLineEdit->text().isEmpty() && !package.isEmpty());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}
