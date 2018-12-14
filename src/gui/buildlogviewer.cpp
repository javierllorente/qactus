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

#include "buildlogviewer.h"
#include "ui_buildlogviewer.h"

BuildLogViewer::BuildLogViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BuildLogViewer)
{
    ui->setupUi(this);
    QAction *findAction = new QAction(this);
    findAction->setShortcut(QKeySequence("Ctrl+F"));
    connect(findAction, SIGNAL(triggered()), this, SLOT(findText()));
    addAction(findAction);

    ui->plainTextEdit->ensureCursorVisible();
}

BuildLogViewer::~BuildLogViewer()
{
    delete ui;
}

void BuildLogViewer::setText(const QString &text)
{
    ui->plainTextEdit->setPlainText(text);
    scrollToBottom();
}

void BuildLogViewer::scrollToBottom()
{
    int value = ui->plainTextEdit->verticalScrollBar()->maximum();
    ui->plainTextEdit->verticalScrollBar()->setValue(value);
}

void BuildLogViewer::findText()
{
    const int widgetIndex = 0;
    SearchWidget *searchWidget = nullptr;

    if (ui->verticalLayout->count() == 2) {
        searchWidget = new SearchWidget(this, ui->plainTextEdit);
        ui->verticalLayout->insertWidget(widgetIndex, searchWidget);
    } else if (ui->verticalLayout->count() == 3) {
        searchWidget = static_cast<SearchWidget *>(ui->verticalLayout->itemAt(widgetIndex)->widget());
    }
    searchWidget->setup();
}
