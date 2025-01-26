/*
 * Copyright (C) 2019-2025 Javier Llorente <javier@opensuse.org>
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
#include "packageactiondialog.h"
#include "ui_packageactiondialog.h"
#include <QPushButton>
#include <QStringListModel>
#include <QCompleter>
#include <QMessageBox>
#include <QDebug>

PackageActionDialog::PackageActionDialog(QWidget *parent, OBS *obs,
                                         const QString &srcProject, const QString &srcPackage, PackageAction action) :
    QDialog(parent),
    ui(new Ui::PackageActionDialog),
    m_obs(obs),
    m_action(action)
{
    ui->setupUi(this);
    ui->sourceProjectLineEdit->setText(srcProject);
    ui->sourceProjectLineEdit->setEnabled(false);
    ui->sourcePackageLineEdit->setText(srcPackage);
    ui->sourcePackageLineEdit->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(ui->targetProjectLineEdit, &QLineEdit::textChanged, this, &PackageActionDialog::toggleOkButton);

    QString windowTitle;

    switch (m_action) {
    case PackageAction::LinkPackage:
        windowTitle = tr("Link package");
        ui->label_3->setText(tr("Destination project:"));
        ui->label_4->setHidden(true);
        ui->commentPlainTextEdit->setHidden(true);
        resize(width(), 150);
        connect(m_obs, &OBS::finishedParsingLinkPkgRevision, this, &PackageActionDialog::revisionFetched);
        connect(m_obs, &OBS::cannotLinkPackage, this, &PackageActionDialog::slotCannot);
        break;
    case PackageAction::CopyPackage:
        windowTitle = tr("Copy package");
        connect(m_obs, &OBS::finishedParsingCopyPkgRevision, this, &PackageActionDialog::revisionFetched);
        connect(m_obs, &OBS::cannotCopyPackage, this, &PackageActionDialog::slotCannot);
        break;
    default:
        qDebug() << __PRETTY_FUNCTION__ << "Error: Unknown case!";
    }

    setWindowTitle(windowTitle);
}

PackageActionDialog::~PackageActionDialog()
{
    delete ui;
}

void PackageActionDialog::addProjectList(const QStringList &projectList)
{
    QStringListModel *projectModel = new QStringListModel(projectList, this);
    QCompleter *projectCompleter = new QCompleter(projectModel, this);
    ui->targetProjectLineEdit->setCompleter(projectCompleter);
}

void PackageActionDialog::toggleOkButton()
{
    bool enable = !ui->targetProjectLineEdit->text().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

void PackageActionDialog::on_buttonBox_accepted()
{
    qDebug() << __PRETTY_FUNCTION__;
    QString message;

    switch (m_action) {
    case PackageAction::LinkPackage:
        m_obs->linkPackage(ui->sourceProjectLineEdit->text(), ui->sourcePackageLineEdit->text(),
                           ui->targetProjectLineEdit->text());
        message = tr("Linking package...");
        break;
    case PackageAction::CopyPackage:
        // FIXME: Overwrites package without asking!
        m_obs->copyPackage(ui->sourceProjectLineEdit->text(), ui->sourcePackageLineEdit->text(),
                           ui->targetProjectLineEdit->text(), ui->sourcePackageLineEdit->text(),
                           ui->commentPlainTextEdit->toPlainText());
        message = tr("Copying package..");
        break;
    default:
        qDebug() << __PRETTY_FUNCTION__ << "Error: Unknown case!";
    }

    emit updateStatusBar(message, false);
}

void PackageActionDialog::revisionFetched(OBSRevision *revision)
{
    QString message;

    switch (m_action) {
    case PackageAction::LinkPackage:
        message = tr("Package %1 has been linked in %2").arg(revision->getPackage(),
                                                                                 revision->getProject());
        break;
    case PackageAction::CopyPackage:
        message = tr("Package %1 has been copied to %2").arg(revision->getPackage(),
                                                                                 revision->getProject());
        break;
    default:
        qDebug() << __PRETTY_FUNCTION__ << "Error: Unknown case!";
    }

    emit updateStatusBar("Done", true);
    emit showTrayMessage(APP_NAME, message);
    close();
}

void PackageActionDialog::slotCannot(QSharedPointer<OBSStatus> status)
{
    qDebug() << __PRETTY_FUNCTION__ << status->getCode();

    QString message;
    switch (m_action) {
    case PackageAction::LinkPackage:
        message = tr("Cannot link");
        break;
    case PackageAction::CopyPackage:
        message = tr("Cannot copy");
        break;
    default:
        qDebug() << __PRETTY_FUNCTION__ << "Error: Unknown case!";
    }

    emit updateStatusBar(message, true);
    QString title = tr("Warning");
    QString text = QString("<b>%1</b><br>%2").arg(status->getSummary(), status->getDetails());
    QMessageBox::warning(this, title, text);
}
