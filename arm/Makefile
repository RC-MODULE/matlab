.SUFFIXES:
ifeq ($(OS),Windows_NT)

SHELL = cmd
AURA  = d:/git/aura
CC    = d:/SDK/arm-module-linux-gnueabi/bin/arm-module-linux-gnueabi-gcc.exe
else
SHELL =/bin/bash
CC    = arm-module-linux-gnueabi-gcc 
endif

PREFIX?=/usr
DESTDIR?=

TARGET   = fft-arm
AURA     = ../aura


INC_DIRS+= -I$(NMPP)/include
INC_DIRS+= -I$(AURA)/include -I$(AURA)/nmc-utils/include
INC_DIRS+= -DLIBEASYNMC_VERSION=\"0.1.1\"
#INC_DIRS+= -I/home/mushkaev/matlab/linux-3.10.x/include/uapi
#INC_DIRS+= -I/home/mushkaev/matlab/linux-3.10.x/drivers/staging/android/uapi
#INC_DIRS+= -I/include/uapi
#INC_DIRS+= -I/drivers/staging/android/uapi
INC_DIRS+= -I./include  -I./include/linux


CC_FLAGS = -g -Wall -fPIC $(INC_DIRS)
LD_FLAGS+=-rdynamic -g  -lelf

#-L$(NMPP)/lib
#INC_DIRS+= -I$(NMPP)/include
#SRC_DIRS = . ./aura 
#ALL_C    = $(wildcard $(addsuffix /*.c,$(SRC_DIRS)))


ALL_C   = ./fft-arm.c 
ALL_C   += $(AURA)/aura.c 
ALL_C   += $(AURA)/buffer.c 
ALL_C   += $(AURA)/buffer-dummy.c 
ALL_C   += $(AURA)/eventloop.c 
ALL_C   += $(AURA)/eventsys-epoll.c 
ALL_C   += $(AURA)/export.c 
ALL_C   += $(AURA)/ion.c 
ALL_C   += $(AURA)/panic.c 
ALL_C   += $(AURA)/queue.c 
ALL_C   += $(AURA)/retparse.c 
ALL_C   += $(AURA)/serdes.c 
ALL_C   += $(AURA)/slog.c 
ALL_C   += $(AURA)/transport.c 
ALL_C   += $(AURA)/transport-dummy.c 
ALL_C   += $(AURA)/transport-nmc.c 
ALL_C   += $(AURA)/utils.c 
ALL_C   += $(AURA)/utils-linux.c 
ALL_C   += $(AURA)/nmc-utils/easynmc-core.c
ALL_C   += $(AURA)/nmc-utils/easynmc-filters.c


OBJECTS_C= $(notdir $(patsubst %.c,%.o,$(ALL_C)))
#VPATH    = $(AURA) $(AURA)/nmc-utils

all : $(TARGET)
	$(MAKE) -C ../nmc	
	
upload:	
	scp ../nmc/fft-nmc.abs root@192.168.20.9:
	scp ./$(TARGET) root@192.168.20.9:

%.o: %.c 
	$(CC) $(CC_FLAGS) -c $(<) -o$(@) $(INC_DIRS) 

%.o: $(AURA)/%.c 
	$(CC) $(CC_FLAGS) -c $(<) -o$(@) $(INC_DIRS) 

%.o: $(AURA)/nmc-utils/%.c 
	$(CC) $(CC_FLAGS) -c $(<) -o$(@) $(INC_DIRS) 
	
	

$(TARGET): $(OBJECTS_C)
	$(CC) -o $(@) $(OBJECTS_C) $(LD_FLAGS)


clean:
	-rm *.o $(TARGET)
	-cd nmc-utils && make clean
	$(MAKE) -C ../nmc clean

	
