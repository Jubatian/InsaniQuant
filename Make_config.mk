############################
# Makefile - configuration #
############################
#
# Author    Sandor Zsuga (Jubatian)
# Copyright 2013 - 2015, GNU General Public License version 2 or any later
#           version, see LICENSE
#
#
#
#
# Alter this file according to your system to build the thing
#
#
#
# Target operating system. This will define how the process will go
# according to the os's features. Currently supported:
#  linux
#  windows_mingw
#
TSYS=linux
#
#
# If you wish to change your output executable's name, do it here.
#
OUT=insaniquant
#
#
# A few paths in case they would be necessary. Leave them alone unless
# it is necessary to modify.
#
CC_BIN=
CC_INC=
CC_LIB=
#
#
# The compiler: should be gcc, but in case you need to alter...
#
CC=gcc
#
#
# In case a test build (debug) is necessary, give 'test' here. It enables
# extra assertions, and compiles the program with no optimizations, debug
# symbols enabled.
#
GO=
