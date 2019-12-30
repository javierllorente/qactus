# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)

# Qt5Keychain requires Qt5DBus, but doesn't quite set this up right
find_package(Qt5DBus REQUIRED)
find_package(Qt5Keychain REQUIRED)

include_directories(
    browser
    monitor
    requestbox
)

set(QACTUS_SRC
    browser/browser.cpp
    browser/buildlogviewer.cpp
    browser/buildresulttreewidget.cpp
    browser/metaconfigeditor.cpp
    browser/repositorycompleter.cpp
    browser/repositoryflagscompleter.cpp
    browser/packageactiondialog.cpp
    browser/createrequestdialog.cpp
    browser/filetreewidget.cpp
    browser/loghighlighter.cpp
    browser/packagelistmodel.cpp
    browser/packagetreewidget.cpp
    browser/projectlistmodel.cpp
    browser/projecttreewidget.cpp
    browser/searchwidget.cpp
    monitor/monitor.cpp
    monitor/monitortreewidget.cpp
    monitor/roweditor.cpp
    requestbox/requestbox.cpp
    requestbox/requestboxtreewidget.cpp
    requestbox/requestitemmodel.cpp
    requestbox/requeststateeditor.cpp
    requestbox/requesttreewidget.cpp
    requestbox/syntaxhighlighter.cpp
    main.cpp
    mainwindow.cpp
    iconbar.cpp
    trayicon.cpp
    configure.cpp
    login.cpp
    autotooltipdelegate.cpp
    utils.cpp
    credentials.cpp
    browserfilter.cpp
    bookmarks.cpp)

set(QACTUS_HDR
    browser/browser.h
    browser/buildlogviewer.h
    browser/buildresulttreewidget.h
    browser/metaconfigeditor.h
    browser/repositorycompleter.h
    browser/repositoryflagscompleter.h
    browser/packageactiondialog.h
    browser/createrequestdialog.h
    browser/filetreewidget.h
    browser/loghighlighter.h
    browser/packagelistmodel.h
    browser/packagetreewidget.h
    browser/projectlistmodel.h
    browser/projecttreewidget.h
    browser/searchwidget.h
    monitor/monitor.h
    monitor/monitortreewidget.h
    monitor/roweditor.h
    requestbox/requestbox.h
    requestbox/requestboxtreewidget.h
    requestbox/requestitemmodel.h
    requestbox/requeststateeditor.h
    requestbox/requesttreewidget.h
    requestbox/syntaxhighlighter.h
    mainwindow.h
    iconbar.h
    trayicon.h
    configure.h
    login.h
    autotooltipdelegate.h
    utils.h
    credentials.h
    browserfilter.h
    bookmarks.h)

set(QACTUS_UI
    browser/browser.ui
    browser/buildlogviewer.ui
    browser/metaconfigeditor.ui
    browser/packageactiondialog.ui
    browser/createrequestdialog.ui
    browser/searchwidget.ui
    monitor/monitor.ui
    monitor/roweditor.ui
    requestbox/requestbox.ui
    requestbox/requeststateeditor.ui
    mainwindow.ui
    configure.ui
    login.ui
    browserfilter.ui)

set(QACTUS_RC
    ../../application.qrc)

qt5_wrap_ui(QACTUS_UI_SRC ${QACTUS_UI})

qt5_add_resources(QACTUS_RC_SRC ${QACTUS_RC})

add_executable(qactus ${QACTUS_SRC} ${QACTUS_UI_SRC} ${QACTUS_RC_SRC})

add_dependencies(qactus libqobs)

target_include_directories(qactus PRIVATE ../qobs)

target_link_libraries(qactus libqobs Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network ${QTKEYCHAIN_LIBRARIES})

install(TARGETS qactus RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
install(FILES ${CMAKE_SOURCE_DIR}/qactus.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
install(FILES ${CMAKE_SOURCE_DIR}/icons/qactus.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps)
