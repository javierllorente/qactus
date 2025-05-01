/*
 * Copyright (C) 2015-2025 Javier Llorente <javier@opensuse.org>
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
#ifndef ROWEDITOR_H
#define ROWEDITOR_H

#include <QDialog>
#include <QCompleter>
#include <QStringListModel>
#include <QDate>
#include <QSettings>
#include <QProgressDialog>
#include <QScopedPointer>
#include "obs.h"
#include "obsprjmetaconfig.h"

namespace Ui {
class RowEditor;
}

class RowEditor : public QDialog
{
    Q_OBJECT

public:
    explicit RowEditor(QWidget *parent = nullptr, OBS *obs = nullptr);
    ~RowEditor();
    QString getProject();
    QString getPackage();
    QString getRepository();
    QString getArch();
    void setProject(const QString &);
    void setPackage(const QString &);
    void setRepository(const QString &);
    void setArch(const QString &);

private:
    Ui::RowEditor *ui;
    OBS *m_obs;
    QString getLastUpdateDate();
    void setLastUpdateDate(const QString &date);
    QStringList projectList;
    QScopedPointer<QStringListModel> projectModel;
    QScopedPointer<QCompleter> projectCompleter;
    void initProjectAutocompleter();
    QStringList packageList;
    QScopedPointer<QStringListModel> packageModel;
    QScopedPointer<QCompleter> packageCompleter;
    QList<QSharedPointer<OBSRepository>> repositories;
    QStringList repositoryList;
    QScopedPointer<QStringListModel> repositoryModel;
    QScopedPointer<QCompleter> repositoryCompleter;
    QStringList archList;
    QScopedPointer<QStringListModel> archModel;
    QScopedPointer<QCompleter> archCompleter;

private slots:
    void insertProjectList(const QStringList &list);
    void refreshProjectAutocompleter(const QString &);
    void autocompletedProjectName_clicked(const QString &projectName);
    void insertPackageList(const QStringList &list);
    void refreshPackageAutocompleter(const QString&);
    void autocompletedPackageName_clicked(const QString&);
    void insertProjectMetaConfig(QSharedPointer<OBSPrjMetaConfig> prjMetaConfig);
    void refreshRepositoryAutocompleter(const QString&);
    void autocompletedRepositoryName_clicked(const QString&repository);
    void refreshArchAutocompleter(const QString&);
};

#endif // ROWEDITOR_H
