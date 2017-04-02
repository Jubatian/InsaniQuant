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
OBJECTS+=$(OBD)depthred.o
OBJECTS+=$(OBD)coldiff.o
OBJECTS+=$(OBD)coldepth.o
OBJECTS+=$(OBD)mquant.o
OBJECTS+=$(OBD)palapp.o
OBJECTS+=$(OBD)idata.o
OBJECTS+=$(OBD)palgen.o
OBJECTS+=$(OBD)uzebox.o


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

$(OBD)depthred.o: depthred.c *.h
	$(CC) -c depthred.c -o $(OBD)depthred.o $(CFSIZ)

$(OBD)coldiff.o: coldiff.c *.h
	$(CC) -c coldiff.c -o $(OBD)coldiff.o $(CFSIZ)

$(OBD)coldepth.o: coldepth.c *.h
	$(CC) -c coldepth.c -o $(OBD)coldepth.o $(CFSIZ)

$(OBD)mquant.o: mquant.c *.h
	$(CC) -c mquant.c -o $(OBD)mquant.o $(CFSIZ)

$(OBD)palapp.o: palapp.c *.h
	$(CC) -c palapp.c -o $(OBD)palapp.o $(CFSIZ)

$(OBD)idata.o: idata.c *.h
	$(CC) -c idata.c -o $(OBD)idata.o $(CFSIZ)

$(OBD)palgen.o: palgen.c *.h
	$(CC) -c palgen.c -o $(OBD)palgen.o $(CFSIZ)

$(OBD)uzebox.o: uzebox.c *.h
	$(CC) -c uzebox.c -o $(OBD)uzebox.o $(CFSIZ)


.PHONY: all clean
