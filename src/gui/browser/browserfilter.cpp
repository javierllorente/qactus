/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2018-2020 Javier Llorente <javier@opensuse.org>
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
