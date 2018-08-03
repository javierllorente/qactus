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

#include "buildlogviewer.h"
#include "ui_buildlogviewer.h"

BuildLogViewer::BuildLogViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BuildLogViewer)
{
    ui->setupUi(this);

}

BuildLogViewer::~BuildLogViewer()
{
    delete ui;
}

void BuildLogViewer::setText(const QString &text)
{
    ui->textEdit->setText(text);
    scrollToBottom();
}

void BuildLogViewer::scrollToBottom()
{
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
}
