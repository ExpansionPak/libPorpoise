##########################################################################
#
# This is a simple Makefile to show how to compile a list of C files into
# a Dolphin binary. Game developers can use this to begin learning how to
# build Dolphin software.
#
# The Dolphin SDK build system is very complex because we need to
# support many prototype development hardware configurations. This file
# ignores most of those configurations and builds only MAC, HW1, and HW2
# targets 
#
# $Log: /Dolphin/build/demos/samplemake/Makefile $
#   
#   11    8/07/01 3:12p Carl
#   HW2 now default; removed CP libs.
#   
#   10    3/26/01 5:13p Carl
#   Added BUG_XF_STALL define.
#   
#   9     11/05/00 6:17p Hirose
#   added GX_REV build flag
#   
#   8     11/01/00 4:40p Tian
#   Added ODEMU support.  AMC DDK no longer required for building.
#   
#   7     10/10/00 11:41a Tian
#   Added log.  Added support for HW1 and HW2.  Updated bug lists and
#   compiler parameters.
# $NoKeywords: $
#
##########################################################################

##########################################################################
# BUILD CONFIGURATION
#
# Change Makefile variable PLATFORM to build for different prototype system
#	HW1, HW2, or MAC
#
# Define Makefile variable DEBUG to build non-optimized application
#
##########################################################################

PLATFORM	= HW2

#PLATFORM	= MAC
#PLATFORM	= HW2
#DEBUG		= TRUE

##########################################################################
# TOOLS
##########################################################################

##########################################################################
# Compiler tools

# empty, space	used to convert paths to UNIX style, forward slash '/' and
#		' ' escaped by '\'
# MWDIR		top level directory of Metrowerks CodeWarrior product
# CWFOLDER	environment variable installed by CodeWarrior installation
# MWCLDIR	command line tool directory
# MWLIBDIR	standard C library directory
# CC AS LD AR	command line tools
# MWINCS	standard library includes
# MWLIBS	standard libraries + EABI libraries

empty		:=
space		:= $(empty) $(empty)
MWDIR		= $(subst $(space),\$(space),$(subst \,/,$(CWFOLDER)))

ifneq ($(PLATFORM), MAC)

	MWCLDIR	= $(MWDIR)/PowerPC_EABI_TOOLS/Command_Line_Tools
	MWLIBDIR= $(MWDIR)/PowerPC_EABI_Support/Runtime/Lib

	CC	= $(MWCLDIR)/mwcceppc.exe
	AS	= $(MWCLDIR)/mwasmeppc.exe
	LD	= $(MWCLDIR)/mwldeppc.exe
	AR	= $(MWCLDIR)/mwldeppc.exe

	MWINCS	= -ir $(MWDIR)/PowerPC_EABI_Support/Msl/Msl_c \
		  -I$(MWDIR)/PowerPC_EABI_Support/Runtime/Inc

	MWLIBS	= $(MWDIR)/PowerPC_EABI_Support/MetroTRK/TRK_MINNOW_DOLPHIN.a	\
		  $(MWLIBDIR)/Runtime.PPCEABI.H.a				\
		  $(MWDIR)/PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/Lib/MSL_C.PPCEABI.bare.H.a \
	 	  $(MWDIR)/PowerPC_EABI_Support/MSL/MSL_C++/PPC_EABI/Lib/MSL_C++.PPCEABI.bare.H.a

else

	MWCLDIR	= $(MWDIR)/Tools/Command\ Line\ Tools
	MWLIBDIR= $(MWDIR)/MacOS\ Support/Libraries

	CC	= $(MWCLDIR)/MWCPPC.exe
	LD	= $(MWCLDIR)/MWLinkPPC.exe
	AR	= $(MWCLDIR)/MWLinkPPC.exe
	AS	= "THERE IS NO MACOS ASSEMBLER"

	MWINCS	= -i $(DOLPHIN_ROOT)/include/Mac/InputSprocket \
		  -i $(DOLPHIN_ROOT)/include/Mac/OpenGL \
		  -ir $(MWDIR)/MacOS\ Support/MacHeaders \
		  -ir $(MWDIR)/MacOS\ Support/Headers \
		  -i $(MWDIR)/MacOS\ Support/Universal/Interfaces/CIncludes \
		  -i $(MWDIR)/MSL/MSL_C/MSL_MacOS/Include \
		  -i $(MWDIR)/MSL/MSL_C/MSL_Common/Include

	MWLIBS	= $(MWDIR)/MacOS\ Support/Universal/Libraries/StubLibraries/MathLib \
		  $(MWDIR)/MacOS\ Support/Universal/Libraries/StubLibraries/InterfaceLib \
		  $(MWLIBDIR)/Runtime/Runtime\ PPC/MSL\ RuntimePPC.Lib \
		  $(MWDIR)/MSL/MSL_C/MSL_MacOS/Lib/PPC/MSL\ C.PPC.Lib \
		  $(MWDIR)/MSL/MSL_C/MSL_MacOS/Lib/PPC/MSL\ SIOUX.PPC.Lib

