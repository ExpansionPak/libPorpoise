###############################################################################
# Makefile for DVD Overview demos
#
# Copyright 1998, 1999 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: /Dolphin/build/demos/dvddemo/makefile $
#   
#   5     6/27/01 3:12p Carl
#   Removed "errorhandling" from Mac build.
#   
#   4     6/15/01 9:02p Hashida
#   Added error handling demo.
#   
#   3     3/28/00 3:41p Hashida
#   Added sample "directory" to MAC build.
#   
#   2     3/02/00 12:43p Hashida
#   Added directory access sample.
#   
#   7     11/03/99 6:14p Shiki
#   Revised to support multiple platforms.
# $NoKeywords: $
#
###############################################################################

##################################
# QUICK START INSTRUCTIONS
# Type "make" at /dolphin/build/demos/dvddemo to build DEBUG versions 
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
MODULENAME	= dvddemo
DEMO		= TRUE

ifdef EPPC
# defining a linker command file will have the build system generate it
# automatically and include it on the linker invocation line
LCF_FILE	= $(INC_ROOT)/dolphin/eppc.$(ARCH_TARGET).lcf
endif

# CSRCS lists all C files that should be built
# The makefile determines which objects are linked into which binaries
# based on the dependencies you fill in at the bottom of this file
CSRCS		= dvddemo1.c dvddemo2.c dvddemo3.c directory.c selectfile.c

ifdef EPPC
CSRCS += errorhandling.c
endif

# BINNAMES lists all binaries that will be linked.  Note that no suffix is
# required, as that will depend on whether this is a DEBUG build or not.
# The final name of the binaries will be $(BINNAME)$(BINSUFFIX)
BINNAMES 	= dvddemo1 dvddemo2 dvddemo3 directory

ifdef EPPC
BINNAMES += errorhandling
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
$(FULLBIN_ROOT)/dvddemo1$(BINSUFFIX): dvddemo1.o \
		      	$(DOLPHINLIBS)
$(FULLBIN_ROOT)/dvddemo2$(BINSUFFIX): dvddemo2.o \
		      	$(DOLPHINLIBS)
$(FULLBIN_ROOT)/dvddemo3$(BINSUFFIX): dvddemo3.o \
		      	$(DOLPHINLIBS)
$(FULLBIN_ROOT)/directory$(BINSUFFIX): directory.o \
				$(DOLPHINLIBS)
$(FULLBIN_ROOT)/errorhandling$(BINSUFFIX): errorhandling.o selectfile.o\
				$(DOLPHINLIBS)

