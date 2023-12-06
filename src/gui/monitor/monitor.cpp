/*
 * Copyright (C) 2019 Javier Llorente <javier@opensuse.org>
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
#include "monitor.h"
#include "ui_monitor.h"
#include "monitorrepositorytab.h"
#include "monitorpackagestab.h"

Monitor::Monitor(QWidget *parent, OBS *obs) :
    QWidget(parent),
    ui(new Ui::Monitor),
    m_obs(obs)
{
    ui->setupUi(this);

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [&]() {
        emit itemSelectionChanged();
    });
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &Monitor::closeTab);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &Monitor::currentTabChanged);

    setupPackagesTab();

    readSettings();
}

Monitor::~Monitor()
{
    writeSettings();
    delete ui;
}

bool Monitor::hasPackageSelection()
{
    return (ui->tabWidget->currentIndex() == 0) &&
            dynamic_cast<MonitorTab *>(ui->tabWidget->widget(0))->hasSelection();
}

void Monitor::refresh()
{
    for (int i=0; i<ui->tabWidget->count(); i++) {
        dynamic_cast<MonitorTab *>(ui->tabWidget->widget(i))->refresh();
    }
}

bool Monitor::tabWidgetContains(const QString &tabText)
{
    for (int i=0; i<ui->tabWidget->count(); i++) {
        if (ui->tabWidget->tabText(i) == tabText) {
            return true;
        }
    }
    return false;
}

void Monitor::setupTabConnections(MonitorTab *tab)
{
    connect(this, &Monitor::markAllRead, tab, &MonitorTab::slotMarkAllRead);
    connect(tab, &MonitorTab::notifyChanged, this, &Monitor::notifyChanged);
    connect(tab, &MonitorTab::updateStatusBar, this, &Monitor::updateStatusBar);
}

int Monitor::addTab(const QString &title)
{
    emit updateStatusBar(tr("Adding tab for ") + title + " ...", false);
    MonitorTab *tab = new MonitorRepositoryTab(ui->tabWidget, title, m_obs);
    setupTabConnections(tab);
    return ui->tabWidget->addTab(tab, title);
    emit updateStatusBar(tr("Tab for ") + title + tr(" added"), true);
}

void Monitor::readSettings()
{
    QSettings settings;
    int size = settings.beginReadArray("Monitor.Repositories");
    for (int i=1; i<size; i++) {
        settings.setArrayIndex(i);
        addTab(settings.value("Project", ui->tabWidget->tabText(i)).toString());
    }
    settings.endArray();

    settings.beginGroup("Monitor.General");
    ui->tabWidget->setCurrentIndex(settings.value("VisibleTab").toInt());
    settings.endGroup();
}

void Monitor::writeSettings()
{
    QSettings settings;
    settings.beginWriteArray("Monitor.Repositories");

    for (int i=1; i<ui->tabWidget->count(); i++) {
        settings.setArrayIndex(i);
        settings.setValue("Project", ui->tabWidget->tabText(i));
    }
    settings.endArray();

    settings.beginGroup("Monitor.General");
    settings.setValue("VisibleTab", ui->tabWidget->currentIndex());
    settings.endGroup();
}

void Monitor::setupPackagesTab()
{
    QString packagesTab = tr("My packages");
    MonitorTab *tab = new MonitorPackagesTab(ui->tabWidget, packagesTab, m_obs);

    connect(dynamic_cast<MonitorPackagesTab *>(tab), &MonitorPackagesTab::itemSelectionChanged, this, &Monitor::itemSelectionChanged);
    connect(this, &Monitor::removeRow, dynamic_cast<MonitorPackagesTab *>(tab), &MonitorPackagesTab::slotRemoveRow);
    connect(this, &Monitor::addRow, dynamic_cast<MonitorPackagesTab *>(tab), &MonitorPackagesTab::slotAddRow);
    setupTabConnections(tab);
    ui->tabWidget->addTab(tab, packagesTab);
    ui->tabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->hide();
}

void Monitor::closeTab(int index)
{
    delete ui->tabWidget->widget(index);

    QSettings settings;
    settings.beginReadArray("Monitor.Repositories");
    settings.setArrayIndex(index);
    settings.remove("Project");
    settings.endArray();
}
