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

#include "browserfilter.h"
#include "ui_browserfilter.h"

BrowserFilter::BrowserFilter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BrowserFilter)
{
    ui->setupUi(this);

    connect(ui->lineEditFilter, SIGNAL(textChanged(QString)), this, SIGNAL(textChanged(QString)));
    connect(ui->radioButtonProject, SIGNAL(clicked(bool)), this, SIGNAL(projectClicked(bool)));
    connect(ui->radioButtonPackages, SIGNAL(clicked(bool)), this, SIGNAL(packageClicked(bool)));
}

BrowserFilter::~BrowserFilter()
{
    delete ui;
}

bool BrowserFilter::isProjectChecked()
{
    return ui->radioButtonProject->isChecked();
}

bool BrowserFilter::isPackageChecked()
{
    return ui->radioButtonPackages->isChecked();
}

QString BrowserFilter::getText() const
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
