# Project: ows-qt-console
# File name: ows-qt-console.pro
# Description: describes the project and how to build it
#
# @author Mathieu Grzybek on 20??-??-??
# @copyright 20?? Mathieu Grzybek. All rights reserved.
# @version $Id: code-gpl-license.txt,v 1.2 2004/05/04 13:19:30 garry Exp $
#
# @see The GNU Public License (GPL) version 3 or higher
#
#
# ? is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXX_FLAGS	+= -O2
QMAKE_C_FLAGS	+= -O2

include(qmake_conf/linux.pro)
include(qmake_conf/macx.pro)
include(qmake_conf/bsd.pro)
#include(qmake_conf/windows.pro)

INCLUDEPATH	+= include \
	../open-workload-scheduler/include \
	../open-workload-scheduler/src/gen-cpp

SOURCES		+= src/main.cpp \
	src/printing.cpp \
	src/text_processing.cpp \
	src/libcli.c \
	../open-workload-scheduler/src/gen-cpp/ows_rpc.cpp \
	../open-workload-scheduler/src/gen-cpp/model_types.cpp \
	../open-workload-scheduler/src/gen-cpp/model_constants.cpp \
	../open-workload-scheduler/src/rpc_client.cpp \
	../open-workload-scheduler/src/convertions.cpp

HEADERS		+= include/main.h \
	include/printing.h \
	include/text_processing.h \
	include/libcli.h \
	../open-workload-scheduler/src/gen-cpp/ows_rpc.h \
	../open-workload-scheduler/src/gen-cpp/model_types.h \
	../open-workload-scheduler/src/gen-cpp/model_constants.h \
	../open-workload-scheduler/include/rpc_client.h \
	../open-workload-scheduler/include/convertions.h
