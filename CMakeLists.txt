cmake_minimum_required(VERSION 3.16)

project(Qactus
   VERSION 3.0.0
   LANGUAGES CXX)

add_definitions(-DQACTUS_VERSION="${PROJECT_VERSION}")
add_definitions(-DAPP_NAME="${PROJECT_NAME}")
add_definitions(-DORG_NAME="${PROJECT_NAME}")

include(GNUInstallDirs)

add_subdirectory(src/qobs)
add_subdirectory(src/gui)

