/*
 * Copyright (C) 2018-2024 Javier Llorente <javier@opensuse.org>
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
#include "locationbar.h"
#include "ui_locationbar.h"
#include <QDebug>
#include <QTimer>
#include <QIcon>

LocationBar::LocationBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LocationBar),
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
            this, &LocationBar::autocompletedProject_clicked);
    connect(ui->lineEditFilter, &QLineEdit::returnPressed, this, [=](){
        emit returnPressed(ui->lineEditFilter->text());
    });
}

LocationBar::~LocationBar()
{
    delete ui;
}

QString LocationBar::text() const
{
    return ui->lineEditFilter->text();
}

void LocationBar::setText(const QString &text)
{
    ui->lineEditFilter->setText(text);
}

void LocationBar::clear()
{
    ui->lineEditFilter->clear();
}

void LocationBar::setFocus()
{
    ui->lineEditFilter->setFocus();
}

QStringList LocationBar::getProjectList() const
{
    return m_projectModel->stringList();
}

bool LocationBar::addProject(const QString &project)
{
   if (m_projectModel->insertRow(m_projectModel->rowCount())) {
       QModelIndex index = m_projectModel->index(m_projectModel->rowCount() - 1, 0);
       return m_projectModel->setData(index, project);
   }
   return false;
}

bool LocationBar::removeProject(const QString &project)
{
    int index = m_projectModel->stringList().indexOf(project);
    if (index == -1) {
        return false;
    }
    m_projectModel->removeRow(index);
    return true;
}

void LocationBar::addProjectList(const QStringList &projectList)
{
    qDebug() << __PRETTY_FUNCTION__;
    ui->lineEditFilter->clear();
    m_projectModel->setStringList(projectList);
}

void LocationBar::autocompletedProject_clicked(const QString &project)
{
    emit setCurrentProject(project);
}
