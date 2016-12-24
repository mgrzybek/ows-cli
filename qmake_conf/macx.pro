# Project: ows-cli
# File name: macx.pro
# Description: describes the project and how to build it
#
# @author Mathieu Grzybek on 2013-06-06
# @copyright 20?? Mathieu Grzybek. All rights reserved.
# @version $Id: code-gpl-license.txt,v 1.2 2004/05/04 13:19:30 garry Exp $
#
# @see The GNU Public License (GPL) version 3 or higher
#
#
# ows-cli is free software; you can redistribute it and/or modify
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

macx {
#	PKGCONFIG += thrift libzmq

	LIBS += -lthrift \
		-L/opt/local/lib \
		-L/usr/lib \
		-L/usr/local/lib \
		/opt/local/lib/libboost_regex-mt.a \
		/opt/local/lib/libboost_program_options-mt.a \
		/opt/local/lib/libboost_system-mt.a \
		-lssl \
		-lpthread \
		-lcrypto \
		-lm \
		-ldl \
		-lz \
		-lreadline

	INCLUDEPATH += /usr/include \
		/opt/local/include \
		/opt/local/include/thrift \
		src
}

