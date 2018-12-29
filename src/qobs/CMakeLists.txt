# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Gui Network REQUIRED)

configure_file(libqobs.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libqobs.pc @ONLY)

set(LIBQOBS_SRC
    obscore.cpp
    obsxmlreader.cpp
    obsrequest.cpp
    obs.cpp
    obsfile.cpp
    obsresult.cpp
    obsrevision.cpp
    obsstatus.cpp
    obsabout.cpp
    obsxmlwriter.cpp
    obsobject.cpp
    obslink.cpp)

set(LIBQOBS_HDR
    obscore.h
    obsxmlreader.h
    obsrequest.h
    obs.h
    obsfile.h
    obsresult.h
    obsrevision.h
    obsstatus.h
    obsabout.h
    obsxmlwriter.h
    obsobject.h
    obslink.h)

add_library(libqobs SHARED ${LIBQOBS_SRC})

target_compile_definitions(libqobs PRIVATE -DQOBS_LIBRARY)

target_link_libraries(libqobs Qt5::Core Qt5::Gui Qt5::Network)

set_target_properties(libqobs PROPERTIES OUTPUT_NAME "qobs"
                              VERSION ${PROJECT_VERSION}
                              SOVERSION ${PROJECT_VERSION_MAJOR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libqobs.pc
        DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig)
install(FILES ${LIBQOBS_HDR} DESTINATION include/qobs)
install(TARGETS libqobs LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

