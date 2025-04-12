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
#ifndef OVERVIEWWIDGET_H
#define OVERVIEWWIDGET_H

#include <QWidget>
#include <QToolBar>
#include <QMenu>
#include <QItemSelection>
#include <QSharedPointer>
#include "datacontroller.h"
#include "obsmetaconfig.h"
#include "obsresult.h"
#include "obsrevision.h"

namespace Ui {
class OverviewWidget;
}

class OverviewWidget : public QWidget, public DataController
{
    Q_OBJECT

public:
    explicit OverviewWidget(QWidget *parent = nullptr);
    ~OverviewWidget();
    void addProjectActions(QList <QAction *> actions);
    void setResultsMenu(QMenu *resultsMenu);
    void setLatestRevision(QSharedPointer<OBSRevision> revision);
    void setPackageCount(const QString &packageCount);
    void addResult(QSharedPointer<OBSResult> result);
    QString getCurrentRepository() const;
    QString getCurrentArch() const;
    QList<OBSResult> getBuilds() const;
    bool hasResultSelection();
    void clear();
    void clearResultsModel();

private:
    void readSettings();
    void writeSettings();
    Ui::OverviewWidget *ui;
    QToolBar *m_projectsToolbar;
    QToolBar *m_resultsToolbar;
    QMenu *m_resultsMenu;
    bool m_dataLoaded;

signals:
    void buildResultSelectionChanged();
    void finishedParsingResultList(QList<QSharedPointer<OBSResult>> resultList);
    void updateStatusBar(const QString &message, bool progressBarHidden);

public slots:
    void setMetaConfig(QSharedPointer<OBSMetaConfig> metaConfig);
    void onPackageSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onProjectNotFound(QSharedPointer<OBSStatus> status);
    void onPackageNotFound(QSharedPointer<OBSStatus> status);

private slots:
    void slotContextMenuResults(const QPoint &point);
};

#endif // OVERVIEWWIDGET_H
