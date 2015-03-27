############
# Makefile #
############
#
# Author    Sandor Zsuga (Jubatian)
# Copyright 2013 - 2015, GNU General Public License version 2 or any later
#           version, see LICENSE
#
#
#
#
# The main makefile of the program
#
#
# make all (or make): build the program
# make clean:         to clean up
#
#

include Make_defines.mk

CFLAGS+=

OBJECTS= $(OBD)main.o
OBJECTS+=$(OBD)colcount.o
OBJECTS+=$(OBD)depthred.o
OBJECTS+=$(OBD)coldiff.o
OBJECTS+=$(OBD)iquant.o
OBJECTS+=$(OBD)fquant.o
OBJECTS+=$(OBD)dither.o


all: $(OUT)
clean:
	$(SHRM) $(OBJECTS) $(OUT)
	$(SHRM) $(OBB)


$(OUT): $(OBB) $(OBJECTS)
	$(CC) -o $(OUT) $(OBJECTS) $(CFSIZ) $(LINK)

$(OBB):
	$(SHMKDIR) $(OBB)

$(OBD)main.o: main.c *.h
	$(CC) -c main.c -o $(OBD)main.o $(CFSIZ)

$(OBD)colcount.o: colcount.c *.h
	$(CC) -c colcount.c -o $(OBD)colcount.o $(CFSIZ)

$(OBD)depthred.o: depthred.c *.h
	$(CC) -c depthred.c -o $(OBD)depthred.o $(CFSIZ)

$(OBD)coldiff.o: coldiff.c *.h
	$(CC) -c coldiff.c -o $(OBD)coldiff.o $(CFSIZ)

$(OBD)iquant.o: iquant.c *.h
	$(CC) -c iquant.c -o $(OBD)iquant.o $(CFSIZ)

$(OBD)fquant.o: fquant.c *.h
	$(CC) -c fquant.c -o $(OBD)fquant.o $(CFSIZ)

$(OBD)dither.o: dither.c *.h
	$(CC) -c dither.c -o $(OBD)dither.o $(CFSIZ)


.PHONY: all clean
