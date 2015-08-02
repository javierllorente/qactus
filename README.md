qactus
======

Qactus is a Qt-based OBS notifier application

Copyright (C) 2010-2015 Javier Llorente <javier@opensuse.org>

Copyright (C) 2010-2011 Sivan Greenberg <sivan@omniqueue.com>


Qactus enables desktop and mobile users to retrieve status and control various aspects of their builds on the OBS, 
Open Build Service. 
The name Qactus sprung into existence when the two authors were discussing the name and scope of the project 
(will it just provide status?) which through Sivan's IRC winter typing turned out as "staqtus" which sounded like 
Qactus to Javier.

This project was originally hosted on Nokia Developer Projects until 2013, when the service was discontinued.

Installation
------------
openSUSE packages are available at http://software.opensuse.org/package/qactus

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

License
-------
This application is licensed under the GPL. See LICENSE for more details.

The icons come from the Oxygen Project (http://www.oxygen-icons.org/), with the exception of the tray icon which comes from the Open Build Service (http://openbuildservice.org/).



