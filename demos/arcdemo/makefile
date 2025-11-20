###############################################################################
# ARC demo makefile
#
# Copyright 2001 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: /Dolphin/build/demos/arcdemo/makefile $
#   
#   1     7/03/01 3:18a Hashida
#   Initial revision.
#   
# $NoKeywords: $
#
###############################################################################

# simply build any basic code modules and headers
# tests are NOT installed.
all:    build

# module name should be set to the name of this subdirectory
MODULENAME  = arcdemo
DEMO        = TRUE

include $(BUILDTOOLS_ROOT)/commondefs

ifdef MAC

else
# EPPC

CSRCS    = arctest1.c arctest2.c arctest3.c

LCF_FILE = $(INC_ROOT)/dolphin/eppc.$(ARCH_TARGET).lcf

BINNAMES = arctest1 arctest2 arctest3

ARCLIB 	 = $(INSTALL_ROOT)/lib/arc$(LIBSUFFIX)

endif

# additional targets may be added here

include $(BUILDTOOLS_ROOT)/modulerules

# Dependencies and rules for any added rules may be placed here to take
# advantage of commondefs.

$(FULLBIN_ROOT)/arctest1$(BINSUFFIX): arctest1.o \
              $(DOLPHINLIBS) $(ARCLIB)

$(FULLBIN_ROOT)/arctest2$(BINSUFFIX): arctest2.o \
              $(DOLPHINLIBS) $(ARCLIB)

$(FULLBIN_ROOT)/arctest3$(BINSUFFIX): arctest3.o \
              $(DOLPHINLIBS) $(ARCLIB)

