qactus
======

Qactus is a Qt-based OBS client

Copyright (C) 2010-2018 Javier Llorente <javier@opensuse.org>

Qactus enables desktop users to browse an OBS server's builds, be notified on builds' status changes and
manage submit requests as email messages.
The name Qactus sprung into existence when Javier and Sivan were discussing the name and scope of the project
(will it just provide status?) which through Sivan's IRC winter typing turned out as "staqtus" which sounded like 
Qactus to Javier.

This project was originally hosted on Nokia Developer Projects until 2013, when the service was discontinued.

Installation
------------
RPM packages are available at http://software.opensuse.org/package/qactus

Dependencies
------------
Qt5
QtKeychain

Building Qactus
------------
```
cd qactus
qmake-qt5 qactus.pro DESTDIR=/install/dir
make
```

Former contributors
-------
Copyright (C) 2010-2011 Sivan Greenberg <sivan@omniqueue.com>

License
-------
This application is licensed under the GPL. See LICENSE for more details.

The icons come from the Oxygen Project (http://www.oxygen-icons.org/), with the exception of the tray icon which comes from the Open Build Service (http://openbuildservice.org/).



