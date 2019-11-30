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

#include "copypackagedialog.h"
#include "ui_copypackagedialog.h"
#include <QPushButton>
#include <QStringListModel>
#include <QCompleter>
#include <QMessageBox>
#include <QDebug>

CopyPackageDialog::CopyPackageDialog(QWidget *parent, OBS *obs, const QString &sourcePrj, const QString &sourcePkg) :
    QDialog(parent),
    ui(new Ui::CopyPackageDialog),
    m_obs(obs)
{
    ui->setupUi(this);
    ui->sourceProjectLineEdit->setText(sourcePrj);
    ui->sourceProjectLineEdit->setEnabled(false);
    ui->sourcePackageLineEdit->setText(sourcePkg);
    ui->sourcePackageLineEdit->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(ui->targetProjectLineEdit, &QLineEdit::textChanged, this, &CopyPackageDialog::toggleOkButton);
    connect(m_obs, &OBS::finishedParsingCopyPkgRevision, this, &CopyPackageDialog::revisionFetched);
    connect(m_obs, &OBS::cannotCopyPackage, this, &CopyPackageDialog::slotCannotCopyPackage);
}

CopyPackageDialog::~CopyPackageDialog()
{
    delete ui;
}

void CopyPackageDialog::addProjectList(const QStringList &projectList)
{
    QStringListModel *m_projectModel = new QStringListModel(projectList, this);
    QCompleter *m_projectCompleter = new QCompleter(m_projectModel, this);
    ui->targetProjectLineEdit->setCompleter(m_projectCompleter);
}

void CopyPackageDialog::toggleOkButton()
{
    bool enable = !ui->targetProjectLineEdit->text().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

void CopyPackageDialog::on_buttonBox_accepted()
{
    qDebug() << __PRETTY_FUNCTION__;
    // FIXME: Overwrites package without asking!
    m_obs->copyPackage(ui->sourceProjectLineEdit->text(), ui->sourcePackageLineEdit->text(),
                       ui->targetProjectLineEdit->text(), ui->sourcePackageLineEdit->text(),
                       ui->commentPlainTextEdit->toPlainText());
    emit updateStatusBar("Copying package...", false);
}

void CopyPackageDialog::revisionFetched(OBSRevision *revision)
{
    close();
    emit updateStatusBar("Done", true);
    emit showTrayMessage(APP_NAME, tr("Package %1 has been copied to %2").arg(revision->getPackage(),
                                                                         revision->getProject()));
    delete revision;
}

void CopyPackageDialog::slotCannotCopyPackage(OBSStatus *status)
{
    qDebug() << __PRETTY_FUNCTION__ << status->getCode();
    emit updateStatusBar("Cannot copy", true);
    QString title = tr("Warning");
    QString text = QString("<b>%1</b><br>%2").arg(status->getSummary(), status->getDetails());
    QMessageBox::warning(this, title, text);
    delete status;
}
