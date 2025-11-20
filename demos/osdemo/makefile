###############################################################################
# Makefile for OS Overview demos
#
# Copyright 1998, 1999 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: /Dolphin/build/demos/osdemo/makefile $
#   
#   7     6/22/00 7:24p Tian
#   Added cppsetupdemo
#   
#   6     4/19/00 12:12p Danm
#   Moved skindmademo to tests/os.
#   
#   5     4/18/00 2:24p Danm
#   Added skindmademo.
#   
#   4     3/08/00 11:42p Shiki
#   Added error demo.
#   
#   3     2/16/00 2:24p Tian
#   Reorganized; added idlefunction and thread demos
#   
#   2     2/15/00 7:03p Tian
#   Added locked cache demos
#   
#   10    11/03/99 3:20p Shiki
#   Revised to support multiple platforms.
# $NoKeywords: $
#
###############################################################################

##################################
# QUICK START INSTRUCTIONS
# Type "make" at /dolphin/build/demos/osdemo to build DEBUG versions 
# of all demos
# Type "make NDEBUG=TRUE" to build OPTIMIZED versions of all tests
# Type "make stopwatchdemoD.bin" to build DEBUG version of just stopwatchdemo
# Type "make NDEBUG=TRUE stopwatchdemo.bin" to build OPTIMIZED version of 
# just stopwatchdemo
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
MODULENAME	= osdemo
DEMO		= TRUE

# CSRCS lists all C files that should be built
# The makefile determines which objects are linked into which binaries
# based on the dependencies you fill in at the bottom of this file
CSRCS		= stopwatchdemo.c \
			  allocdemo1-gettingmemory.c \
			  allocdemo2-oneheap.c \
			  allocdemo3-multipleheaps.c

CPPSRCS		= cppsetupdemo.cpp

# BINNAMES lists all binaries that will be linked.  Note that no suffix is
# required, as that will depend on whether this is a DEBUG build or not.
# The final name of the binaries will be $(BINNAME)$(BINSUFFIX)
BINNAMES 	= stopwatchdemo \
			  allocdemo1-gettingmemory \
			  allocdemo2-oneheap \
			  allocdemo3-multipleheaps \
			  cppsetupdemo

ifdef EPPC
CSRCS		+= cachedemo.c \
               timerdemo.c \
               idlefunctiondemo.c 	\
               threaddemo1.c \
               threaddemo2.c \
               threaddemo3.c \
               threaddemo4.c \
               threaddemo5.c \
               errordemo.c
BINNAMES 	+= cachedemo \
               timerdemo \
               idlefunctiondemo \
               threaddemo1 \
               threaddemo2 \
               threaddemo3 \
               threaddemo4 \
               threaddemo5 \
               errordemo

# defining a linker command file will have the build system generate it
# automatically and include it on the linker invocation line
LCF_FILE	= $(INC_ROOT)/dolphin/eppc.$(ARCH_TARGET).lcf

ifdef GEKKO
# GEKKO specific tests
CSRCS 		+= lockedcachedemo1.c lockedcachedemo2.c 
BINNAMES	+= lockedcachedemo1 lockedcachedemo2 
endif
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
$(FULLBIN_ROOT)/cachedemo$(BINSUFFIX): cachedemo.o \
		      	$(DOLPHINLIBS)
$(FULLBIN_ROOT)/timerdemo$(BINSUFFIX): timerdemo.o \
		      	$(DOLPHINLIBS)
$(FULLBIN_ROOT)/stopwatchdemo$(BINSUFFIX): stopwatchdemo.o \
			    $(DOLPHINLIBS)
$(FULLBIN_ROOT)/allocdemo1-gettingmemory$(BINSUFFIX): \
				allocdemo1-gettingmemory.o \
				$(DOLPHINLIBS)
$(FULLBIN_ROOT)/allocdemo2-oneheap$(BINSUFFIX): \
				allocdemo2-oneheap.o \
				$(DOLPHINLIBS)
$(FULLBIN_ROOT)/allocdemo3-multipleheaps$(BINSUFFIX): \
				allocdemo3-multipleheaps.o \
				$(DOLPHINLIBS)
$(FULLBIN_ROOT)/lockedcachedemo1$(BINSUFFIX): lockedcachedemo1.o \
		      	$(DOLPHINLIBS)
$(FULLBIN_ROOT)/lockedcachedemo2$(BINSUFFIX): lockedcachedemo2.o \
		      	$(DOLPHINLIBS)
$(FULLBIN_ROOT)/idlefunctiondemo$(BINSUFFIX): idlefunctiondemo.o \
		      	$(DOLPHINLIBS)
$(FULLBIN_ROOT)/threaddemo1$(BINSUFFIX): threaddemo1.o \
		      	$(DOLPHINLIBS)
$(FULLBIN_ROOT)/threaddemo2$(BINSUFFIX): threaddemo2.o \
		      	$(DOLPHINLIBS)
$(FULLBIN_ROOT)/threaddemo3$(BINSUFFIX): threaddemo3.o \
		      	$(DOLPHINLIBS)
$(FULLBIN_ROOT)/threaddemo4$(BINSUFFIX): threaddemo4.o \
		      	$(DOLPHINLIBS)
$(FULLBIN_ROOT)/threaddemo5$(BINSUFFIX): threaddemo5.o \
		      	$(DOLPHINLIBS)
$(FULLBIN_ROOT)/errordemo$(BINSUFFIX): errordemo.o \
		      	$(DOLPHINLIBS)
$(FULLBIN_ROOT)/cppsetupdemo$(BINSUFFIX): cppsetupdemo.o \
		      	$(DOLPHINLIBS)
