# -------------------------------------------------
# Project created by QtCreator 2010-12-22T04:00:42
# -------------------------------------------------

TEMPLATE = subdirs

src_lib.subdir = src/qobs
src_gui.subdir = src/gui
src_gui.depends = src_lib

SUBDIRS += src_lib src_gui
