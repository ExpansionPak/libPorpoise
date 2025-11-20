###############################################################################
# Makefile for paddemo
#
# Copyright 1998-2001 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: /Dolphin/build/demos/paddemo/makefile $
#   
#   8     9/03/01 19:41 Shiki
#   Added sampling.
#   
#   7     01/03/15 13:25 Shiki
#   Fixed BINNAMES.
#   
#   6     01/03/13 13:04 Shiki
#   Fixed not to create cont demo with Mac build.
#   
#   5     11/29/00 4:37p Shiki
#   Fixed not to create motor demo with Mac build.
#   
#   4     11/29/00 4:27p Shiki
#   Added motor.
#   
#   3     11/13/00 6:03p Shiki
#   Added cont.
#   
#   2     3/01/00 11:49p Shiki
#   Added simple and basic. Removed main.
#   
#   6     11/03/99 6:10p Shiki
#   Revised to support multiple platforms.
# $NoKeywords: $
#
###############################################################################

##################################
# QUICK START INSTRUCTIONS
# Type "make" at /dolphin/build/demos/paddemo to build DEBUG versions 
# of all demos
# Type "make NDEBUG=TRUE" to build OPTIMIZED versions of all tests
# Type "make dvddemo1D.bin" to build DEBUG version of just dvddemo1
# Type "make NDEBUG=TRUE dvddemo1.bin" to build OPTIMIZED version of 
# just dvddemo1
#
# To add another demo
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
MODULENAME	= paddemo
DEMO		= TRUE

# CSRCS lists all C files that should be built
# The makefile determines which objects are linked into which binaries
# based on the dependencies you fill in at the bottom of this file

CSRCS		= simple.c basic.c

ifdef EPPC
# defining a linker command file will have the build system generate it
# automatically and include it on the linker invocation line
LCF_FILE	= $(INC_ROOT)/dolphin/eppc.$(ARCH_TARGET).lcf
endif

# BINNAMES lists all binaries that will be linked.  Note that no suffix is
# required, as that will depend on whether this is a DEBUG build or not.
# The final name of the binaries will be $(BINNAME)$(BINSUFFIX)
BINNAMES 	= simple basic

ifdef EPPC
CSRCS		+= cont.c contdemo.c motordemo.c sampling.c
BINNAMES 	+= cont motor sampling
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
$(FULLBIN_ROOT)/simple$(BINSUFFIX): simple.o \
		      $(DOLPHINLIBS)

$(FULLBIN_ROOT)/basic$(BINSUFFIX): basic.o \
		      $(DOLPHINLIBS)

$(FULLBIN_ROOT)/cont$(BINSUFFIX): cont.o contdemo.o \
		      $(DOLPHINLIBS)

$(FULLBIN_ROOT)/motor$(BINSUFFIX): cont.o motordemo.o \
		      $(DOLPHINLIBS)

$(FULLBIN_ROOT)/sampling$(BINSUFFIX): sampling.o \
		      $(DOLPHINLIBS)
