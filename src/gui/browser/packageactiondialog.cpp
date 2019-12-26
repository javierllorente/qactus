/*
 *  Qactus - A Qt-based OBS client
 *
 *  Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
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
    delete revision;
    close();
}

void PackageActionDialog::slotCannot(OBSStatus *status)
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
    delete status;
}
