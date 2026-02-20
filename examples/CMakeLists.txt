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

# GX onetri demo
add_executable(onetri_demo onetri_demo.c)
target_link_libraries(onetri_demo porpoise)
target_include_directories(onetri_demo PRIVATE ${CMAKE_SOURCE_DIR}/include)

# GX octa demo (rotating octahedron from gxdemo)
add_executable(octa_demo ${CMAKE_SOURCE_DIR}/demos/gxdemo/src/Simple/smp-octa.c)
target_link_libraries(octa_demo porpoise)
target_include_directories(octa_demo PRIVATE ${CMAKE_SOURCE_DIR}/include)

# GX mgt-single-buf demo (rotating octahedron, single-buffer framebuffer test)
add_executable(mgt_single_buf_demo ${CMAKE_SOURCE_DIR}/demos/gxdemo/src/Management/mgt-single-buf.c)
target_link_libraries(mgt_single_buf_demo porpoise)
target_include_directories(mgt_single_buf_demo PRIVATE ${CMAKE_SOURCE_DIR}/include)

# GX onetri display list demo (struts via GXCallDisplayList)
add_executable(onetri_dl_demo ${CMAKE_SOURCE_DIR}/demos/gxdemo/src/Simple/smp-onetri_dl.c)
target_link_libraries(onetri_dl_demo porpoise)
target_include_directories(onetri_dl_demo PRIVATE ${CMAKE_SOURCE_DIR}/include)

# GX culling/viewport demo (uses GXSetScissor for draggable clip window)
add_executable(cul_viewport_demo
  ${CMAKE_SOURCE_DIR}/demos/gxdemo/src/Culling/cul-viewport.c
  ${CMAKE_SOURCE_DIR}/demos/gxdemo/src/PixelProc/cmn-model.c
)
target_link_libraries(cul_viewport_demo porpoise)
target_include_directories(cul_viewport_demo PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/demos/gxdemo/include)

# GX framebuffer full-frame AA demo (uses GXSetScissor for interlaced half-screen)
add_executable(frb_aa_full_demo ${CMAKE_SOURCE_DIR}/demos/gxdemo/src/Framebuffer/frb-aa-full.c)
target_link_libraries(frb_aa_full_demo porpoise)
target_include_directories(frb_aa_full_demo PRIVATE ${CMAKE_SOURCE_DIR}/include)

