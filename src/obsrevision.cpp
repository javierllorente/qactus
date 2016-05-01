#include "obsrevision.h"

OBSRevision::OBSRevision()
{

}

uint OBSRevision::getRev() const
{
    return rev;
}

void OBSRevision::setRev(const uint &value)
{
    rev = value;
}

QString OBSRevision::getVersion() const
{
    return version;
}

void OBSRevision::setVersion(const QString &value)
{
    version = value;
}

uint OBSRevision::getTime() const
{
    return time;
}

void OBSRevision::setTime(const uint &value)
{
    time = value;
}

QString OBSRevision::getUser() const
{
    return user;
}

void OBSRevision::setUser(const QString &value)
{
    user = value;
}

QString OBSRevision::getComment() const
{
    return comment;
}

void OBSRevision::setComment(const QString &value)
{
    comment = value;
}
