###############################################################################
# Makefile for GD demos
#
# Copyright 2001 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
#  $Log: /Dolphin/build/demos/gddemo/makefile $
#   
#   6     11/26/01 7:56p Hirose
#   Added host side build path.
#   
#   5     10/04/01 2:53p Hirose
#   Added multitexturing (TEV) demo.
#   
#   4     9/25/01 6:25p Carl
#   Added init demo.
#   
#   3     9/24/01 2:27p Hirose
#   Added light demos. Changes due to file naming structure changes.
#   
#   2     9/19/01 5:52p Carl
#   Added matrix demo.
#
#   1     9/19/01 4:26p Carl
#   Makefile for gd demos.
#
#  $NoKeywords: $
#
###############################################################################

# All modules have "setup" and "build" as targets.  System libraries 
# and demo programs also have an "install" target that copies the compiled
# binaries to the binary tree (/dolphin/$(ARCH_TARGET)).
# tests are NOT installed into the binary release directory.

ifdef X86
#build host library
all:	buildhost
clean:	cleanhost
else
# build and install library for GC hardware
all:	setup build install
endif

ifdef X86
buildhost:
	@echo =========== Build host demos ==============
	@./buildhostscript
	@echo ===========================================

cleanhost:
	@echo =========== Clean up host demos ===========
	@./buildhostscript /CLEAN
	@echo ===========================================
endif

# module name should be set to the name of this subdirectory
# DEMO = TRUE indicates that this module resides under the "demos" subtree.
# The list of selectable paths can be found in modulerules.
MODULENAME	= gddemo
DEMO		= TRUE

# commondefs must be included near the top so that all common variables
# will be defined before their use.
include $(BUILDTOOLS_ROOT)/commondefs

# common sources built for both EPPC and MAC
# CSRCS lists all C files that should be built
# The makefile determines which objects are linked into which binaries
# based on the dependencies you fill in at the bottom of this file
CSRCS = \
	gd-texture-gc.c \
	gd-texture-gc-load.c \
	gd-texture-create.c \
	\
	gd-matrix-gc.c \
	gd-matrix-gc-load.c \
	gd-matrix-create.c \
	\
	gd-light-gc.c \
	gd-light-gc-load.c \
	gd-light-create.c \
	\
	gd-tev-gc.c \
	gd-tev-gc-load.c \
	gd-tev-create.c \
	\
	gd-init-gc.c \
	gd-init-gc-load.c \
	gd-init-create.c

# common binaries linked for both EPPC and MAC
# BINNAMES lists all binaries that will be linked.  Note that no suffix is
# required, as that will depend on whether this is a DEBUG build or not.
# The final name of the binaries will be $(BINNAME)$(BINSUFFIX)
BINNAMES = \
	gd-texture-gc-load \
	gd-texture-gc-create \
	gd-matrix-gc-load \
	gd-matrix-gc-create \
	gd-light-gc-load \
	gd-light-gc-create \
	gd-tev-gc-load \
	gd-tev-gc-create \
	gd-init-gc-load \
	gd-init-gc-create

ifdef EPPC
LCF_FILE =  $(INC_ROOT)/dolphin/eppc.$(ARCH_TARGET).lcf
endif

ifdef WALL
CCFLAGS +=  -w all
endif

# modulerules contains the rules that will use the above variables 
# and dependencies below to construct the binaries specified.
include $(BUILDTOOLS_ROOT)/modulerules

# Dependencies for the binaries listed in BINNAMES should come here
# They are your typical makefile rule, with extra variables to ensure
# that the names and paths match up.
# $(FULLBIN_ROOT) is the location of the local bin directory
# $(BINSUFFIX) depends on whether this is a debug build or not
# $(DOLPHINLIBS) includes all the Dolphin libraries.
$(FULLBIN_ROOT)/gd-texture-gc-load$(BINSUFFIX): \
	gd-texture-gc-load.o \
	$(DOLPHINLIBS) $(DOL_INSTALL_ROOT)/lib/gd$(LIBSUFFIX)

$(FULLBIN_ROOT)/gd-texture-gc-create$(BINSUFFIX): \
	gd-texture-gc.o gd-texture-create.o \
	$(DOLPHINLIBS) $(DOL_INSTALL_ROOT)/lib/gd$(LIBSUFFIX)

$(FULLBIN_ROOT)/gd-matrix-gc-load$(BINSUFFIX): \
	gd-matrix-gc-load.o \
	$(DOLPHINLIBS) $(DOL_INSTALL_ROOT)/lib/gd$(LIBSUFFIX)

$(FULLBIN_ROOT)/gd-matrix-gc-create$(BINSUFFIX): \
	gd-matrix-gc.o gd-matrix-create.o \
	$(DOLPHINLIBS) $(DOL_INSTALL_ROOT)/lib/gd$(LIBSUFFIX)

$(FULLBIN_ROOT)/gd-light-gc-load$(BINSUFFIX): \
	gd-light-gc-load.o \
	$(DOLPHINLIBS) $(DOL_INSTALL_ROOT)/lib/gd$(LIBSUFFIX)

$(FULLBIN_ROOT)/gd-light-gc-create$(BINSUFFIX): \
	gd-light-gc.o gd-light-create.o \
	$(DOLPHINLIBS) $(DOL_INSTALL_ROOT)/lib/gd$(LIBSUFFIX)

$(FULLBIN_ROOT)/gd-tev-gc-load$(BINSUFFIX): \
	gd-tev-gc-load.o \
	$(DOLPHINLIBS) $(DOL_INSTALL_ROOT)/lib/gd$(LIBSUFFIX)

$(FULLBIN_ROOT)/gd-tev-gc-create$(BINSUFFIX): \
	gd-tev-gc.o gd-tev-create.o \
	$(DOLPHINLIBS) $(DOL_INSTALL_ROOT)/lib/gd$(LIBSUFFIX)

$(FULLBIN_ROOT)/gd-init-gc-load$(BINSUFFIX): \
	gd-init-gc-load.o \
	$(DOLPHINLIBS) $(DOL_INSTALL_ROOT)/lib/gd$(LIBSUFFIX)

$(FULLBIN_ROOT)/gd-init-gc-create$(BINSUFFIX): \
	gd-init-gc.o gd-init-create.o \
	$(DOLPHINLIBS) $(DOL_INSTALL_ROOT)/lib/gd$(LIBSUFFIX)

#======== End of makefile ========#
