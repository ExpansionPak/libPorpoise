###############################################################################
# Relocatable module demo
#
# Copyright 2001 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: /Dolphin/build/demos/reldemo/makefile $
#   
#   12    11/27/01 9:25 Shiki
#   Modified to link global_destructor_chain.c from MSL.
#   
#   11    5/21/01 9:45a Shiki
#   Modified to evaluate static$(STRSUFFIX) target after setup.
#   
#   10    5/14/01 3:13p Shiki
#   Defined PREREL_LCF_FILE.
#   
#   9     01/05/09 15:48 Shiki
#   Revised.
#   
#   8     01/04/02 15:00 Shiki
#   Added an echo message for ignorable warnings.
#   
#   7     01/04/02 14:44 Shiki
#   Initial check-in.
# $NoKeywords: $
#
###############################################################################

##################################
# QUICK START INSTRUCTIONS
# Type "make" at /dolphin/build/demos/reldemo to build DEBUG versions.
# Type "make NDEBUG=TRUE" to build OPTIMIZED versions.
#
# Copy bin/$(PLATFORM)/*.str and bin/$(PLATFORM)/*.rel to the emulation 
# drive root.
##################################

# commondefs must be included near the top so that all common variables
# will be defined before their use.
include $(BUILDTOOLS_ROOT)/commondefs

# build any code modules and module name string table
all:    setup build install

# module name should be set to the name of this subdirectory
# DEMO = TRUE indicates that this module resides under the "demos" subtree.
# The list of selectable paths can be found in modulerules.
MODULENAME  = reldemo
DEMO        = TRUE

# small data sections are not supported by relocatable modules
CCFLAGS += -sdata 0 -sdata2 0

ifdef MAC

else
# EPPC

# CSRCS lists all C files that should be built
# The makefile determines which objects are linked into which binaries
# based on the dependencies you fill in at the bottom of this file
CSRCS = static.c

# CPPSRCS lists all C++ files that should be built
# The makefile determines which objects are linked into which binaries
# based on the dependencies you fill in at the bottom of this file
CPPSRCS = a.cpp b.cpp

# BINNAMES specifies the static portion of the program.  Note that no suffix
# is required, as that will depend on whether this is a DEBUG build or not.
# The final name of the binaries will be $(STATIC)$(BINSUFFIX)
BINNAMES    = static

# RELNAMES lists all relocatable modules that will be linked.
# Note that no suffix is required, as that will depend on whether
# this is a DEBUG build or not.
# The final name of the modules will be $(RELNAMES)$(RELSUFFIX)
RELNAMES    = a b

# linker command file for building the static module
LCF_FILE        = static$(LCFSUFFIX)

# linker command file for building relocatable modules (for 1st step)
PREREL_LCF_FILE = $(INC_ROOT)/dolphin/eppc.lcf

# linker command file for building relocatable modules (for 2nd step)
REL_LCF_FILE    = partial$(LCFSUFFIX)

# filename of the force active symbol name list
LST_FILE        = active$(LSTSUFFIX)

endif

# modulerules contains the rules that will use the above variables 
# and dependencies below to construct the binaries specified.
include $(BUILDTOOLS_ROOT)/modulerules

# all build targets that depend on 'setup' target must be listed as
# prerequisites  for 'dobuild'.
dobuild: static$(STRSUFFIX)

# dependencies and rules for any added rules may be placed here to take
# advantage of commondefs.
# $(FULLBIN_ROOT) is the location of the local bin directory
# $(BINSUFFIX) depends on whether this is a debug build or not
# $(DOLPHINLIBS) includes all the Dolphin libraries.

# generate the module name string table. *.rel files are also generated.
static$(STRSUFFIX): $(FULLBIN_ROOT)/static$(BINSUFFIX) $(TARGET_PLFS)
	@echo 
	@echo ">> $(notdir $+) --> $@"
	"$(ROOT)/X86/bin/makerelD.exe" $+

# $(PREPLFSUFFIX) are for the 1st .plf file build without .lcf file
# $(PLFSUFFIX)    are for the 2nd .plf file build with .lcf file

$(FULLBIN_ROOT)/a$(PREPLFSUFFIX): a.o \
	$(BINOBJ_ROOT)/global_destructor_chain.o

$(FULLBIN_ROOT)/b$(PREPLFSUFFIX): b.o \
	$(BINOBJ_ROOT)/global_destructor_chain.o

$(FULLBIN_ROOT)/static$(BINSUFFIX): static.o $(DOLPHINLIBS) $(LCF_FILE)

$(FULLBIN_ROOT)/a$(PLFSUFFIX): a.o \
	$(BINOBJ_ROOT)/global_destructor_chain.o $(REL_LCF_FILE)

$(FULLBIN_ROOT)/b$(PLFSUFFIX): b.o \
	$(BINOBJ_ROOT)/global_destructor_chain.o $(REL_LCF_FILE)

# generate force active symbol list.
$(LST_FILE): $(TARGET_PREPLFS)
	@echo 
	@echo ">> $(notdir $+) --> $@"
	"$(ROOT)/X86/bin/makerelD.exe" $(TARGET_PREPLFS)
	cat import.lst | sort | uniq > $(LST_FILE)

# generate linker command file for the static module.
$(LCF_FILE): $(LST_FILE)
	@echo 
	@echo ">> $+ --> $@"
	cat $(INC_ROOT)/dolphin/eppc.$(ARCH_TARGET).lcf > $@
	echo "FORCEACTIVE { " >> $@
	cat $(LST_FILE) >> $@
	echo "    OSLink" >> $@
	echo "    OSUnlink" >> $@
	echo "}" >> $@

# generate linker command file for relocatable modules.
$(REL_LCF_FILE): $(LST_FILE)
	@echo 
	@echo ">> $+ --> $@"
	cat $(PREREL_LCF_FILE) > $@
	echo "FORCEACTIVE { " >> $@
	echo "    _prolog" >> $@
	echo "    _epilog" >> $@
	echo "    _unresolved" >> $@
	cat $(LST_FILE) >> $@
	echo "}" >> $@

clean:
	rm -f *.lst
	rm -f *.lcf

$(BINOBJ_ROOT)/global_destructor_chain.o:
	$(CC) $(CCFLAGS) $(INCLUDES) \
        $(COMPILE) $(MWDIR)/PowerPC_EABI_Support/Runtime/Src/global_destructor_chain.c \
        -o $@
