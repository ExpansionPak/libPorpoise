###############################################################################
# Makefile for GX demos
#
# Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
#  $Log: /Dolphin/build/demos/gxdemo/makefile $
#   
#   41    7/05/01 10:15a Hirose
#   added DL-tf-mtx
#   
#   40    6/12/01 8:33p Hirose
#   got rid of tev-emu.c
#   
#   39    6/12/01 3:37p Hirose
#   deleted DL-state and tev-clamp.
#   
#   38    4/11/01 4:52p John
#   Moved and renamed some demos:
#   Simple/smp-dualfifo --> Management/mgt-fifo-brkpt (EPPC only)
#   Simple/smp-dualfifo-sort --> Management/mgt-fifo-dual (EPPC only)
#   Framebuffer/frb-triple --> Management/mgt-triple-buf (EPPC only)
#   Framebuffer/frb-single --> Management/mgt-single-buf (EPPC and MAC)
#   
#   37    4/04/01 5:12p John
#   Moved smp-dualfifo-sort to EPPC build only.
#   
#   36    4/03/01 2:56p Dante
#   Added tg-clr-persp
#   
#   35    4/03/01 11:14a John
#   Added smp-dualfifo-sort demo.
#   
#   34    3/23/01 12:03p John
#   Removed smp-gxmodels and smp-gxmodels-perf.
#   
#   33    2/15/01 11:58a Carl
#   Added DL-tex.
#   
#   32    11/15/00 6:04p Carl
#   Added tf-clip-bug.
#   
#   31    11/01/00 5:58p Hirose
#   added tev-swap and tg-dual
#   
#   30    10/06/00 12:10p Tian
#   Added smp-dualfifo.c
#   
#   29    8/23/00 2:55p Alligator
#   added perf-sync demo
#   
#   28    8/04/00 5:43p Hirose
#   moved DL-base and DL-overflow into test directory again
#   
#   27    8/04/00 2:20p Carl
#   Added ind-warp
#   
#   26    8/03/00 7:17p Hirose
#   removed frb-aa-full/perf-geo/DL-state from MAC build
#   
#   25    7/31/00 7:42p Hirose
#   added smp-light
#   
#   24    7/18/00 4:52p Carl
#   Added ind-pseudo-3d.
#   
#   23    7/06/00 5:08p Tian
#   Added smp-gxmodels-perf
#   
#   22    6/27/00 1:40p Carl
#   Fixed comment problem with frb-preempt.
#   
#   21    6/23/00 7:31p Carl
#   Added ind-tile-test.
#   
#   20    6/22/00 7:29p Carl
#   Added bump demos.
#   
#   19    6/20/00 12:15p Alligator
#   added perf demos
#   
#   18    6/07/00 5:02p Hirose
#   added frb-gamma / deleted frb-preempt (moved to test directory)
#   
#   17    5/30/00 5:07p Carl
#   Moved frb-triple and frb-preempt to EPPC-only section.
#   Removed seperate list for GPL/TPL bins.
#   Reordered the various sections to be more alphabetical.
#   
#   16    5/24/00 7:03p Hirose
#   added frb-bound-box
#   
#   15    5/24/00 3:53a Carl
#   Fixed missing $'s.
#   
#   14    5/24/00 3:24a Hirose
#   added frb-preempt
#   
#   13    5/23/00 7:15p Carl
#   Added frb-aa-full, frb-single, frb-triple.
#   
#   12    5/21/00 11:05p Alligator
#   added frb aa tests, deleted frb-aa and frb-aa-simple
#   
#   11    5/12/00 5:33p Alligator
#   fix bug in last checkin
#   
#   10    5/12/00 4:52p Alligator
#   added sphere map tests
#   
#   9     5/11/00 10:58p Hirose
#   added tev-ztex
#   
#   8     4/13/00 8:37p Hirose
#   added tg-shadow3
#   
#   7     4/02/00 10:09p Hirose
#   added tg-shadow2
#   
#   6     3/24/00 7:11p Hirose
#   deleted some source files which became unnecessary after DEMOPad
#   library implimentation
#   
#   5     3/17/00 2:44p Hirose
#   added tev-multi and tev-complex
#   
#   4     3/16/00 7:24p Alligator
#   rename simple tests, make singleframe versions
#   
#   3     3/06/00 12:15p Alligator
#   add demos from tests/gx and rename.  structure makefile like test
#   makefile.
#   
#   22    3/03/00 4:21p Alligator
#   integrate with ArtX source
#   
#   21    2/28/00 4:15p Alligator
#   added previewer test
#   
#   20    2/23/00 7:03p Carl
#   Added code to pass field flags to the compiler.
#   
#   19    2/21/00 6:02p Danm
#   Added support for frb-test06, a more sophisticated AA test case.
#   
#   18    2/15/00 6:14p Hirose
#   added tev-test01
#   
#   17    2/14/00 4:26p Alligator
#   deleted donut tests from makefile
#   
#   16    2/12/00 5:16p Alligator
#   Integrate ArtX source tree changes
#   
#   15    2/09/00 6:48p Danm
#   Added frb-test05.c to test AA.
#   
#   14    2/07/00 6:06p Carl
#   Added DL-test03
#   
#   13    2/02/00 11:32p Hirose
#   added tev-test00
#   
#   12    2/01/00 7:39p Alligator
#   
#   11    2/01/00 7:22p Alligator
#   second GX update from ArtX
#   
#   10    1/25/00 2:53p Tian
#   Removed eppc test00 - it was using illegal means of accessing data
#   structures.
#   
#   9     1/20/00 5:56p Danm
#   Single frame testing for single field rendering.
#   
#   8     1/13/00 5:55p Alligator
#   integrate with ArtX GX library code
#   
#   7     1/05/00 6:56p Hirose
#   added tex-test09 and tex-test11
#   
#   6     12/24/99 3:41p Hirose
#   added tex-test10
#   
#   5     12/17/99 7:24p Hirose
#   added tex-test07 and tex-test08
#   
#   4     12/13/99 6:45p Yasu
#   Add tf-test04
#   
#   3     99/12/12 10:06p Paul
#   
#   2     11/30/99 12:01p Carl
#   Added DL-test02.
#   
#   64    11/17/99 11:59a Alligator
#   added tex-test06 
#   
#   63    11/16/99 6:00p Tian
#   Fix singleframe default for eppc builds.
#   
#   62    11/12/99 1:09p Alligator
#   added GXProject, GXGetViewportv, and GXGetProjectionv functions.
#   
#   61    11/11/99 11:56p Hirose
#   added lit-test11
#   
#   60    11/11/99 6:57p Tian
#   Added basic charpipeline libs to EPPC builds, thus enabling all tests
#   to be built for EPPC.
#   
#   59    11/11/99 4:03p Yasu
#   Added tev-test05
#   
#   58    11/10/99 4:29p Tian
#   Temporarily removed tf-test03 from build
#   
#   57    11/10/99 3:51p Tian
#   Added fifo-test02 and initial display list tests
#   
#   56    11/10/99 3:27p Alligator
#   added tf-test03
#   
#   55    11/04/99 3:56p Hirose
#   added frb-test02
#   
#   54    11/03/99 3:13p Shiki
#   Revised to support multiple platforms.
#
#  $NoKeywords: $
#
###############################################################################

