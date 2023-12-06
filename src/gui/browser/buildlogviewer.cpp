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