endif


##########################################################################
# debug channel library

# AMCDDKBIN	environment variable in shell installed by DDK installation

ifneq ($(PLATFORM), MAC)
	ifdef AMCDDKBIN
		AMCDIR    = $(subst $(space),\$(space),$(subst \,/,$(AMCDDKBIN)))
		AMCLIBS   = $(AMCDIR)/../lib/AmcExi2.a -lamcnotstub.a
	else
		AMCLIBS   = -lamcstubs.a
	endif # AMCDDKBIN


	ifdef ODEMUSDKBIN
		ODEDIR    = $(subst $(space),\$(space),$(subst \,/,$(ODEMUSDKBIN)))
		ODELIBS   = $(ODEDIR)/../lib/odemuexi2.a -lodenotstub.a
	else  # ODEMUSDKBIN
		ODELIBS   = -lodemustubs.a
	endif # ODEMUSDKBIN
else
endif



##########################################################################
# DOLPHIN SDK
##########################################################################

# SDKINC	SDK library include path
# SDKLIBDIR	SDK library path
# SDKLIB_NAME	base name of SDK libraries used during linking
# SDKLIBS	converted names of SDK libraries used on the linker command line

ifneq ($(PLATFORM), MAC)

	SDKINC	= -i- \
		  -I$(DOLPHIN_ROOT)/include

	SDKLIBDIR= $(DOLPHIN_ROOT)/$(PLATFORM)/lib

	LIBNAME	= base os db mtx dvd vi demo pad ai ar dsp card hio perf \
	 	  gx G2D texPalette fileCache support

	SDKLIBS1= $(addprefix -l, $(LIBNAME))
	SDKLIBS	= $(addsuffix .a, $(SDKLIBS1))

else

	SDKINC	= -i- \
		  -i $(DOLPHIN_ROOT)/include \
		  -i $(DOLPHIN_ROOT)/include/Mac/InputSprocket \
		  -i $(DOLPHIN_ROOT)/include/Mac/OpenGL \

	SDKLIBDIR= $(DOLPHIN_ROOT)/MAC/lib

	LIBNAME	= os emu mtx demo pad \
	 	  emu_CP gx G2D geoPalette texPalette fileCache support \
	 	  control actor anim lighting shader

	SDKLIBS1= $(addprefix -l, $(LIBNAME))
	SDKLIBS	= $(addsuffix .lib, $(SDKLIBS1))

	SDKLIBS	+= -lInputSprocket/InputSprocketLib \
		   -lOpenGL/glut.lib \
		   -lOpenGL/OpenGLLibraryStub \
		   -lOpenGL/OpenGLUtilityStub
endif

##########################################################################
# COMPILE FLAGS
##########################################################################

# INCS		include flags
# CFLAGS	C compiler flags

INCS		= $(SDKINC) $(MWINCS)

ifndef DEBUG
	DBGFLAG	= -DNDEBUG -O4,p -inline auto -pragma "warn_notinlined off"
else
	DBGFLAG = -opt off -D_DEBUG -inline off
endif

CFLAGS		= -nodefaults		\
		  -align powerpc	\
		  -W all		\
		  -enum int		\
		  -g			\
		  $(DBGFLAG)

ifneq ($(PLATFORM), MAC)

	CFLAGS	+= -proc gekko		\
		  -fp hardware		\
		  -Cpp_exceptions off	\
		  -DEPPC

