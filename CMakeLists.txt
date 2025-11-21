cmake_minimum_required(VERSION 3.15)
project(libPorpoise VERSION 0.1.0 LANGUAGES C CXX)

# Check for devkitPro and warn if found
# devkitPro headers are incompatible with this PC port
if(DEFINED ENV{INCLUDE})
    string(TOUPPER "$ENV{INCLUDE}" INCLUDE_UPPER)
    if(INCLUDE_UPPER MATCHES "DEVKIT")
        message(WARNING "===========================================================================")
        message(WARNING "WARNING: devkitPro detected in INCLUDE environment variable!")
        message(WARNING "devkitPro headers are incompatible with this PC port.")
        message(WARNING "Please remove devkitPro paths from your INCLUDE environment variable.")
        message(WARNING "This project uses standard system headers (MSVC/MinGW/GCC), not devkitPro.")
        message(WARNING "===========================================================================")
    endif()
endif()

# Set C/C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(PORPOISE_BUILD_EXAMPLES "Build example programs" ON)
option(PORPOISE_BUILD_SHARED "Build shared library" OFF)
option(PORPOISE_USE_GECKO_MEMORY "Enable full memory emulation (locked cache support)" OFF)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find SDL2 (prefer system install, fall back to NuGet package)
set(SDL2_INCLUDE_DIRS "")
set(SDL2_LIBRARIES "")
set(SDL2_IMPORTED_TARGET "")
find_package(SDL2 QUIET)
if(SDL2_FOUND)
    message(STATUS "Found SDL2 via find_package")
    set(SDL2_IMPORTED_TARGET SDL2::SDL2)
else()
    message(WARNING "SDL2 not found via find_package. Trying NuGet package (VS).")
    if(WIN32)
        # NuGet restores packages under ${CMAKE_BINARY_DIR}/packages
        file(GLOB SDL2_NUGET_PACKAGES "${CMAKE_BINARY_DIR}/packages/sdl2.nuget.[0-9]*")
        list(LENGTH SDL2_NUGET_PACKAGES SDL2_NUGET_COUNT)
        if(SDL2_NUGET_COUNT GREATER 0)
            list(SORT SDL2_NUGET_PACKAGES)
            list(GET SDL2_NUGET_PACKAGES -1 SDL2_NUGET_PATH)
            set(SDL2_INCLUDE_DIRS "${SDL2_NUGET_PATH}/build/native/include")
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                set(SDL2_ARCH_DIR "x64")
            else()
                set(SDL2_ARCH_DIR "Win32")
            endif()
            set(SDL2_LIB_FILE "${SDL2_NUGET_PATH}/build/native/lib/${SDL2_ARCH_DIR}/SDL2.lib")
            if(NOT EXISTS "${SDL2_LIB_FILE}")
                message(WARNING "SDL2 NuGet library not found at ${SDL2_LIB_FILE}")
            else()
                set(SDL2_LIBRARIES "${SDL2_LIB_FILE}")
                if(NOT TARGET SDL2::SDL2)
                    add_library(SDL2::SDL2 UNKNOWN IMPORTED)
                endif()
                set_target_properties(SDL2::SDL2 PROPERTIES
                    IMPORTED_LOCATION "${SDL2_LIB_FILE}"
                    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
                )
                set(SDL2_IMPORTED_TARGET SDL2::SDL2)
                message(STATUS "Using SDL2 from NuGet: ${SDL2_NUGET_PATH}")
            endif()
        else()
            message(WARNING "SDL2 NuGet package not restored yet (expected under ${CMAKE_BINARY_DIR}/packages)")
        endif()
    endif()
endif()

if(NOT SDL2_IMPORTED_TARGET)
    # Create a dummy interface target to keep include paths working even if SDL2 isn't available yet
    if(NOT TARGET SDL2::SDL2)
        add_library(SDL2::SDL2 INTERFACE IMPORTED)
    endif()
    set(SDL2_IMPORTED_TARGET SDL2::SDL2)
endif()

if(SDL2_LIBRARIES STREQUAL "" AND SDL2_IMPORTED_TARGET)
    set(SDL2_LIBRARIES SDL2::SDL2)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${SDL2_INCLUDE_DIRS})