##################################
# QUICK START INSTRUCTIONS
# Type "make" at /dolphin/build/tests/gx to build DEBUG versions of all tests.
# Type "make NDEBUG=TRUE" to build OPTIMIZED versions of all tests
# Type "make lit-basicD.bin" to build DEBUG version of just lit-basic
# Type "make NDEBUG=TRUE lit-basic.bin" to build OPTIMIZED version of 
# just lit-basic
#
# To add another test
# 1. add the .c files to CSRCS to make sure they are built
# 2. add the binary name (no suffix) to BINNAMES
# 3. add a dependency rule for this executable at the bottom of this file
##################################


# All modules have "setup" and "build" as targets.  System libraries 
# and demo programs also have an "install" target that copies the compiled
# binaries to the binary tree (/dolphin/$(ARCH_TARGET)).
# tests are NOT installed into the binary release directory.
all:	setup build install

# module name should be set to the name of this subdirectory
# DEMO = TRUE indicates that this module resides under the "demos" subtree.
# The list of selectable paths can be found in modulerules.
MODULENAME	= gxdemo
DEMO		= TRUE

ifeq ($(PLATFORM), SPRUCE_MARLIN)
ifndef __SINGLEFRAME
# automatically enable single frame mode if compiling for EPPC
__SINGLEFRAME = 0
endif
endif

