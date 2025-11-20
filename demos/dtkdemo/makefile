###############################################################################
# Makefile for dtkdemo
#
# Copyright 2001 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
#  $Log: /Dolphin/build/demos/dtkdemo/makefile $
#   
#   1     5/10/01 5:03a Eugene
#   
###############################################################################

all: setup build
##################################
# QUICK START INSTRUCTIONS
# Type "make" at /dolphin/build/tests/audio to build DEBUG versions of all tests.
# Type "make NDEBUG=TRUE" to build OPTIMIZED versions of all tests
# Type "make lit-test00D.bin" to build DEBUG version of just lit-test00
# Type "make NDEBUG=TRUE lit-test00.bin" to build OPTIMIZED version of 
# ju ctable paths can be found in modulerules.
# just lit-test00
#
# To add another test
# 1. add the .c files to CSRCS to make sure they are built
# 2. add the binary name (no suffix) to BINNAMES
# 3. add a dependency rule for this executable at the bottom of this file
##################################


# All modules have "setup" and "build" as targets.  System libraries 
# and demo programs also have an "install" target that copies the compiled
# binaries to the binary tree (/dolphin/$(ARCH_TARGET)).
all: 	setup build install

# commondefs must be included near the top so that all common variables
# will be defined before their use.
include $(BUILDTOOLS_ROOT)/commondefs

# module name should be set to the name of this subdirectory
# DEMO = TRUE indicates that this module resides under the "demos" subtree.
# The list of selectable paths can be found in modulerules.
MODULENAME	= dtkdemo
DEMO		= TRUE

# CSRCS lists all C files that should be built
# The makefile determines which objects are linked into which binaries
# based on the dependencies you fill in at the bottom of this file
CSRCS		=	main.c ui_stuff.c dtk_stuff.c

# BINNAMES lists all binaries that will be linked.  Note that no suffix is
# required, as that will depend on whether this is a DEBUG build or not.
# The final name of the binaries will be $(BINNAME)$(BINSUFFIX)
BINNAMES 	= dtkdemo

LCF_FILE = $(INC_ROOT)/dolphin/eppc.$(ARCH_TARGET).lcf

# modulerules contains the rules that will use the above variables 
# and dependencies below to construct the binaries specified.
include $(BUILDTOOLS_ROOT)/modulerules

# Dependencies for the binaries listed in BINNAMES should come here
# They are your typical makefile rule, with extra variables to ensure
# that the names and paths match up.
# $(FULLBIN_ROOT) is the location of the local bin directory
# $(BINSUFFIX) depends on whether this is a debug build or not
# $(DOLPHINLIBS) includes all the Dolphin libraries.


$(FULLBIN_ROOT)/dtkdemo$(BINSUFFIX):	main.o	\
										ui_stuff.o \
										dtk_stuff.o \
										$(DOLPHINLIBS) 
#======== End of makefile ========#
