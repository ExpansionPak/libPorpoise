###############################################################################
# Makefile for HIO demos
#
# Copyright 1998 - 2001 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
#  $Log: /Dolphin/build/demos/hiodemo/makefile $
#   
#   8     11/26/01 8:59p Hirose
#   Separated host side build from main build path.
#   
#   7     3/15/01 2:13p Hashida
#   Restructuring hio libraries.
#   
#   6     3/05/01 1:57p Hashida
#   Added reboot demo.
#   
#   5     3/05/01 1:46p Hashida
#   Added color demo.
#   
#   4     3/05/01 12:30p Hashida
#   Changed so that host side application is also built by this makefile.
#   
#   3     3/05/01 11:46a Hashida
#   Added mcc_viewer sample
#   
#   2     11/28/00 4:39p Tian
#   Fixed lame bug in makefile
#   
#   1     11/27/00 3:46p Tian
#   Initial checkin
#  $NoKeywords: $
#
###############################################################################

ifdef X86
# build host PC side
all:	buildhost
else
# build GC hardware side
all:	setup build install
endif

ifdef X86
buildhost:
		@echo ========Build host side application========
		@./buildhostscript
		@echo ===========================================

clobber clean:
		@echo =======Clean up host side application======
		@./buildhostscript /CLEAN
		@echo ===========================================
endif

MODULENAME	= hiodemo
DEMO		= TRUE

include $(BUILDTOOLS_ROOT)/commondefs
ifdef EPPC
LCF_FILE	= $(INC_ROOT)/dolphin/eppc.$(ARCH_TARGET).lcf

CSRCS		=	main.c \
				mccViewer.c mccFiler.c \
				core.c coreTV.c corePAD.c bmp.c \
				print.c \
				color.c color_vi.c font12x24.c	\
				viewerMain.c viewerMcc.c viewerDraw.c

BINNAMES	= mcc_viewer hioprint color viewer

endif

include $(BUILDTOOLS_ROOT)/modulerules

$(FULLBIN_ROOT)/mcc_viewer$(BINSUFFIX): \
				main.o \
				mccViewer.o mccFiler.o \
				core.o coreTV.o corePAD.o bmp.o \
                $(DOLPHINLIBS)

$(FULLBIN_ROOT)/hioprint$(BINSUFFIX): print.o \
                    $(DOLPHINLIBS)

$(FULLBIN_ROOT)/color$(BINSUFFIX): color.o color_vi.o font12x24.o \
		      $(DOLPHINLIBS)

$(FULLBIN_ROOT)/viewer$(BINSUFFIX): \
			viewerMain.o viewerMcc.o viewerDraw.o bmp.o \
			$(DOLPHINLIBS)
