# Example programs for libPorpoise

# Simple example
add_executable(simple_example simple.c)
target_link_libraries(simple_example porpoise)
target_include_directories(simple_example PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Thread example
add_executable(thread_example thread_test.c)
target_link_libraries(thread_example porpoise)
target_include_directories(thread_example PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Locked cache example (demonstrates full memory emulation)
add_executable(locked_cache_example locked_cache_example.c)
target_link_libraries(locked_cache_example porpoise)

add_executable(reset_button_example reset_button_example.c)
target_link_libraries(reset_button_example porpoise)
target_include_directories(reset_button_example PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_include_directories(locked_cache_example PRIVATE ${CMAKE_SOURCE_DIR}/include)

# PAD (controller) example
add_executable(pad_test pad_test.c)
target_link_libraries(pad_test porpoise)
target_include_directories(pad_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

# DVD (file system) example
add_executable(dvd_test dvd_test.c)
target_link_libraries(dvd_test porpoise)
target_include_directories(dvd_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

# ARAM example
add_executable(aram_test aram_test.c)
target_link_libraries(aram_test porpoise)
target_include_directories(aram_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

# VI (video interface) example
add_executable(vi_test vi_test.c)
target_link_libraries(vi_test porpoise)
target_include_directories(vi_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

# CARD (memory card) example
add_executable(card_test card_test.c)
target_link_libraries(card_test porpoise)
target_include_directories(card_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

# onetri_test - GX initialization test
add_executable(onetri_test onetri_test.c)
target_link_libraries(onetri_test porpoise)
target_include_directories(onetri_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

if(WIN32 AND MSVC)
    target_link_options(onetri_test PRIVATE "/SUBSYSTEM:CONSOLE")
endif()

# Add SDL2 NuGet package reference for Visual Studio projects
if(WIN32 AND MSVC)
    set_target_properties(onetri_test PROPERTIES
        VS_PACKAGE_REFERENCES "SDL2;SDL2.redist"
    )
endif()

# gx_demo - GX graphics demo (based on Aurora's simple.c)
add_executable(gx_demo gx_demo.c)
target_link_libraries(gx_demo porpoise)
target_include_directories(gx_demo PRIVATE ${CMAKE_SOURCE_DIR}/include)

if(WIN32 AND MSVC)
    target_link_options(gx_demo PRIVATE "/SUBSYSTEM:CONSOLE")
    set_target_properties(gx_demo PROPERTIES
        VS_PACKAGE_REFERENCES "SDL2;SDL2.redist"
    )
endif()

# onetri_demo - Official GameCube SDK onetri demo
add_executable(onetri_demo onetri_demo.c)
target_link_libraries(onetri_demo PRIVATE porpoise)
target_include_directories(onetri_demo PRIVATE ${CMAKE_SOURCE_DIR}/include)

if(WIN32 AND MSVC)
    # Force console subsystem (not Windows subsystem)
    set_target_properties(onetri_demo PROPERTIES
        WIN32_EXECUTABLE FALSE
        VS_PACKAGE_REFERENCES "SDL2;SDL2.redist"
    )
    target_link_options(onetri_demo PRIVATE "/SUBSYSTEM:CONSOLE")
endif()

# Link SDL2 if found (porpoise should already link it, but ensure it's available)
if(SDL2_FOUND)
    target_link_libraries(onetri_demo PRIVATE SDL2::SDL2)
endif()

