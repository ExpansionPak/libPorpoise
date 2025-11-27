set(PIKMIN_DEMO_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE PIKMIN_DEMO_SOURCES CONFIGURE_DEPENDS
    ${PIKMIN_DEMO_ROOT}/src/*.cpp
    ${PIKMIN_DEMO_ROOT}/src/*.cp
)

set(PIKMIN_DEMO_EXCLUDE_PATTERNS
    "[/\\\\]src[/\\\\]os[/\\\\]"
    "[/\\\\]src[/\\\\]pad[/\\\\]"
    "[/\\\\]src[/\\\\]si[/\\\\]"
    "[/\\\\]src[/\\\\]dsp[/\\\\]"
    "[/\\\\]src[/\\\\]dvd[/\\\\]"
    "[/\\\\]src[/\\\\]card[/\\\\]"
    "[/\\\\]src[/\\\\]gx[/\\\\]"
    "[/\\\\]src[/\\\\]vi[/\\\\]"
    "[/\\\\]src[/\\\\]MSL_C[/\\\\]"
    "[/\\\\]src[/\\\\]TRK_"
    "[/\\\\]src[/\\\\]Runtime[/\\\\]PPCEABI[/\\\\]"
    "[/\\\\]src[/\\\\]amcExi2[/\\\\]"
    "[/\\\\]src[/\\\\]amcnotstub[/\\\\]"
    "[/\\\\]src[/\\\\]ar[/\\\\]"
    "[/\\\\]src[/\\\\]base[/\\\\]"
    "[/\\\\]src[/\\\\]jaudio[/\\\\]"
    "[/\\\\]src[/\\\\]hio[/\\\\]"
    "[/\\\\]src[/\\\\]OdemuExi2[/\\\\]"
    "[/\\\\]src[/\\\\]odenotstub[/\\\\]"
    "[/\\\\]src[/\\\\]mtx[/\\\\]"
)

set(PIKMIN_DEMO_FILTERED_SOURCES)
foreach(source IN LISTS PIKMIN_DEMO_SOURCES)
    set(skip_source FALSE)
    foreach(pattern IN LISTS PIKMIN_DEMO_EXCLUDE_PATTERNS)
        if("${source}" MATCHES "${pattern}")
            set(skip_source TRUE)
            break()
        endif()
    endforeach()
    if(NOT skip_source)
        list(APPEND PIKMIN_DEMO_FILTERED_SOURCES "${source}")
    endif()
endforeach()
set(PIKMIN_DEMO_SOURCES "${PIKMIN_DEMO_FILTERED_SOURCES}")

if(NOT PIKMIN_DEMO_SOURCES)
    message(FATAL_ERROR "No sources found for Pikmin demo")
endif()

set(PIKMIN_DEMO_CP_SOURCES)
foreach(source IN LISTS PIKMIN_DEMO_SOURCES)
    if(source MATCHES "\\.cp$")
        list(APPEND PIKMIN_DEMO_CP_SOURCES "${source}")
    endif()
endforeach()

if(PIKMIN_DEMO_CP_SOURCES)
    set_source_files_properties(${PIKMIN_DEMO_CP_SOURCES} PROPERTIES LANGUAGE CXX)
endif()

add_executable(pikmin_demo ${PIKMIN_DEMO_SOURCES})

target_include_directories(pikmin_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${PIKMIN_DEMO_ROOT}/include
)

target_compile_definitions(pikmin_demo PRIVATE TARGET_PC=1)
target_compile_features(pikmin_demo PRIVATE cxx_std_20)
if(MSVC)
    target_compile_options(pikmin_demo PRIVATE /Zc:strictStrings-)
endif()

target_link_libraries(pikmin_demo PRIVATE porpoise)


