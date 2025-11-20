###############################################################################
# Makefile for axdemo
#
# Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
#  $Log: /Dolphin/build/demos/axdemo/makefile $
#   
#   13    11/26/01 6:41p Billyjack
#   removed axdemo
#   
#   12    9/05/01 4:33p Eugene
#   Updated AM API. 
#   
#   11    8/20/01 6:06p Billyjack
#   added axart3ddemo.c
#   
#   10    8/16/01 12:24p Billyjack
#   added am.c (ARAM manager for demos)
#   
#   9     7/20/01 7:17p Billyjack
#   removed ax3ddemo
#   
#   8     7/06/01 11:49a Billyjack
#   added ax3ddemo
#   
#   7     5/10/01 7:06p Billyjack
#   got rid of TXWIN references
#   
#   6     5/09/01 6:08p Billyjack
#   added delay
#   
#   5     3/26/01 2:32p Billyjack
#   added axstream
#   
#   4     3/21/01 11:49a Billyjack
#   removed synth
#   
#   3     3/15/01 2:04p Billyjack
#   added syndemo
#   
#   2     2/22/01 5:10p Billyjack
#   now uses mix
#   
#   1     1/12/01 2:58p Billyjack
#   created
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
MODULENAME	= axdemo
DEMO		= TRUE

# CSRCS lists all C files that should be built
# The makefile determines which objects are linked into which binaries
# based on the dependencies you fill in at the bottom of this file
CSRCS		= axstream.c    \
              syndemo.c     \
              seqdemo.c     \
              delay.c       \
              axart3ddemo.c

# BINNAMES lists all binaries that will be linked.  Note that no suffix is
# required, as that will depend on whether this is a DEBUG build or not.
# The final name of the binaries will be $(BINNAME)$(BINSUFFIX)
BINNAMES 	= axstream      \
              syndemo       \
              seqdemo       \
              axart3ddemo    

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

$(FULLBIN_ROOT)/axstream$(BINSUFFIX):	axstream.o	\
										$(DOLPHINLIBS) 

$(FULLBIN_ROOT)/syndemo$(BINSUFFIX):	syndemo.o	\
										$(DOLPHINLIBS) 

$(FULLBIN_ROOT)/seqdemo$(BINSUFFIX):	seqdemo.o	\
										$(DOLPHINLIBS) 

$(FULLBIN_ROOT)/axart3ddemo$(BINSUFFIX):axart3ddemo.o   \
										$(DOLPHINLIBS) 

#======== End of makefile ========#
