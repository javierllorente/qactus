/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2018 Javier Llorente <javier@opensuse.org>
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

CreateDialog::CreateDialog(OBS *obs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateDialog),
    mOBS(obs)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    projectMode = false;
    setWindowTitle(tr("Create package"));

    connect(this, SIGNAL(createProject(QString,QByteArray)), mOBS, SLOT(createProject(QString,QByteArray)));
    connect(this, SIGNAL(createPackage(QString,QString,QByteArray)), mOBS, SLOT(createPackage(QString,QString,QByteArray)));
    connect(mOBS, SIGNAL(finishedParsingCreatePrjStatus(OBSStatus*)), this, SLOT(slotCreateResult(OBSStatus*)));
    connect(mOBS, SIGNAL(finishedParsingCreatePkgStatus(OBSStatus*)), this, SLOT(slotCreateResult(OBSStatus*)));
    connect(mOBS, SIGNAL(cannotCreateProject(OBSStatus*)), this, SLOT(slotCreateResult(OBSStatus*)));
    connect(mOBS, SIGNAL(cannotCreatePackage(OBSStatus*)), this, SLOT(slotCreateResult(OBSStatus*)));
}

CreateDialog::~CreateDialog()
{
    delete ui;
}

void CreateDialog::setProjectMode(bool projectMode)
{
    this->projectMode = projectMode;
    if (projectMode) {
        setWindowTitle(tr("Create project"));
        ui->packageLabel->hide();
        ui->packageLineEdit->hide();
    }
}

void CreateDialog::setProject(const QString &project)
{
    QString projectStr = project;
    if (projectMode) {
        projectStr += ":";
    }
    ui->projectLineEdit->setText(projectStr);

   if (!projectMode && !project.isEmpty()) {
       ui->packageLineEdit->setFocus();
       ui->projectLineEdit->setDisabled(true);
   }
}

void CreateDialog::on_buttonBox_accepted()
{
    QProgressDialog progress(tr("Creating..."), 0, 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    OBSXmlWriter *xmlWriter = new OBSXmlWriter(this);
    QByteArray data;

    if (projectMode) {
        data = xmlWriter->createProjectMeta(ui->projectLineEdit->text(),
                                     ui->titleLineEdit->text(),
                                     ui->descriptionTextEdit->toPlainText(),
                                     mOBS->getUsername());
        emit createProject(ui->projectLineEdit->text(), data);
    } else {
        data = xmlWriter->createPackageMeta(ui->projectLineEdit->text(),
                                     ui->packageLineEdit->text(),
                                     ui->titleLineEdit->text(),
                                     ui->descriptionTextEdit->toPlainText(),
                                     mOBS->getUsername());
        emit createPackage(ui->projectLineEdit->text(), ui->packageLineEdit->text(), data);
    }

    delete xmlWriter;
    xmlWriter = nullptr;
}

void CreateDialog::on_buttonBox_rejected()
{
    close();
}

void CreateDialog::slotCreateResult(OBSStatus *obsStatus)
{
   qDebug() << "CreateDialog::slotCreateResult()" << obsStatus->getCode();
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

void CreateDialog::on_projectLineEdit_textChanged(const QString &project)
{
    bool enable = (projectMode && !project.isEmpty()) ||
            (!projectMode && !project.isEmpty() && !ui->packageLineEdit->text().isEmpty());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

void CreateDialog::on_packageLineEdit_textChanged(const QString &package)
{
    bool enable = (!ui->projectLineEdit->text().isEmpty() && !package.isEmpty());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}