else

	CFLAGS	+= -proc 750		\
		  -nomapcr		\
		  -proto strict		\
		  -codegen
endif

##########################################################################
# LINKER FLAGS
##########################################################################

# LIBS		library linker flags and libraries
# LDFLAGS	linker flags
# LCF		linker command file

LIBS		= -L$(SDKLIBDIR) $(SDKLIBS) $(MWLIBS) $(AMCLIBS) $(ODELIBS)

ifneq ($(PLATFORM), MAC)

	LDFLAGS	= -unused -nodefaults -fp hardware -g
	LCF	= -lcf $(DOLPHIN_ROOT)/include/dolphin/eppc.$(PLATFORM).lcf

else

	LDFLAGS	= -nodefaults	\
		  -merge off	\
		  -size on	\
		  -xm a		\
		  -sizemax 32768 -sizemin 32768 \
		  -t APPL	\
		  -g		\
		  -macbinary on

	LCF	=

endif

##########################################################################
# HARDWARE FLAGS
##########################################################################

##########################################################################
# HW PROFILE

HW1BUGS = -DBUG_TRIANGLE_FAN  \
          -DBUG_NO_8b_SCALE   \
          -DBUG_INDX_MTX_LOAD \
          -DBUG_MTX_INDX_VTX  \
          -DBUG_DBL_CLR_LOAD  \
          -DBUG_16MB_XFB_LMT  \
          -DBUG_BYPASS_REG    \
          -DBUG_INDX_ALIGN    \
          -DBUG_XF_STALL

HW2BUGS	= -DBUG_CLR_LOAD_DLY  \
          -DBUG_XF_STALL

ifeq ($(PLATFORM), HW1)

	BUGLIST = $(HW1BUGS)
	GX_REV = 1

endif

ifeq ($(PLATFORM), HW2)

	BUGLIST = $(HW2BUGS)
	GX_REV = 2

endif


ifneq ($(PLATFORM), MAC)

	HWCONFIG= -D$(PLATFORM) \
		  -DMARLIN=DI	\
		  -D__HWSIM=0	\
		  -DORCA	\
		  -DGEKKO	\
		  -DGX_REV=$(GX_REV)

else

	HWCONFIG= -D__HWSIM=0	\
		  -DMAC		\
		  -DMACOS	\
		  -DEMU		\
		  -DMACBUGPATCH

endif

##########################################################################
# PROJECT FILES
#
# Change CSRC and BINNAME variable to insert new C files and the result binary
# name. Many demonstration programs under build/demos can use this Makefile to
# build them.
#
##########################################################################

# BINNAME	name of binary without suffix extension
# BIN		list of binaries
# CSRCS		list of C source files
# OBJS		list of object files

BINNAME		= smp-onetri
CSRCS		= smp-onetri.c
OBJS		= $(CSRCS:.c=.o)

ifneq ($(PLATFORM), MAC)

	BIN	= $(addsuffix .elf, $(BINNAME))

else

	BIN	= $(addsuffix .bin, $(BINNAME))

endif

##########################################################################
# PROJECT RULES
##########################################################################

default: $(OBJS)
	@echo
ifneq ($(PLATFORM), MAC)
	@echo Linking $(BIN) for $(PLATFORM) platform
	@echo
	$(LD) $+ $(LDFLAGS) $(LIBS) $(LCF) -o $(BIN)
	@echo
else
	@echo Linking $(PLATFORM) version of $(BIN)
	@echo
	$(LD) $+ $(LDFLAGS) $(LIBS) $(LCF) -o $(BINNAME)
	@echo
endif
	@echo

clean:
	rm -rf $(OBJS) .*.swp *.MAP *.xSYM

clobber: clean
	rm -rf *.elf *.bin

##########################################################################
# COMMON SUFFIX RULES
##########################################################################

.SUFFIXES:
.SUFFIXES: .s .h .c .o .cpp

.c.o:
	@echo
	@echo Compiling $< for $(PLATFORM) platform
	@echo
ifneq ($(PLATFORM), MAC)
	$(CC) $(CFLAGS) $(INCS) $(HWCONFIG) $(BUGLIST) -c $<
else
	$(CC) $(CFLAGS) $(INCS) $(HWCONFIG) $(BUGLIST) $< -o $@
endif
	@echo
	@echo
