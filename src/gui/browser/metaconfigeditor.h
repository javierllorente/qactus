/*
 * Copyright (C) 2018-2025 Javier Llorente <javier@opensuse.org>
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
#ifndef METACONFIGEDITOR_H
#define METACONFIGEDITOR_H

#include <QDialog>
#include <QProgressDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QTreeWidget>
#include <QSharedPointer>
#include "obs.h"
#include "obsxmlwriter.h"
#include "obsprjmetaconfig.h"
#include "obspkgmetaconfig.h"
#include "repositorycompleter.h"
#include "repositoryflagscompleter.h"
#include "repositorytreewidget.h"

namespace Ui {
class MetaConfigEditor;
}

enum class MCEMode {
    CreateProject,
    EditProject,
    CreatePackage,
    EditPackage
};

class MetaConfigEditor : public QDialog
{
    Q_OBJECT

public:
    explicit MetaConfigEditor(QWidget *parent = nullptr, OBS *obs = nullptr, const QString &project = QString(), const QString &package = QString(), MCEMode mode = MCEMode::EditPackage);
    ~MetaConfigEditor();

signals:
    void createProject(QString, QByteArray);
    void createPackage(QString, QString, QByteArray);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void slotCreateResult(QSharedPointer<OBSStatus> status);
    void slotFetchedProjectMetaConfig(QSharedPointer<OBSPrjMetaConfig> prjMetaConfig);
    void slotSetupRepositoryFlagsCompleter(QSharedPointer<OBSPrjMetaConfig> prjMetaConfig);
    void slotFetchedPackageMetaConfig(QSharedPointer<OBSPkgMetaConfig> pkgMetaConfig);
    void on_projectLineEdit_textChanged(const QString &project);
    void packageTextChanged(const QString &package);

private:
    Ui::MetaConfigEditor *ui;
    OBS *m_obs;
    QString m_project;
    QString m_package;
    MCEMode m_mode;
    QLineEdit *packageLineEdit;
    QLineEdit *urlLineEdit;
    RepositoryTreeWidget *tableRepositories;
    QTreeWidget *buildFlagTree;
    QTreeWidget *debugInfoFlagTree;
    QTreeWidget *publishFlagTree;
    QTreeWidget *useForFlagTree;
    RepositoryCompleter *repositoryCompleter;
    RepositoryFlagsCompleter *repositoryFlagsCompleter;
    QTreeWidget *usersTree;
    QTreeWidget *groupsTree;
    enum class RepositoryFlag { Build, DebugInfo, Publish, UseForBuild };
    void fillRepositoryTab(QSharedPointer<OBSPrjMetaConfig> prjMetaConfig);
    void fillTabs(QSharedPointer<OBSMetaConfig> metaConfig);
    QWidget *createButtonBar(QTreeWidget *treeWidget);
    QWidget *createSideBar(QTreeWidget *treeWidget);
    RepositoryTreeWidget *createRepositoryTable();
    QTreeWidget *createRepositoryFlagsTable(const QString &header, const QHash<QString, bool> &flag);
    QTreeWidget *createRoleTable(const QString &header, const QMultiHash<QString, QString> &userRoles);
    void createPackageField();
    void createUrlField();
    void addDefaultMaintainer(QSharedPointer<OBSMetaConfig> metaConfig);
    void addDefaultRepositories(QSharedPointer<OBSPrjMetaConfig> prjMetaConfig);
    void fillMetaConfigRoles(QTreeWidget *tree, QSharedPointer<OBSMetaConfig> metaConfig, const QString &type);
    void fillMetaConfigRepositoryFlags(QTreeWidget *tree, QSharedPointer<OBSMetaConfig> metaConfig, RepositoryFlag flag);
    void fillMetaConfigRepositories(RepositoryTreeWidget *tree, QSharedPointer<OBSPrjMetaConfig> prjMetaConfig);

};

#endif // METACONFIGEDITOR_H
