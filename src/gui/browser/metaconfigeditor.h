/*
 *  Qactus - A Qt based OBS notifier
 *
 *  Copyright (C) 2018-2019 Javier Llorente <javier@opensuse.org>
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

#ifndef METACONFIGEDITOR_H
#define METACONFIGEDITOR_H

#include <QDialog>
#include <QProgressDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QTreeWidget>
#include "obs.h"
#include "obsxmlwriter.h"
#include "obsprjmetaconfig.h"
#include "obspkgmetaconfig.h"

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
    void slotCreateResult(OBSStatus *obsStatus);
    void slotFetchedProjectMetaConfig(OBSPrjMetaConfig *prjMetaConfig);
    void slotFetchedPackageMetaConfig(OBSPkgMetaConfig *pkgMetaConfig);
    void on_projectLineEdit_textChanged(const QString &project);
    void packageTextChanged(const QString &package);

private:
    Ui::MetaConfigEditor *ui;
    enum class RepositoryFlag { Build, DebugInfo, Publish, UseForBuild };
    void fillTabs(OBSMetaConfig *metaConfig);
    QTreeWidget *createRepositoryTable(const QString &header, const QHash<QString, bool> &flag);
    QTreeWidget *createRoleTable(const QString &header, const QMultiHash<QString, QString> &userRoles);
    void createPackageField();
    void createUrlField();
    void addDefaultMaintainer(OBSMetaConfig *metaConfig);
    void addDefaultRepositories(OBSPrjMetaConfig *prjMetaConfig);
    void fillMetaConfigRoles(QTreeWidget *tree, OBSMetaConfig *metaConfig, const QString &type);
    void fillMetaConfigRepositoryFlags(QTreeWidget *tree, OBSMetaConfig *metaConfig, RepositoryFlag flag);
    void fillMetaConfigRepositories(QTreeWidget *tree, OBSPrjMetaConfig *prjMetaConfig);
    OBS *m_obs;
    QString m_project;
    QString m_package;
    MCEMode m_mode;
    QLineEdit *packageLineEdit;
    QLineEdit *urlLineEdit;
    QTreeWidget *buildFlagTree;
    QTreeWidget *debugInfoFlagTree;
    QTreeWidget *publishFlagTree;
    QTreeWidget *useForFlagTree;

};

#endif // METACONFIGEDITOR_H
