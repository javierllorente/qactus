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

#include "createdialog.h"
#include "ui_createdialog.h"

CreateDialog::CreateDialog(QWidget *parent, OBS *obs, const QString &project, const QString &package, Mode mode) :
    QDialog(parent),
    ui(new Ui::CreateDialog),
    m_obs(obs),
    m_project(project),
    m_package(package),
    m_mode(mode)
{
    ui->setupUi(this);
    QString windowTitle;

    switch (m_mode) {
    case Mode::CreateProject:
        windowTitle = tr("Create project");
        ui->projectLineEdit->setText(project + ":");
        ui->packageLabel->hide();
        ui->packageLineEdit->hide();
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        break;
    case Mode::EditProject:
        windowTitle = tr("Edit project");
        ui->projectLineEdit->setText(m_project);
        ui->projectLineEdit->setDisabled(true);
        ui->packageLabel->hide();
        ui->packageLineEdit->hide();
        ui->titleLineEdit->setFocus();
        m_obs->getProjectMetaConfig(m_project);
        break;
    case Mode::CreatePackage:
        windowTitle = tr("Create package");
        ui->projectLineEdit->setText(m_project);
        ui->packageLineEdit->setFocus();
        ui->projectLineEdit->setDisabled(true);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        break;
    case Mode::EditPackage:
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

    connect(this, &CreateDialog::createProject, m_obs, &OBS::createProject);
    connect(this, &CreateDialog::createPackage, m_obs, &OBS::createPackage);
    connect(m_obs, &OBS::finishedParsingCreatePrjStatus, this, &CreateDialog::slotCreateResult);
    connect(m_obs, &OBS::finishedParsingCreatePkgStatus, this, &CreateDialog::slotCreateResult);
    connect(m_obs, &OBS::cannotCreateProject, this, &CreateDialog::slotCreateResult);
    connect(m_obs, &OBS::cannotCreatePackage, this, &CreateDialog::slotCreateResult);

    connect(m_obs, &OBS::finishedParsingProjectMetaConfig, this, &CreateDialog::slotFetchedProjectMetaConfig);
    connect(m_obs, &OBS::finishedParsingPackageMetaConfig, this, &CreateDialog::slotFetchedPackageMetaConfig);
}

CreateDialog::~CreateDialog()
{
    delete ui;
}

void CreateDialog::on_buttonBox_accepted()
{
    QProgressDialog progress(tr("Creating..."), nullptr, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    OBSXmlWriter *xmlWriter = new OBSXmlWriter(this);
    QByteArray data;

    switch (m_mode) {
    case Mode::CreateProject:
    case Mode::EditProject:
        data = xmlWriter->createProjectMeta(ui->projectLineEdit->text(),
                                            ui->titleLineEdit->text(),
                                            ui->descriptionTextEdit->toPlainText(),
                                            m_obs->getUsername());
        emit createProject(ui->projectLineEdit->text(), data);
        break;
    case Mode::CreatePackage:
    case Mode::EditPackage:
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

void CreateDialog::on_buttonBox_rejected()
{
    close();
}

void CreateDialog::slotCreateResult(OBSStatus *obsStatus)
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

void CreateDialog::slotFetchedProjectMetaConfig(OBSPrjMetaConfig *prjMetaConfig)
{
    ui->titleLineEdit->setText(prjMetaConfig->getTitle());
    ui->descriptionTextEdit->setText(prjMetaConfig->getDescription());

    delete prjMetaConfig;
}

void CreateDialog::slotFetchedPackageMetaConfig(OBSPkgMetaConfig *pkgMetaConfig)
{
    ui->titleLineEdit->setText(pkgMetaConfig->getTitle());
    ui->descriptionTextEdit->setText(pkgMetaConfig->getDescription());
    ui->packageLineEdit->setText(pkgMetaConfig->getName());

    delete pkgMetaConfig;
}

void CreateDialog::on_projectLineEdit_textChanged(const QString &project)
{
    bool enable = false;

    switch (m_mode) {
    case Mode::CreateProject:
    case Mode::EditProject:
        enable = !project.isEmpty();
        break;
    case Mode::CreatePackage:
    case Mode::EditPackage:
        enable = !project.isEmpty() && !ui->packageLineEdit->text().isEmpty();
        break;
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

void CreateDialog::on_packageLineEdit_textChanged(const QString &package)
{
    bool enable = (!ui->projectLineEdit->text().isEmpty() && !package.isEmpty());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}
