###############################################################################
# Makefile for videmo
#
# Copyright 2000 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: /Dolphin/build/demos/videmo/makefile $
#   
#   4     01/11/13 15:39 Hashida
#   Fixed a bug that smp-onetri_PAL is not built by default.
#   
#   3     11/04/01 10:37a Hashida
#   Added smp-onetri_PAL.c
#   
#   2     3/01/00 5:38p Hashida
#   Moved viewer from tests.
#   
#   1     2/18/00 10:39a Hashida
#   initial checkin
#   
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
MODULENAME	= videmo
DEMO		= TRUE

# CSRCS lists all C files that should be built
# The makefile determines which objects are linked into which binaries
# based on the dependencies you fill in at the bottom of this file
CSRCS		= color.c moving.c moving.30Hz.c viewer.c bmp.c smp-onetri_PAL.c

ifdef EPPC
# defining a linker command file will have the build system generate it
# automatically and include it on the linker invocation line
LCF_FILE	= $(INC_ROOT)/dolphin/eppc.$(ARCH_TARGET).lcf
endif

TVTYPES = _NTSC_INT _NTSC_DS _PAL_INT _PAL_DS _MPAL_INT _MPAL_DS

# BINNAMES lists all binaries that will be linked.  Note that no suffix is
# required, as that will depend on whether this is a DEBUG build or not.
# The final name of the binaries will be $(BINNAME)$(BINSUFFIX)
BINNAMES 	= color moving moving.30Hz $(addprefix viewer,$(TVTYPES)) \
		smp-onetri_PAL


# modulerules contains the rules that will use the above variables 
# and dependencies below to construct the binaries specified.
include $(BUILDTOOLS_ROOT)/modulerules


# Dependencies for the binaries listed in BINNAMES should come here
# They are your typical makefile rule, with extra variables to ensure
# that the names and paths match up.
# $(FULLBIN_ROOT) is the location of the local bin directory
# $(BINSUFFIX) depends on whether this is a debug build or not
# $(DOLPHINLIBS) includes all the Dolphin libraries.
$(FULLBIN_ROOT)/color$(BINSUFFIX): color.o \
		      $(DOLPHINLIBS)

$(FULLBIN_ROOT)/moving$(BINSUFFIX): moving.o \
		      $(DOLPHINLIBS)

$(FULLBIN_ROOT)/moving.30Hz$(BINSUFFIX): moving.30Hz.o \
		      $(DOLPHINLIBS)

$(FULLBIN_ROOT)/smp-onetri_PAL$(BINSUFFIX): smp-onetri_PAL.o \
		      $(DOLPHINLIBS)

#################################
## viewer
#################################
$(FULLBIN_ROOT)/viewer_NTSC_INT$(BINSUFFIX): viewer_NTSC_INT.o bmp.o\
              $(DOLPHINLIBS)
$(FULLBINOBJ_ROOT)/viewer_NTSC_INT.o: src/viewer.c
	$(CC) -DNTSC $(CCFLAGS) $(INCLUDES) $(COMPILE) $< \
		-o $(subst $(FULLBINOBJ_ROOT)/,$(BINOBJ_ROOT)/,$@)

$(FULLBIN_ROOT)/viewer_NTSC_DS$(BINSUFFIX): viewer_NTSC_DS.o bmp.o\
              $(DOLPHINLIBS)
$(FULLBINOBJ_ROOT)/viewer_NTSC_DS.o: src/viewer.c
	$(CC) -DNTSC -DNON_INTERLACE $(CCFLAGS) $(INCLUDES) $(COMPILE) $< \
		-o $(subst $(FULLBINOBJ_ROOT)/,$(BINOBJ_ROOT)/,$@)

$(FULLBIN_ROOT)/viewer_PAL_INT$(BINSUFFIX): viewer_PAL_INT.o bmp.o\
              $(DOLPHINLIBS)
$(FULLBINOBJ_ROOT)/viewer_PAL_INT.o: src/viewer.c
	$(CC) -DPAL $(CCFLAGS) $(INCLUDES) $(COMPILE) $< \
		-o $(subst $(FULLBINOBJ_ROOT)/,$(BINOBJ_ROOT)/,$@)

$(FULLBIN_ROOT)/viewer_PAL_DS$(BINSUFFIX): viewer_PAL_DS.o bmp.o\
              $(DOLPHINLIBS)
$(FULLBINOBJ_ROOT)/viewer_PAL_DS.o: src/viewer.c
	$(CC) -DPAL -DNON_INTERLACE $(CCFLAGS) $(INCLUDES) $(COMPILE) $< \
		-o $(subst $(FULLBINOBJ_ROOT)/,$(BINOBJ_ROOT)/,$@)

$(FULLBIN_ROOT)/viewer_MPAL_INT$(BINSUFFIX): viewer_MPAL_INT.o bmp.o\
              $(DOLPHINLIBS)
$(FULLBINOBJ_ROOT)/viewer_MPAL_INT.o: src/viewer.c
	$(CC) -DMPAL $(CCFLAGS) $(INCLUDES) $(COMPILE) $< \
		-o $(subst $(FULLBINOBJ_ROOT)/,$(BINOBJ_ROOT)/,$@)

$(FULLBIN_ROOT)/viewer_MPAL_DS$(BINSUFFIX): viewer_MPAL_DS.o bmp.o\
              $(DOLPHINLIBS)
$(FULLBINOBJ_ROOT)/viewer_MPAL_DS.o: src/viewer.c
	$(CC) -DMPAL -DNON_INTERLACE $(CCFLAGS) $(INCLUDES) $(COMPILE) $< \
		-o $(subst $(FULLBINOBJ_ROOT)/,$(BINOBJ_ROOT)/,$@)