# commondefs must be included near the top so that all common variables
# will be defined before their use.
include $(BUILDTOOLS_ROOT)/commondefs

# pass field flags on to the compiler
ifeq ($(__ODDFIELD), TRUE)
CCFLAGS += -D__ODDFIELD
endif

ifeq ($(__EVENFIELD), TRUE)
CCFLAGS += -D__EVENFIELD
endif

ifdef __HWSIM
CCFLAGS += -D__HWSIM=1
else
CCFLAGS += -D__HWSIM=0
endif

# common sources built for both EPPC and MAC
# CSRCS lists all C files that should be built
# The makefile determines which objects are linked into which binaries
# based on the dependencies you fill in at the bottom of this file
CSRCS = \
		Culling/cul-faces.c 	 Culling/cul-viewport.c \
		\
		DisplayList/DL-geom.c	 \
		\
		Framebuffer/frb-bound-box.c   \
		Framebuffer/frb-copy.c		  \
		Framebuffer/frb-fld-dbs.c	  \
		Framebuffer/frb-fld-dbs-aa.c  \
		Framebuffer/frb-fld-int.c	  \
		Framebuffer/frb-fld-int-aa.c  \
		Framebuffer/frb-zcopy.c 	  \
		Framebuffer/frb-gamma.c 	  \
		\
		G2D/G2D-test.c			 G2D/G2D-testEditor.c \
		G2D/G2D-testPhy.c		 G2D/G2D-testLevel1.c \
		\
		Geometry/geo-particle.c  \
		Geometry/geo-vtx-fmt.c	 Geometry/geo-vtx-fmtPrim.c \
		\
		Indirect/ind-bump-st.c	 Indirect/ind-bump-xyz.c \
		Indirect/ind-tile-data.c Indirect/ind-tile-test.c \
		Indirect/ind-pseudo-3d.c Indirect/ind-warp.c	 \
		\
		Lighting/lit-basic.c	 Lighting/lit-multi.c	 \
		Lighting/lit-alpha.c	 Lighting/lit-spot.c	 \
		Lighting/lit-dist-atn.c  Lighting/lit-unclamp.c  \
		Lighting/lit-cartoon.c	 Lighting/lit-texture.c  \
		Lighting/lit-tx-alpha.c  Lighting/lit-prelit.c	 \
		Lighting/lit-atn-func.c  Lighting/lit-specular.c \
		\
		Performance/perf-fill.c  \
		Performance/perf-sync.c  \
		\
		PixelProc/cmn-model.c	 PixelProc/pix-fog.c	\
		PixelProc/pix-blend.c	 PixelProc/pix-logic.c	\
		PixelProc/pix-z-comp.c	 PixelProc/pix-a-comp.c \
		\
		Simple/smp-onetri.c 	 Simple/smp-onetri_dl.c \
		Simple/smp-toy-draw.c  \
		Simple/smp-toy-anim.c	 Simple/smp-toy-stick.c \
		Simple/smp-texexample.c  Simple/smp-octa.c \
		Simple/smp-light.c		 \
		\
		Tev/tev-one-op.c		Tev/tev-multi.c \
		Tev/tev-complex.c		Tev/tev-outline.c \
		Tev/tev-ztex.c			Tev/tev-swap.c    \
		\
		TexGen/tg-basic.c		TexGen/tg-project.c	 \
		TexGen/tg-emboss.c		TexGen/tg-shadow.c 	 \
		TexGen/tg-shadow2.c 	TexGen/tg-shadow3.c	 \
		TexGen/tg-light-fx.c	TexGen/tg-dual.c		 \
		TexGen/spheremap.c		TexGen/tg-spheremap.c	 \
		TexGen/parabolicmap.c	 TexGen/tg-parabolicmap.c \
		TexGen/tg-clr-persp.c \
		\
		Texture/tex-fmt-cube.c	 Texture/tex-fmt-cubeData.c \
		Texture/tex-filter.c	 Texture/tex-fmt-tpl.c		\
		Texture/tex-wrap.c		 Texture/tex-tlut.c 		\
		Texture/tex-tlutData.c	 Texture/tex-layer.c		\
		Texture/tex-2-tex.c 	 Texture/tex-tc-alloc.c 	\
		Texture/tex-preload.c	 Texture/tex-mix-mode.c 	\
		Texture/tex-invalid.c	 Texture/tex-lod.c			\
		\
		Transform/tf-pn-mtx.c	 Transform/tf-stitch.c \
		Transform/tf-tg-mtx.c	 Transform/tf-mirror.c \
		Transform/tf-reflect.c	 Transform/tf-clip-bug.c \
		\
		Management/mgt-single-buf.c

