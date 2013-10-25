#ifndef OBSREQUEST_H
#define OBSREQUEST_H

#include <QString>

class OBSrequest
{
public:
    OBSrequest();
    void setId(const QString &);
    void setActionType(const QString &);
    void setSourceProject(const QString &);
    void setSourcePackage(const QString &);
    void setTargetProject(const QString &);
    void setTargetPackage(const QString &);
    void setState(const QString &);
    void setRequester(const QString &);
    void setDate(const QString &);
    void setDescription(const QString &);

    QString getId();
    QString getActionType();
    QString getSourceProject();
    QString getSourcePackage();
    QString getSource();
    QString getTargetProject();
    QString getTargetPackage();
    QString getTarget();
    QString getState();
    QString getRequester();
    QString getDate();
    QString getDescription();

private:
    QString id;
    QString actionType;
    QString sourceProject;
    QString sourcePackage;
    QString targetProject;
    QString targetPackage;
    QString state;
    QString requester;
    QString date;
    QString description;
};

#endif // OBSREQUEST_H
