.SUFFIXES:

SHELL:=/bin/bash
PREFIX?=/usr
DESTDIR?=

TARGET   = fft-arm
NMPP     = /home/mushkaev/git/nmpp

CC       = arm-module-linux-gnueabi-gcc 

INC_DIRS+= -I./aura/include
INC_DIRS+= -I./nmc-utils/include/ -DLIBEASYNMC_VERSION=\"0.1.1\"
INC_DIRS+= -I/home/mushkaev/matlab/linux-3.10.x/include/uapi
INC_DIRS+= -I/home/mushkaev/matlab/linux-3.10.x/drivers/staging/android/uapi
INC_DIRS+= -I$(NMPP)/include

SRC_DIRS = . ./aura ./nmc-utils 
VPATH    = $(SRC_DIRS)

CC_FLAGS =-Iinclude/ -g -Wall -fPIC $(INC_DIRS)
LD_FLAGS+=-rdynamic -g -L$(NMPP)/lib  -lelf

ALL_C    = $(wildcard $(addsuffix /*.c,$(SRC_DIRS)))
OBJECTS_C= $(notdir $(patsubst %.c,%.o,$(ALL_C)))


all : $(TARGET)
	$(MAKE) -C ../nmc	
	scp ../nmc/fft-nmc.abs root@192.168.20.9:
	scp ./$(TARGET) root@192.168.20.9:

%.o: %.c 
	$(CC) $(CC_FLAGS) -c $(<) -o$(@) $(INC_DIRS) 


$(TARGET): $(OBJECTS_C)
	$(CC) -o $(@) $(OBJECTS_C) $(LD_FLAGS)


clean:
	-rm *.o $(TARGET)
	-cd nmc-utils && make clean

	
