##
## src/Makefile.am
##
## Copyright (C) 2016 by
## Jeffrey Fulmer - <jeff@joedog.org>, et al.
## This file is distributed as part of ajping
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 2 of
## the License, or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
##

bin_PROGRAMS       =   ajping

WARN_CFLAGS        =   @WARN_CFLAGS@

AM_CFLAGS          =   $(WARN_CFLAGS) 

ajping_SOURCES     =   \
ajp.c     ajp.h        \
getopt.c  getopt1.c    \
main.c    setup.h      \
memory.c  memory.h     \
notify.c  notify.h     \
perl.c    perl.h       \
sock.c    sock.h       \
url.c     url.h        \
util.c    util.h       \
version.c version.h

AUTOMAKE_OPTIONS   =   foreign no-dependencies                   
 
