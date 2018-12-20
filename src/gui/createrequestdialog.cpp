/*
 *  Qactus - A Qt-based OBS client
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

#include "createrequestdialog.h"
#include "ui_createrequestdialog.h"

CreateRequestDialog::CreateRequestDialog(OBSRequest *request, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateRequestDialog),
    m_request(request)
{
    ui->setupUi(this);
    ui->sourceProjectLineEdit->setText(request->getSourceProject());
}

CreateRequestDialog::~CreateRequestDialog()
{
    delete ui;
}

void CreateRequestDialog::on_buttonBox_accepted()
{
    m_request->setTargetProject(ui->targetProjectLineEdit->text());
    m_request->setTargetPackage(ui->targetPackageLineEdit->text());
    m_request->setDescription(ui->descriptionPlainTextEdit->toPlainText());
    if (ui->sourceUpdateCheckBox->isChecked()) {
        m_request->setSourceUpdate("cleanup");
    }

    OBSXmlWriter *xmlWriter = new OBSXmlWriter(this);
    QByteArray data = xmlWriter->createRequest(m_request);
    delete xmlWriter;

    emit createRequest(data);
}
