#
# This is a basic Makefile template for a Neuromatrix DSP project 
# to be run on Module MB77.07. To compile it you need: 
# * Latest NMSDK installed with utilities in your $PATH
# * NEURO environment variable pointing to NMSDK directory
# * Host GCC (Since nmcpp doesn't support generating deps, 
#	       we use $(CROSS_COMPILE)gcc for that) 
#
# For verbose build run 'make VERBOSE=y'
#

-include colorizer.mk
-include *.dep

.SUFFIXES:

OBJECTS :=  \
	main.o \
	easyconf.o \
	aura-exports.o

TARGET=fft-nmc
NMPP  = /home/mushkaev/git/nmpp

# Set this to libeasynmc-nmc dir. Relative or absolute. 
# Make sure you build it prior to building the actual project. 
EASYNMC_DIR     = /home/mushkaev/matlab/aura/nmc-utils/libeasynmc-nmc

CROSS_COMPILE   =
NMCPP_FLAGS     = -DNEURO -OPT2 -inline -I$(EASYNMC_DIR)/include
ASM_FLAGS       = -soc -Sc -Stmp -X-q -I$(EASYNMC_DIR)/include
C2ASM_FLAGS     = -soc -q 

#BIG FAT WARNING: easynmc.lib MUST go BEFORE libc
#BIG FAT WARNING: Otherwise argc/argv won't work 

LIBS            = easynmc.lib libc05.lib nmpp_nmc3.lib libint_soc.lib

BUILDER_FLAGS	= -cK1879.cfg -m -heap=4000 -heap1=26384 -heap2=26384 -heap3=26384 -stack=2000 \
		  -full_names
IDIRS           = -I. -I"$(NEURO)/include" -I"$(NMPP)/include"
LIBDIR		= -l"$(NEURO)/lib" -l"$(EASYNMC_DIR)" -l$(NMPP)/lib

.DEFAULT_GOAL=all

all: $(TARGET).abs

%.asmx: %.cpp
	$(SILENT_DEP)$(CROSS_COMPILE)gcc -E -MM $(<) -o$(@).dep $(IDIRS) 
	$(SILENT_NMCPP)nmcpp -Tp $(NMCPP_FLAGS) $(<) -O$(@) $(IDIRS) 

%.asmx: %.c
	$(SILENT_DEP)$(CROSS_COMPILE)gcc -E -MM $(<) -o$(@).dep $(IDIRS) 
	$(SILENT_NMCPP)nmcpp -Tc99 $(NMCPP_FLAGS) $(<) -O$(@) $(IDIRS) 

%.o: %.asmx
	$(SILENT_ASM)asm $(C2ASM_FLAGS) $(<) -o$(@)

%.o: %.asm
	$(SILENT_DEP)$(CROSS_COMPILE)gcc -E -MM -xassembler-with-cpp $(<) -o$(@).dep
	$(SILENT_ASM)nmcc $(ASM_FLAGS) $(<) -o$(@)

$(TARGET).lib: $(OBJECTS)
	-$(SILENT_LIBRARIAN)libr -c $(@) $(^) > /dev/null

$(TARGET).abs: $(OBJECTS)
	-$(SILENT_LINKER)linker  $(BUILDER_FLAGS) -o$(@) $(^) $(LIBS) $(LIBDIR)

$(TARGET).dump: $(TARGET).abs
	-$(SILENT_NMDUMP)nmdump -f $(^) > $(@)

run: $(TARGET).abs
	edcltool -f run_nmc_code.edcl -i eth1

clean:
	-$(SILENT_CLEAN)rm -f *.asmx; rm -f *.o; rm -f $(TARGET).abs $(TARGET).dump *.dep \
	*.ac *.map *~ *.abs *.lib
