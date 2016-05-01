#ifndef OBSREVISION_H
#define OBSREVISION_H

#include <QString>

class OBSRevision
{
public:
    OBSRevision();

    uint getRev() const;
    void setRev(const uint &value);

    QString getVersion() const;
    void setVersion(const QString &value);

    uint getTime() const;
    void setTime(const uint &value);

    QString getUser() const;
    void setUser(const QString &value);

    QString getComment() const;
    void setComment(const QString &value);

private:
    uint rev;
    QString version;
    uint time;
    QString user;
    QString comment;
};

#endif // OBSREVISION_H
