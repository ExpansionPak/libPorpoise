###############################################################################
# CARD demo makefile
#
# Copyright 2000, 2001 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: /Dolphin/build/demos/carddemo/makefile $
#   
#   5     7/13/01 14:19 Shiki
#   Added MULTIBYTE = TRUE.
#   
#   4     6/19/01 1:05p Dante
#   Added new create demos
#   
#   3     6/14/01 11:37a Shiki
#   Added save.
#   
#   2     5/18/01 5:51p Shiki
#   Added listdemo.
#   
#   1     11/28/00 2:13p Tian
#   Initial checkin
# $NoKeywords: $
#
###############################################################################

all:    setup build install

# module name should be set to the name of this subdirectory
MODULENAME  = carddemo
DEMO        = TRUE

# listdemo.c uses Japanese Kanji characters
MULTIBYTE   = TRUE

include $(BUILDTOOLS_ROOT)/commondefs

ifdef MAC

else
# EPPC

CSRCS    = format.c create.c list.c save.c \
           create-1icon.c create-8icon.c create-bnr-4icon.c

# listdemo files
CSRCS   += listdemo.c cardutil.c cont.c 

LCF_FILE = $(INC_ROOT)/dolphin/eppc.$(ARCH_TARGET).lcf

BINNAMES = format create list save listdemo \
           create-1icon create-8icon create-bnr-4icon

endif

include $(BUILDTOOLS_ROOT)/modulerules

$(FULLBIN_ROOT)/format$(BINSUFFIX): format.o \
              $(DOLPHINLIBS)

$(FULLBIN_ROOT)/create$(BINSUFFIX): create.o \
              $(DOLPHINLIBS)

$(FULLBIN_ROOT)/list$(BINSUFFIX): list.o \
              $(DOLPHINLIBS)

$(FULLBIN_ROOT)/save$(BINSUFFIX): save.o \
              $(DOLPHINLIBS)

$(FULLBIN_ROOT)/listdemo$(BINSUFFIX): listdemo.o cont.o cardutil.o \
              $(DOLPHINLIBS)

$(FULLBIN_ROOT)/create-1icon$(BINSUFFIX): create-1icon.o \
              $(DOLPHINLIBS)

$(FULLBIN_ROOT)/create-8icon$(BINSUFFIX): create-8icon.o \
              $(DOLPHINLIBS)

$(FULLBIN_ROOT)/create-bnr-4icon$(BINSUFFIX): create-bnr-4icon.o \
              $(DOLPHINLIBS)
