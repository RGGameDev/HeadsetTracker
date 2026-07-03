cmake_minimum_required(VERSION 3.16)
project(HeadsetTracker)

set(CMAKE_CXX_STANDARD 20)

add_executable(HeadsetTracker main.cpp)

target_include_directories(HeadsetTracker PRIVATE include)

add_library(SDL3 SHARED IMPORTED)
set_target_properties(SDL3 PROPERTIES
	IMPORTED_LOCATION_DEBUG ${CMAKE_SOURCE_DIR}/lib/sdl/debug/libSDL3.so.0.5.0
	IMPORTED_LOCATION_RELEASE ${CMAKE_SOURCE_DIR}/lib/sdl/release/libSDL3.so.0.5.0
	IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/sdl/release/libSDL3.so.0.5.0
)

add_library(SDL3_gfx SHARED IMPORTED)
set_target_properties(SDL3_gfx PROPERTIES
	IMPORTED_LOCATION_DEBUG ${CMAKE_SOURCE_DIR}/lib/sdl_gfx/debug/libSDL3_gfx.so.1.0.1
	IMPORTED_LOCATION_RELEASE ${CMAKE_SOURCE_DIR}/lib/sdl_gfx/release/libSDL3_gfx.so.1.0.1
	IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/sdl_gfx/release/libSDL3_gfx.so.1.0.1
)

target_link_libraries(HeadsetTracker PRIVATE SDL3 SDL3_gfx)

add_custom_command(TARGET HeadsetTracker POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_SOURCE_DIR}/lib/sdl/$<LOWER_CASE:$<CONFIG>>/libSDL3.so
		${CMAKE_SOURCE_DIR}/lib/sdl/$<LOWER_CASE:$<CONFIG>>/libSDL3.so.0
		${CMAKE_SOURCE_DIR}/lib/sdl/$<LOWER_CASE:$<CONFIG>>/libSDL3.so.0.5.0
		${CMAKE_SOURCE_DIR}/lib/sdl_gfx/$<LOWER_CASE:$<CONFIG>>/libSDL3_gfx.so
		${CMAKE_SOURCE_DIR}/lib/sdl_gfx/$<LOWER_CASE:$<CONFIG>>/libSDL3_gfx.so.1
		${CMAKE_SOURCE_DIR}/lib/sdl_gfx/$<LOWER_CASE:$<CONFIG>>/libSDL3_gfx.so.1.0.1
		$<TARGET_FILE_DIR:HeadsetTracker>
)

install(TARGETS HeadsetTracker DESTINATION .)
install(FILES
	${CMAKE_SOURCE_DIR}/lib/sdl/release/libSDL3.so
	${CMAKE_SOURCE_DIR}/lib/sdl/release/libSDL3.so.0
	${CMAKE_SOURCE_DIR}/lib/sdl/release/libSDL3.so.0.5.0
        ${CMAKE_SOURCE_DIR}/lib/sdl_gfx/release/libSDL3_gfx.so
	${CMAKE_SOURCE_DIR}/lib/sdl_gfx/release/libSDL3_gfx.so.1
	${CMAKE_SOURCE_DIR}/lib/sdl_gfx/release/libSDL3_gfx.so.1.0.1
	DESTINATION .
)

set_target_properties(HeadsetTracker PROPERTIES
	INSTALL_RPATH "$ORIGIN"
)
