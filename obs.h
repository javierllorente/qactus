#ifndef OBS_H
#define OBS_H

#include <QObject>
#include "obsaccess.h"
#include "obsxmlreader.h"
#include "obspackage.h"

class OBS : public QObject
{
    Q_OBJECT

public:
    explicit OBS(QObject *parent = 0);
    void setCredentials(const QString &username, const QString &password);
    QString getUsername();
    void setApiUrl(const QString &apiUrl);
    void request(const QString& urlStr);
    void request(const QString &urlStr, const int &row);
    void postRequest(const QString &urlStr, const QByteArray &data);
    bool isAuthenticated();
    void login();
    OBSPackage* getBuildStatus(const QStringList &stringList, const int &row);
    QList<OBSRequest*> getRequests();
    int getRequestCount();
    QString acceptRequest(const QString &id, const QString &comments);
    QString declineRequest(const QString &id, const QString &comments);
    QString getRequestDiff(const QString &source);
    QStringList getProjectList();
    QStringList getProjectPackageList(const QString &projectName);
    QStringList getProjectMetadata(const QString &projectName);
    OBSXmlReader* getXmlReader();

private:
    OBSAccess *obsAccess;
    OBSXmlReader *xmlReader;
    QString apiUrl;

signals:
    void isAuthenticated(bool);
    void finishedParsingPackage(OBSPackage*, const int&);
    void finishedParsingRequests(QList<OBSRequest*>);
    void finishedParsingList(QStringList);

public slots:

};

#endif // OBS_H
