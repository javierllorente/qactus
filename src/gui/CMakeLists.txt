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
)

set(QACTUS_SRC
    browser/browser.cpp
    browser/buildlogviewer.cpp
    browser/buildresulttreewidget.cpp
    browser/createdialog.cpp
    browser/createrequestdialog.cpp
    browser/filetreewidget.cpp
    browser/loghighlighter.cpp
    browser/packagelistmodel.cpp
    browser/packagetreewidget.cpp
    browser/projectlistmodel.cpp
    browser/projecttreewidget.cpp
    browser/searchwidget.cpp
    main.cpp
    mainwindow.cpp
    iconbar.cpp
    trayicon.cpp
    configure.cpp
    login.cpp
    roweditor.cpp
    requeststateeditor.cpp
    autotooltipdelegate.cpp
    utils.cpp
    credentials.cpp
    syntaxhighlighter.cpp
    browserfilter.cpp
    monitortreewidget.cpp
    requestboxtreewidget.cpp
    requesttreewidget.cpp
    requestitemmodel.cpp
    bookmarks.cpp)

set(QACTUS_HDR
    browser/browser.h
    browser/buildlogviewer.h
    browser/buildresulttreewidget.h
    browser/createdialog.h
    browser/createrequestdialog.h
    browser/filetreewidget.h
    browser/loghighlighter.h
    browser/packagelistmodel.h
    browser/packagetreewidget.h
    browser/projectlistmodel.h
    browser/projecttreewidget.h
    browser/searchwidget.h
    mainwindow.h
    iconbar.h
    trayicon.h
    configure.h
    login.h
    roweditor.h
    requeststateeditor.h
    autotooltipdelegate.h
    utils.h
    credentials.h
    syntaxhighlighter.h
    browserfilter.h
    monitortreewidget.h
    requestboxtreewidget.h
    requesttreewidget.h
    requestitemmodel.h
    bookmarks.h)

set(QACTUS_UI
    browser/browser.ui
    browser/buildlogviewer.ui
    browser/createdialog.ui
    browser/createrequestdialog.ui
    browser/searchwidget.ui
    mainwindow.ui
    configure.ui
    login.ui
    roweditor.ui
    requeststateeditor.ui
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
