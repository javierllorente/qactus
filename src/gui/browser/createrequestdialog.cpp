/*
 * Copyright (C) 2018 Javier Llorente <javier@opensuse.org>
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
#include "createrequestdialog.h"
#include "ui_createrequestdialog.h"

CreateRequestDialog::CreateRequestDialog(OBSRequest *request, OBS *obs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateRequestDialog),
    m_request(request),
    m_obs(obs),
    m_projectModel(nullptr),
    m_projectCompleter(nullptr),
    m_packageModel(nullptr),
    m_packageCompleter(nullptr)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->sourceProjectLineEdit->setText(request->getSourceProject());

    connect(m_obs, &OBS::finishedParsingPackageList, this, &CreateRequestDialog::addPackageList);
    connect(m_obs, &OBS::finishedParsingLink, this, &CreateRequestDialog::linkFetched);
    connect(ui->targetProjectLineEdit, &QLineEdit::textChanged, this, &CreateRequestDialog::toggleOkButton);
    connect(ui->targetPackageLineEdit, &QLineEdit::textChanged, this, &CreateRequestDialog::toggleOkButton);
    connect(ui->descriptionPlainTextEdit, &QPlainTextEdit::textChanged, this, &CreateRequestDialog::toggleOkButton);
    connect(this, &CreateRequestDialog::createRequest, obs, &OBS::createRequest);
}

CreateRequestDialog::~CreateRequestDialog()
{
    delete ui;
}

void CreateRequestDialog::addProjectList(const QStringList &projectList)
{
    qDebug() << "CreateRequestDialog::addProjectList()";
    m_projectModel = new QStringListModel(projectList, this);
    m_projectCompleter = new QCompleter(m_projectModel, this);
    ui->targetProjectLineEdit->setCompleter(m_projectCompleter);

#if QT_VERSION >= 0x050700
    connect(m_projectCompleter, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CreateRequestDialog::autocompletedProject_activated);
#else
    connect(m_projectCompleter, static_cast<void (QCompleter::*)(const QString &)>(&QCompleter::activated),
            this, &CreateRequestDialog::autocompletedProject_activated);
#endif
}

void CreateRequestDialog::autocompletedProject_activated(const QString &project)
{
    ui->targetPackageLineEdit->setFocus();
    m_obs->getPackages(project);
}

void CreateRequestDialog::addPackageList(const QStringList &packageList)
{
    qDebug() << "CreateRequestDialog::addPackageList()";
    m_packageModel = new QStringListModel(packageList, this);
    m_packageCompleter = new QCompleter(m_packageModel, this);
    ui->targetPackageLineEdit->setCompleter(m_packageCompleter);

#if QT_VERSION >= 0x050700
    connect(m_packageCompleter, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CreateRequestDialog::autocompletedPackage_activated);
#else
    connect(m_packageCompleter, static_cast<void (QCompleter::*)(const QString &)>(&QCompleter::activated),
            this, &CreateRequestDialog::autocompletedPackage_activated);
#endif
}

void CreateRequestDialog::linkFetched(OBSLink *link)
{
    QString project = link->getProject().isEmpty() ? m_request->getSourceProject() : link->getProject();
    ui->targetProjectLineEdit->setText(project);

    QString package = link->getPackage().isEmpty() ? m_request->getSourcePackage() : link->getPackage();
    m_obs->getPackages(project); // Make autocomplete work when filling the target package automatically
    ui->targetPackageLineEdit->setText(package);

    ui->descriptionPlainTextEdit->setFocus();
    delete link;
}

void CreateRequestDialog::autocompletedPackage_activated(const QString &package)
{
    Q_UNUSED(package);
    ui->descriptionPlainTextEdit->setFocus();
}

void CreateRequestDialog::toggleOkButton()
{
    bool enable = !ui->targetProjectLineEdit->text().isEmpty() &&
            !ui->targetPackageLineEdit->text().isEmpty() &&
            !ui->descriptionPlainTextEdit->toPlainText().isEmpty();

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
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
