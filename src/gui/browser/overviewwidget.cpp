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
    , m_resultsToolbar(new QToolBar(this))
    , m_resultsMenu(nullptr)
{
    ui->setupUi(this);

    ui->packages->setVisible(false);
    ui->packagesCount->setVisible(false);
    ui->resultsWidget->setVisible(false);
    m_projectsToolbar->setIconSize(QSize(15, 15));
    m_resultsToolbar->setIconSize(QSize(15, 15));
    m_resultsToolbar->setVisible(false);
    ui->verticalLayout->addWidget(m_projectsToolbar);
    ui->verticalLayout_2->addWidget(m_resultsToolbar);

    connect(ui->resultsWidget, &BuildResultTreeWidget::customContextMenuRequested, this, &OverviewWidget::slotContextMenuResults);
    connect(ui->resultsWidget, &BuildResultTreeWidget::updateStatusBar, this, &OverviewWidget::updateStatusBar);
    connect(this, &OverviewWidget::finishedParsingResultList, ui->resultsWidget, &BuildResultTreeWidget::finishedAddingResults);

    QItemSelectionModel *buildResultsSelectionModel = ui->resultsWidget->selectionModel();
    connect(buildResultsSelectionModel, &QItemSelectionModel::selectionChanged, this, &OverviewWidget::buildResultSelectionChanged);

    readSettings();
}

OverviewWidget::~OverviewWidget()
{
    writeSettings();
    delete ui;
}

void OverviewWidget::addProjectActions(QList<QAction *> actions)
{
    m_projectsToolbar->addActions(actions);
}

void OverviewWidget::setResultsMenu(QMenu *resultsMenu)
{
    m_resultsMenu = resultsMenu;
    m_resultsToolbar->addActions(m_resultsMenu->actions());
}

void OverviewWidget::setLatestRevision(QSharedPointer<OBSRevision> revision)
{
    qDebug() << Q_FUNC_INFO;
    uint unixTime = revision->getTime();
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(qint64(unixTime), QTimeZone::UTC);
    QString dateStr = dateTime.toString("dd/MM/yyyy H:mm");
    ui->latestRevision->setText(dateStr);
    emit updateStatusBar(tr("Done"), true);
}

void OverviewWidget::setPackageCount(const QString &packageCount)
{
    ui->packagesCount->setText(packageCount);
}

void OverviewWidget::addResult(QSharedPointer<OBSResult> result)
{
    ui->resultsWidget->addResult(result);
}

QString OverviewWidget::getCurrentRepository() const
{
    return ui->resultsWidget->getCurrentRepository();
}

QString OverviewWidget::getCurrentArch() const
{
    return ui->resultsWidget->getCurrentArch();
}

bool OverviewWidget::hasResultSelection()
{
    return ui->resultsWidget->hasSelection();
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
    ui->resultsWidget->clearModel();
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

void OverviewWidget::setMetaConfig(QSharedPointer<OBSMetaConfig> metaConfig)
{
    qDebug() << Q_FUNC_INFO;
    QString title = metaConfig->getTitle();
    if (title.isEmpty()) {
        title = metaConfig->getName();
    }
    ui->title->setText(title);

    OBSPkgMetaConfig *pkgMetaConfig = dynamic_cast<OBSPkgMetaConfig *>(metaConfig.data());
    if (pkgMetaConfig) {
        QString url = pkgMetaConfig->getUrl().toString();
        ui->link->setText(!url.isEmpty() ? "<a href=\"" + url + "\">" + url + "</a>" : "");
        ui->link->setVisible(!url.isEmpty());
        // overviewPackage = metaConfig->getName();
        ui->packages->setVisible(false);
        ui->packagesCount->setVisible(false);

        m_projectsToolbar->setVisible(false);
        m_resultsToolbar->setVisible(true);
        m_projectsToolbar->setDisabled(false);
        m_resultsToolbar->setDisabled(false);
        emit updateStatusBar(tr("Done"), true);
    } else {
        // overviewProject = metaConfig->getName();
        ui->packages->setVisible(true);
        ui->packagesCount->setVisible(true);

        m_projectsToolbar->setVisible(true);
        m_resultsToolbar->setVisible(false);
        m_projectsToolbar->setDisabled(false);
        m_resultsToolbar->setDisabled(false);
        emit updateStatusBar(tr("Done"), true);
    }
    ui->link->setVisible(pkgMetaConfig);
    QString description = metaConfig->getDescription();
    if (description.isEmpty()) {
        description = "No description set";
    }
    ui->description->setText(description);
    m_dataLoaded = true;

}

void OverviewWidget::onPackageSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    qDebug() << Q_FUNC_INFO;
    Q_UNUSED(deselected)

    ui->resultsWidget->setVisible(!selected.isEmpty());
    m_dataLoaded = false;

    if (!selected.isEmpty()) {
        m_projectsToolbar->setVisible(false);
        m_resultsToolbar->setVisible(true);
        m_projectsToolbar->setDisabled(false);
        m_resultsToolbar->setDisabled(false);
    }
}

void OverviewWidget::onProjectNotFound(QSharedPointer<OBSStatus> status)
{
    qDebug() << Q_FUNC_INFO;
    clear();
    m_projectsToolbar->setDisabled(true);
    emit updateStatusBar(tr("Done"), true);
}

void OverviewWidget::onPackageNotFound(QSharedPointer<OBSStatus> status)
{
    qDebug() << Q_FUNC_INFO;
    m_projectsToolbar->setDisabled(true);
    m_resultsToolbar->setDisabled(true);
    emit updateStatusBar(tr("Done"), true);
}

void OverviewWidget::slotContextMenuResults(const QPoint &point)
{
    QModelIndex index = ui->resultsWidget->indexAt(point);
    if (index.isValid() && m_resultsMenu) {
        m_resultsMenu->exec(ui->resultsWidget->mapToGlobal(point));
    }
}