# common binaries linked for both EPPC and MAC
# BINNAMES lists all binaries that will be linked.  Note that no suffix is
# required, as that will depend on whether this is a DEBUG build or not.
# The final name of the binaries will be $(BINNAME)$(BINSUFFIX)
BINNAMES =	\
		cul-faces		cul-viewport	\
		\
		DL-geom 		\
		\
		frb-bound-box	frb-copy		\
		frb-fld-dbs 	frb-fld-dbs-aa	frb-fld-int 	frb-fld-int-aa	\
		frb-zcopy		frb-gamma		\
		\
		G2D-test		\
		\
		geo-particle	geo-vtx-fmt 	\
		\
		ind-bump-st 	ind-bump-xyz	ind-tile-test	ind-pseudo-3d	\
		ind-warp		\
		\
		lit-basic		lit-multi		lit-alpha		lit-spot		\
		lit-dist-atn	lit-unclamp 	lit-cartoon 	lit-texture 	\
		lit-tx-alpha	lit-prelit		lit-atn-func	lit-specular	\
		\
		perf-fill		\
		perf-sync		\
		\
		pix-fog 		\
		pix-blend		pix-logic		pix-z-comp		pix-a-comp		\
		\
		smp-onetri		smp-onetri_dl	smp-toy 		\
		smp-texexample	smp-octa		smp-light		\
		\
		tev-one-op	tev-multi		tev-complex		\
		tev-outline 	tev-ztex		tev-swap		\
		\
		tg-basic		tg-project		tg-emboss		tg-shadow		\
		tg-shadow2		tg-shadow3		tg-light-fx 	tg-dual 		\
		tg-spheremap	tg-parabolicmap tg-clr-persp \
		\
		tex-fmt-cube	tex-filter		tex-fmt-tpl 	tex-wrap		\
		tex-tlut		tex-layer		tex-2-tex		tex-tc-alloc	\
		tex-preload 	tex-mix-mode	tex-invalid 	tex-lod 		\
		\
		tf-pn-mtx		tf-stitch		tf-tg-mtx		tf-mirror		\
		tf-reflect		tf-clip-bug	\
		\
		mgt-single-buf	

# EPPC specific demo source codes
ifdef EPPC
CSRCS += \
		DisplayList/DL-tex.c \
		DisplayList/DL-tf-mtx.c \
		Framebuffer/frb-aa-full.c \
		Performance/perf-geo.c \
		Management/mgt-triple-buf.c \
		Management/mgt-fifo-brkpt.c \
		Management/mgt-fifo-dual.c

LCF_FILE =  $(INC_ROOT)/dolphin/eppc.$(ARCH_TARGET).lcf

