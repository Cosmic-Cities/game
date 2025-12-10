cmake_minimum_required(VERSION 3.22...4.1)

set(APP_NAME CosmicCities)

project(${APP_NAME})

if(XCODE)
  set(CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY TRUE)
endif()

set(_is_axmol_embed FALSE)
set(_AX_USE_PREBUILT FALSE)

set(CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
)

# Disable pthreads for WASM (required for platforms without COOP/COEP headers like Newgrounds)
if(EMSCRIPTEN)
  set(AX_WASM_THREADS "0" CACHE STRING "Disable pthreads for WASM" FORCE)
endif()

# NOTE: The order of the cmake module "include(AXGame...)" statements matters
include(AXGameEngineOptions)
include(AXGameEngineSetup)

# The common cross-platforms source files and header files
file(GLOB_RECURSE GAME_HEADER
  CONFIGURE_DEPENDS
  Source/*.h Source/*.hpp
)

file(GLOB_RECURSE GAME_SOURCE
  CONFIGURE_DEPENDS
  Source/*.cpp Source/*.c
)

set(GAME_INC_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

set(content_folder
  "${CMAKE_CURRENT_SOURCE_DIR}/Content"
)

if(APPLE)
  ax_mark_multi_resources(common_content_files RES_TO "Resources" FOLDERS ${content_folder})
elseif(WINDOWS)
  ax_mark_multi_resources(common_content_files RES_TO "Content" FOLDERS ${content_folder})
endif()

include(AXGameSourceSetup)

# mark app complie info and libs info
set(APP_SOURCES
  ${GAME_HEADER}
  ${GAME_SOURCE}
)

include(AXGameTargetSetup)

# mark app resources, resource will be copy auto after mark
ax_setup_app_config(${APP_NAME})

# CPM dependency management
include(CPM)

CPMAddPackage("gh:aantron/better-enums#0.11.3")
CPMAddPackage("gh:nlohmann/json#v3.12.0")
CPMAddPackage("gh:catchorg/Catch2#v3.11.0")
CPMAddPackage("gh:TsudaKageyu/minhook#v1.3.4")

# Include directories for all CPM packages
target_include_directories(${PROJECT_NAME} PRIVATE 
  ${better_enums_SOURCE_DIR}
  ${nlohmann_json_SOURCE_DIR}/include
  ${Catch2_SOURCE_DIR}/include
  ${minhook_SOURCE_DIR}/include
)

# Default Platform-specific setup
include(AXGamePlatformSetup)

# Make sure the AXGameFinalSetup is included at the end of this file
include(AXGameFinalSetup)