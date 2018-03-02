
DEBUG = 0
CC = g++
TARGET = main.exe
ifeq ($(DEBUG),0)
DIR_BUILD = release
CFLAGS =  -Wall
else
DIR_BUILD = debug
CFLAGS =  -Wall -g
endif
DIR_OBJ = $(DIR_BUILD)/obj
DIR_BIN = $(DIR_BUILD)/bin
DIR_IND = $(DIR_BUILD)/ind
BIN_TARGET = ${DIR_BIN}/${TARGET}
SRC := 
OBJ := 
SRC += ./main.cpp
OBJ += ${DIR_OBJ}/main.o

all:|createdir ${BIN_TARGET}

createdir:
	@if [ ! -d $(DIR_OBJ) ]; then\
		mkdir -p $(DIR_OBJ) ;\
	fi
	@if [ ! -d $(DIR_BIN) ]; then\
		mkdir -p $(DIR_BIN) ;\
	fi
	@if [ ! -d $(DIR_IND) ]; then\
		mkdir -p $(DIR_IND) ;\
	fi

${BIN_TARGET}:${OBJ}
	$(CC) ${OBJ} -o $@

${DIR_OBJ}/main.o: main.cpp
	$(CC) $(CFLAGS) -c ./main.cpp -o $@

.PHONY:clean
clean:
	-del ${OBJ} ${BIN_TARGET}
	-rm -rf ${OBJ} ${BIN_TARGET}
