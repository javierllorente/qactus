/*
 * Copyright (C) 2025 Javier Llorente <javier@opensuse.org>
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
#include "overviewwidget.h"
#include "obsrevision.h"
#include "ui_overviewwidget.h"
#include <QSettings>
#include <QTimeZone>
#include "obspkgmetaconfig.h"

OverviewWidget::OverviewWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OverviewWidget)
    , m_projectsToolbar(new QToolBar(this))
    , m_resultsMenu(nullptr)
{
    ui->setupUi(this);

    ui->packages->setVisible(false);
    ui->packagesCount->setVisible(false);
    ui->treeBuildResults->setVisible(false);
    ui->verticalLayout->addWidget(m_projectsToolbar);

    connect(ui->treeBuildResults, &BuildResultTreeWidget::customContextMenuRequested, this, &OverviewWidget::slotContextMenuResults);
    connect(this, &OverviewWidget::finishedParsingResultList, ui->treeBuildResults, &BuildResultTreeWidget::finishedAddingResults);

    QItemSelectionModel *buildResultsSelectionModel = ui->treeBuildResults->selectionModel();
    connect(buildResultsSelectionModel, &QItemSelectionModel::selectionChanged, this, &OverviewWidget::buildResultSelectionChanged);

    readSettings();
}

OverviewWidget::~OverviewWidget()
{
    writeSettings();
    delete ui;
}

void OverviewWidget::setLatestRevision(OBSRevision *revision)
{
    qDebug() << __PRETTY_FUNCTION__;
    uint unixTime = revision->getTime();
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(qint64(unixTime), QTimeZone::UTC);
    QString dateStr = dateTime.toString("dd/MM/yyyy H:mm");
    ui->latestRevision->setText(dateStr);
}

void OverviewWidget::setPackageCount(const QString &packageCount)
{
    ui->packagesCount->setText(packageCount);
}

void OverviewWidget::addResult(OBSResult *result)
{
    ui->treeBuildResults->addResult(result);
}

void OverviewWidget::setPackageCountVisible(bool visible)
{
    ui->packages->setVisible(visible);
    ui->packagesCount->setVisible(visible);
}

void OverviewWidget::setResultsVisible(bool visible)
{
    ui->treeBuildResults->setVisible(visible);
}

QString OverviewWidget::getCurrentRepository() const
{
    return ui->treeBuildResults->getCurrentRepository();
}

QString OverviewWidget::getCurrentArch() const
{
    return ui->treeBuildResults->getCurrentArch();
}

bool OverviewWidget::hasResultSelection()
{
    return ui->treeBuildResults->hasSelection();
}

void OverviewWidget::clear()
{
    ui->title->clear();
    ui->latestRevision->clear();
    ui->link->clear();
    ui->description->clear();
    // overviewProject.clear();
    // overviewPackage.clear();
    ui->packages->setVisible(false);
    ui->packagesCount->setVisible(false);
    m_dataLoaded = false;
}

void OverviewWidget::clearResultsModel()
{
    ui->treeBuildResults->clearModel();
}

void OverviewWidget::readSettings()
{
    QSettings settings;
    settings.beginGroup("Browser");
    ui->vSplitterBrowser->restoreState(settings.value("verticalSplitterSizes").toByteArray());
    settings.endGroup();
}

void OverviewWidget::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Browser");
    settings.setValue("verticalSplitterSizes", ui->vSplitterBrowser->saveState());
    settings.endGroup();
}

void OverviewWidget::setMetaConfig(OBSMetaConfig *metaConfig)
{
    qDebug() << Q_FUNC_INFO;
    QString title = metaConfig->getTitle();
    if (title.isEmpty()) {
        title = metaConfig->getName();
    }
    ui->title->setText(title);

    OBSPkgMetaConfig *pkgMetaConfig = dynamic_cast<OBSPkgMetaConfig *>(metaConfig);
    if (pkgMetaConfig) {
        QString url = pkgMetaConfig->getUrl().toString();
        ui->link->setText(!url.isEmpty() ? "<a href=\"" + url + "\">" + url + "</a>" : "");
        ui->link->setVisible(!url.isEmpty());
        // overviewPackage = metaConfig->getName();
    } else {
        // overviewProject = metaConfig->getName();
    }
    ui->link->setVisible(pkgMetaConfig);
    QString description = metaConfig->getDescription();
    if (description.isEmpty()) {
        description = "No description set";
    }
    ui->description->setText(description);
    m_dataLoaded = true;

}

void OverviewWidget::slotContextMenuResults(const QPoint &point)
{
    QModelIndex index = ui->treeBuildResults->indexAt(point);
    if (index.isValid() && m_resultsMenu) {
        m_resultsMenu->exec(ui->treeBuildResults->mapToGlobal(point));
    }
}