# EPPC specific binaries
BINNAMES += \
		DL-tex \
		DL-tf-mtx \
		frb-aa-full 	\
		perf-geo	\
		mgt-triple-buf	mgt-fifo-brkpt	mgt-fifo-dual

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
$(FULLBIN_ROOT)/cul-faces$(BINSUFFIX): Culling/cul-faces.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/cul-viewport$(BINSUFFIX): Culling/cul-viewport.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/DL-geom$(BINSUFFIX):  DisplayList/DL-geom.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/DL-tex$(BINSUFFIX):  DisplayList/DL-tex.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/DL-tf-mtx$(BINSUFFIX):  DisplayList/DL-tf-mtx.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/frb-copy$(BINSUFFIX): Framebuffer/frb-copy.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/frb-zcopy$(BINSUFFIX): Framebuffer/frb-zcopy.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/frb-fld-int$(BINSUFFIX): Framebuffer/frb-fld-int.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/frb-fld-dbs$(BINSUFFIX): Framebuffer/frb-fld-dbs.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/frb-fld-int-aa$(BINSUFFIX): Framebuffer/frb-fld-int-aa.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/frb-fld-dbs-aa$(BINSUFFIX): Framebuffer/frb-fld-dbs-aa.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/frb-aa-full$(BINSUFFIX): Framebuffer/frb-aa-full.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/frb-bound-box$(BINSUFFIX): Framebuffer/frb-bound-box.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/frb-gamma$(BINSUFFIX): Framebuffer/frb-gamma.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/G2D-test$(BINSUFFIX): G2D/G2D-test.o \
					G2D/G2D-testPhy.o \
					G2D/G2D-testLevel1.o \
					G2D/G2D-testEditor.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/geo-particle$(BINSUFFIX): Geometry/geo-particle.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/geo-vtx-fmt$(BINSUFFIX): Geometry/geo-vtx-fmt.o \
					Geometry/geo-vtx-fmtPrim.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/ind-bump-st$(BINSUFFIX): Indirect/ind-bump-st.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/ind-bump-xyz$(BINSUFFIX): Indirect/ind-bump-xyz.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/ind-tile-test$(BINSUFFIX): Indirect/ind-tile-test.o Indirect/ind-tile-data.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/ind-pseudo-3d$(BINSUFFIX): Indirect/ind-pseudo-3d.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/ind-warp$(BINSUFFIX): Indirect/ind-warp.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/lit-basic$(BINSUFFIX): Lighting/lit-basic.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/lit-multi$(BINSUFFIX): Lighting/lit-multi.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/lit-alpha$(BINSUFFIX): Lighting/lit-alpha.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/lit-spot$(BINSUFFIX): Lighting/lit-spot.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/lit-dist-atn$(BINSUFFIX): Lighting/lit-dist-atn.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/lit-unclamp$(BINSUFFIX): Lighting/lit-unclamp.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/lit-cartoon$(BINSUFFIX): Lighting/lit-cartoon.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/lit-texture$(BINSUFFIX): Lighting/lit-texture.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/lit-tx-alpha$(BINSUFFIX): Lighting/lit-tx-alpha.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/lit-prelit$(BINSUFFIX): Lighting/lit-prelit.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/lit-atn-func$(BINSUFFIX): Lighting/lit-atn-func.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/lit-specular$(BINSUFFIX): Lighting/lit-specular.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/perf-fill$(BINSUFFIX): Performance/perf-fill.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/perf-sync$(BINSUFFIX): Performance/perf-sync.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/perf-geo$(BINSUFFIX): Performance/perf-geo.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/pix-fog$(BINSUFFIX): PixelProc/pix-fog.o \
					PixelProc/cmn-model.o $(DOLPHINLIBS)

$(FULLBIN_ROOT)/pix-blend$(BINSUFFIX): PixelProc/pix-blend.o \
					PixelProc/cmn-model.o $(DOLPHINLIBS)

$(FULLBIN_ROOT)/pix-logic$(BINSUFFIX): PixelProc/pix-logic.o \
					PixelProc/cmn-model.o $(DOLPHINLIBS)

$(FULLBIN_ROOT)/pix-z-comp$(BINSUFFIX): PixelProc/pix-z-comp.o \
					PixelProc/cmn-model.o $(DOLPHINLIBS)

$(FULLBIN_ROOT)/pix-a-comp$(BINSUFFIX): PixelProc/pix-a-comp.o \
					PixelProc/cmn-model.o $(DOLPHINLIBS)

