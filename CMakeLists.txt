cmake_minimum_required(VERSION 3.22...4.1)

set(APP_NAME CosmicCities)

project(${APP_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

# Exclude optional modding files from the build (no in-game mod API)
list(FILTER GAME_SOURCE EXCLUDE REGEX ".*/Source/utils/(HookManager|ModAPI|ModLoader)\\.cpp")
list(FILTER GAME_HEADER EXCLUDE REGEX ".*/Source/utils/(HookManager|ModAPI|ModLoader)\\.h")

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
CPMAddPackage("gh:nlohmann/json#v3.11.3")
CPMAddPackage("gh:catchorg/Catch2#v3.11.0")
CPMAddPackage(
  NAME minhook
  GITHUB_REPOSITORY TsudaKageyu/minhook
  VERSION 1.3.4
  OPTIONS "BUILD_SHARED_LIBS ON"
)
CPMAddPackage("gh:gabime/spdlog#v1.16.0")
CPMAddPackage(
  NAME sqlite3
  GITHUB_REPOSITORY sqlite/sqlite
  GIT_TAG version-3.51.1
  OPTIONS "BUILD_SHARED_LIBS ON"
)
CPMAddPackage("gh:digint/tinyfsm#v0.3.3")
CPMAddPackage("gh:skypjack/entt#v3.16.0")
CPMAddPackage("gh:g-truc/glm#1.0.2")
CPMAddPackage("gh:ThePhD/sol2#v3.3.0")
CPMAddPackage("gh:icculus/physfs#main")
CPMAddPackage("gh:lz4/lz4#v1.10.0")

# Include directories for all CPM packages
target_include_directories(${PROJECT_NAME} PRIVATE 
  ${better_enums_SOURCE_DIR}
  ${nlohmann_json_SOURCE_DIR}/include
  ${Catch2_SOURCE_DIR}/include
  ${minhook_SOURCE_DIR}/include
  ${spdlog_SOURCE_DIR}/include
  ${tinyfsm_SOURCE_DIR}
  ${entt_SOURCE_DIR}/src
  ${glm_SOURCE_DIR}
  ${sol2_SOURCE_DIR}/include
  ${physfs_SOURCE_DIR}/src
  ${lz4_SOURCE_DIR}/lib
)

target_link_libraries(${PROJECT_NAME} minhook)

# Provide Lua headers/library for sol2 usage
if(TARGET plainlua)
  target_link_libraries(${PROJECT_NAME} plainlua)
else()
  target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/axmol/3rdparty/lua/plainlua)
endif()

# Copy minhook DLL next to the game binary on Windows
if(WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:minhook>"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
  )
endif()

# Default Platform-specific setup
include(AXGamePlatformSetup)

# Make sure the AXGameFinalSetup is included at the end of this file
include(AXGameFinalSetup)