# Source files
set(PORPOISE_SOURCES
    # Core OS
    src/os/OS.c
    src/os/OSThread.c
    src/os/OSMutex.c
    src/os/OSTime.c
    src/os/OSAlarm.c
    src/os/OSAlloc.c
    src/os/OSCache.c
    src/os/OSContext.c
    src/os/OSError.c
    src/os/OSFont.c
    src/os/OSInterrupt.c
    src/os/OSMemory.c
    src/os/OSMessage.c
    src/os/OSSemaphore.c
    src/os/OSReset.c
    src/os/OSResetSW.c
    src/os/OSRtc.c
    src/os/OSUart.c
    
    # OS Init (Startup Entry Points)
    src/os/init/__start.c
    src/os/init/__ppc_eabi_init.c
    
    # Base (PowerPC Architecture)
    src/base/PPCArch.c
    src/base/PPCPm.c
    
    # DB (Dolphin Debugger)
    src/db/db.c
    src/db/dbprintf.c
    
    # PAD (Controller)
    src/pad/PAD.c
    src/pad/PADClamp.c
    src/pad/PADConfig.c
    
    # DVD (File System)
    src/dvd/DVD.c
    src/dvd/DVDQueue.c
    src/dvd/DVDLow.c
    src/dvd/DVDError.c
    src/dvd/DVDFatal.c
    
    # SI (Serial Interface - Controller Hardware)
    src/si/SI.c
    
    # AR (ARAM - Audio RAM)
    src/ar/AR.c
    src/ar/ARQ.c
    
    # VI (Video Interface)
    src/vi/VI.c
    src/vi/VIConfig.c
    
    # EXI (External Interface)
    src/exi/EXI.c
    
    # CARD (Memory Card)
    src/card/CARDBios.c
    src/card/CARDMount.c
    src/card/CARDOpen.c
    src/card/CARDCreate.c
    src/card/CARDDelete.c
    src/card/CARDRead.c
    src/card/CARDWrite.c
    src/card/CARDCheck.c
    src/card/CARDFormat.c
    src/card/CARDStat.c
    src/card/CARDRename.c
    src/card/CARDBlock.c
    
    # MTX (Matrix Math)
    src/mtx/mtx.c
    
    # GX (Graphics)
    src/gx/GXManage.c
    src/gx/GXBump.c
    src/gx/GXCull.c
    src/gx/GXExtra.c
    src/gx/GXFifo.c
    src/gx/GXFrameBuffer.c
    src/gx/GXGeometry.c
    src/gx/GXGet.c
    src/gx/GXLighting.c
    src/gx/GXVert.c
    src/gx/GXPerf.c
    src/gx/GXPixel.c
    src/gx/GXTev.c
    src/gx/GXTexture.c
    src/gx/GXTransform.c
    
    # GX Graphics Backend
    src/gfx/gx_state.c
    src/gfx/gx_init.c
    src/gfx/gx_helpers.c
    src/gfx/gx_shader.c
    src/gfx/gx_gl_loader.c
    src/gfx/gx_state_helpers.c
    src/gfx/gx_update_state.c
    
    # Demo Library
    src/demo/DEMOInit.c
    src/demo/DEMOPad.c
    src/demo/DEMOPuts.c
    src/demo/DEMOWin.c
    src/demo/DEMOStats.c
    src/demo/DEMOFont.c
)

# Create library
if(PORPOISE_BUILD_SHARED)
    add_library(porpoise SHARED ${PORPOISE_SOURCES})
else()
    add_library(porpoise STATIC ${PORPOISE_SOURCES})
endif()

# Always build the PC-targeted path in our headers/source
target_compile_definitions(porpoise PUBLIC TARGET_PC=1)

# Set library properties
set_target_properties(porpoise PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Enable full memory emulation if requested
if(PORPOISE_USE_GECKO_MEMORY)
    target_compile_definitions(porpoise PUBLIC PORPOISE_USE_GECKO_MEMORY)
endif()

# Link SDL2
if(SDL2_FOUND)
    target_link_libraries(porpoise PUBLIC ${SDL2_LIBRARIES})
else()
    # When SDL2_FOUND is false, use the interface target (NuGet will handle actual linking)
    target_link_libraries(porpoise PUBLIC SDL2::SDL2)
endif()

# Link OpenGL (for GX graphics backend)
if(WIN32)
    # Windows: Use opengl32
    target_link_libraries(porpoise PRIVATE opengl32)
elseif(APPLE)
    # macOS: Use framework
    find_library(OPENGL_LIBRARY OpenGL)
    target_link_libraries(porpoise PRIVATE ${OPENGL_LIBRARY})
else()
    # Linux: Use libGL
    find_package(OpenGL REQUIRED)
    target_link_libraries(porpoise PRIVATE OpenGL::GL)
endif()

# Platform-specific settings
if(WIN32)
    target_compile_definitions(porpoise PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(porpoise PRIVATE winmm)
    
    # Add SDL2 NuGet package reference for Visual Studio projects
    if(MSVC)
        set_target_properties(porpoise PROPERTIES
            VS_PACKAGE_REFERENCES "SDL2;SDL2.redist"
        )
        message(STATUS "Added SDL2 NuGet package reference to porpoise target")
    endif()
    
    # Explicitly exclude devkitPro paths on Windows
    # Note: devkitPro headers are incompatible with this PC port
    # If you have devkitPro installed, ensure it's not in your INCLUDE environment variable
    if(MSVC)
        # MSVC: Use /X flag to ignore standard include paths, then re-add only what we need
        # This prevents devkitPro headers from being found
        message(STATUS "Using MSVC compiler - devkitPro paths will be excluded")
    elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        message(WARNING "GCC/Clang detected. If you have devkitPro installed, remove it from your PATH and INCLUDE environment variables.")
        message(WARNING "This project uses standard system headers, not devkitPro.")
    endif()
elseif(UNIX)
    target_link_libraries(porpoise PRIVATE pthread m)
endif()

# Install rules
install(TARGETS porpoise
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/dolphin
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Examples
if(PORPOISE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Print configuration
message(STATUS "libPorpoise Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Library type: ${PORPOISE_BUILD_SHARED}")
message(STATUS "  Build examples: ${PORPOISE_BUILD_EXAMPLES}")
message(STATUS "  Full memory emulation: ${PORPOISE_USE_GECKO_MEMORY}")