$(FULLBIN_ROOT)/smp-onetri$(BINSUFFIX): Simple/smp-onetri.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/smp-onetri_dl$(BINSUFFIX): Simple/smp-onetri_dl.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/smp-toy$(BINSUFFIX): Simple/smp-toy-draw.o \
					Simple/smp-toy-anim.o Simple/smp-toy-stick.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/smp-texexample$(BINSUFFIX): Simple/smp-texexample.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/smp-octa$(BINSUFFIX): Simple/smp-octa.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/smp-light$(BINSUFFIX): Simple/smp-light.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tev-one-op$(BINSUFFIX): Tev/tev-one-op.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tev-swap$(BINSUFFIX): Tev/tev-swap.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tev-multi$(BINSUFFIX): Tev/tev-multi.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tev-complex$(BINSUFFIX): Tev/tev-complex.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tev-outline$(BINSUFFIX): Tev/tev-outline.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tev-ztex$(BINSUFFIX): Tev/tev-ztex.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tg-basic$(BINSUFFIX):  TexGen/tg-basic.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tg-project$(BINSUFFIX):  TexGen/tg-project.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tg-emboss$(BINSUFFIX):	TexGen/tg-emboss.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tg-shadow$(BINSUFFIX):	TexGen/tg-shadow.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tg-shadow2$(BINSUFFIX):  TexGen/tg-shadow2.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tg-shadow3$(BINSUFFIX):  TexGen/tg-shadow3.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tg-spheremap$(BINSUFFIX):  TexGen/tg-spheremap.o \
					TexGen/spheremap.o $(DOLPHINLIBS)

$(FULLBIN_ROOT)/tg-parabolicmap$(BINSUFFIX):  TexGen/tg-parabolicmap.o \
					TexGen/parabolicmap.o $(DOLPHINLIBS)

$(FULLBIN_ROOT)/tg-light-fx$(BINSUFFIX):  TexGen/tg-light-fx.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tg-dual$(BINSUFFIX):  TexGen/tg-dual.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tg-clr-persp$(BINSUFFIX):  TexGen/tg-clr-persp.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tex-fmt-cube$(BINSUFFIX): Texture/tex-fmt-cube.o \
					Texture/tex-fmt-cubeData.o $(DOLPHINLIBS)

$(FULLBIN_ROOT)/tex-filter$(BINSUFFIX): Texture/tex-filter.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tex-fmt-tpl$(BINSUFFIX): Texture/tex-fmt-tpl.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tex-wrap$(BINSUFFIX): Texture/tex-wrap.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tex-tlut$(BINSUFFIX): Texture/tex-tlut.o \
					Texture/tex-tlutData.o $(DOLPHINLIBS)

$(FULLBIN_ROOT)/tex-layer$(BINSUFFIX): Texture/tex-layer.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tex-2-tex$(BINSUFFIX): Texture/tex-2-tex.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tex-tc-alloc$(BINSUFFIX): Texture/tex-tc-alloc.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tex-preload$(BINSUFFIX): Texture/tex-preload.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tex-mix-mode$(BINSUFFIX): Texture/tex-mix-mode.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tex-invalid$(BINSUFFIX): Texture/tex-invalid.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tex-lod$(BINSUFFIX): Texture/tex-lod.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tf-pn-mtx$(BINSUFFIX):	Transform/tf-pn-mtx.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tf-stitch$(BINSUFFIX):	Transform/tf-stitch.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tf-tg-mtx$(BINSUFFIX):	Transform/tf-tg-mtx.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tf-mirror$(BINSUFFIX):	Transform/tf-mirror.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tf-reflect$(BINSUFFIX):  Transform/tf-reflect.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/tf-clip-bug$(BINSUFFIX):  Transform/tf-clip-bug.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/mgt-single-buf$(BINSUFFIX): Management/mgt-single-buf.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/mgt-triple-buf$(BINSUFFIX): Management/mgt-triple-buf.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/mgt-fifo-brkpt$(BINSUFFIX): Management/mgt-fifo-brkpt.o \
					$(DOLPHINLIBS)

$(FULLBIN_ROOT)/mgt-fifo-dual$(BINSUFFIX): Management/mgt-fifo-dual.o \
					$(DOLPHINLIBS)

#======== End of makefile ========#
