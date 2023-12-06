/*
 * Copyright (C) 2018-2020 Javier Llorente <javier@opensuse.org>
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
#include "browserfilter.h"
#include "ui_browserfilter.h"
#include <QDebug>
#include <QTimer>
#include <QIcon>

BrowserFilter::BrowserFilter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BrowserFilter),
    m_projectModel(new QStringListModel(this)),
    m_projectCompleter(new QCompleter(m_projectModel, this))
{
    ui->setupUi(this);

    QIcon filterIcon(QIcon::fromTheme("go-next"));
    ui->lineEditFilter->addAction(filterIcon, QLineEdit::LeadingPosition);

    QTimer::singleShot(0, ui->lineEditFilter, [this](){
        ui->lineEditFilter->setFocus();
    });

    m_projectCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    ui->lineEditFilter->setCompleter(m_projectCompleter);

    connect(m_projectCompleter, QOverload<const QString &>::of(&QCompleter::activated),
            this, &BrowserFilter::autocompletedProject_clicked);
}

BrowserFilter::~BrowserFilter()
{
    delete ui;
}

QString BrowserFilter::text() const
{
    return ui->lineEditFilter->text();
}

void BrowserFilter::clear()
{
    ui->lineEditFilter->clear();
}

void BrowserFilter::setFocus()
{
    ui->lineEditFilter->setFocus();
}

void BrowserFilter::addProjectList(const QStringList &projectList)
{
    qDebug() << __PRETTY_FUNCTION__;
    m_projectModel->setStringList(projectList);
}

void BrowserFilter::autocompletedProject_clicked(const QString &project)
{
    emit setCurrentProject(project);
}
