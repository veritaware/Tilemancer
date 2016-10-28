file(GLOB app_src_glob *.cpp;*.cc;*.h;*.inl)

set(src_lua
    ../external/lua/src/lapi.c
    ../external/lua/src/lauxlib.c
    ../external/lua/src/lbaselib.c
    ../external/lua/src/lbitlib.c
    ../external/lua/src/lcode.c
    ../external/lua/src/lcorolib.c
    ../external/lua/src/lctype.c
    ../external/lua/src/ldblib.c
    ../external/lua/src/ldebug.c
    ../external/lua/src/ldo.c
    ../external/lua/src/ldump.c
    ../external/lua/src/lfunc.c
    ../external/lua/src/lgc.c
    ../external/lua/src/linit.c
    ../external/lua/src/liolib.c
    ../external/lua/src/llex.c
    ../external/lua/src/lmathlib.c
    ../external/lua/src/lmem.c
    ../external/lua/src/loadlib.c
    ../external/lua/src/lobject.c
    ../external/lua/src/lopcodes.c
    ../external/lua/src/loslib.c
    ../external/lua/src/lparser.c
    ../external/lua/src/lstate.c
    ../external/lua/src/lstring.c
    ../external/lua/src/lstrlib.c
    ../external/lua/src/ltable.c
    ../external/lua/src/ltablib.c
    ../external/lua/src/ltm.c
    ../external/lua/src/lundump.c
    ../external/lua/src/lutf8lib.c
    ../external/lua/src/lvm.c
    ../external/lua/src/lzio.c
    )
source_group("lua" FILES ${src_lua})
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/external/lua/src/)

find_package(wxWidgets COMPONENTS core base gl)
include( "${wxWidgets_USE_FILE}" )

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

include_directories(../external/glm/include)

set(app_src ${app_src_glob})

source_group("" FILES ${app_src})

set(all_src ${src_lua} ${app_src})

add_executable(tilemancer WIN32 MACOSX_BUNDLE ${all_src})
set_target_properties(tilemancer
                      PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/dist"
                      DEBUG_OUTPUT_NAME "tilemancer-debug"
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/dist"
                      RELEASE_OUTPUT_NAME "tilemancer"
                      )

target_link_libraries(tilemancer
                      ${OPENGL_LIBRARIES}
                      ${wxWidgets_LIBRARIES}
                      ${SDL2_LIBRARY}
                      SDL2_image # we are using absolute reference to sdl2 image, but that's oke. the plan is to remove sdl and use wx or some other image library
                      )
target_link_libraries(tilemancer ${CMAKE_DL_LIBS})
target_link_libraries(tilemancer ${CMAKE_THREAD_LIBS_INIT})
