cmake_minimum_required(VERSION 3.22...4.1)

set(APP_NAME CosmicCities)

project(${APP_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions(FMT_HEADER_ONLY=1)

if(XCODE)
  set(CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY TRUE)
endif()

set(_is_axmol_embed FALSE)
set(_AX_USE_PREBUILT FALSE)

set(CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
)

if(EMSCRIPTEN)
  set(AX_WASM_THREADS "0" CACHE STRING "Disable pthreads for WASM" FORCE)
endif()

include(AXGameEngineOptions)
include(AXGameEngineSetup)

file(GLOB_RECURSE GAME_HEADER
  CONFIGURE_DEPENDS
  Source/*.h Source/*.hpp
)

file(GLOB_RECURSE GAME_SOURCE
  CONFIGURE_DEPENDS
  Source/*.cpp Source/*.c
)

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

set(APP_SOURCES
  ${GAME_HEADER}
  ${GAME_SOURCE}
)

include(AXGameTargetSetup)

ax_setup_app_config(${APP_NAME})

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
CPMAddPackage("gh:OmgRod/sol2#develop")
CPMAddPackage("gh:icculus/physfs#main")
CPMAddPackage("gh:lz4/lz4#v1.10.0")

option(ENABLE_DISCORD "Enable Discord Rich Presence integration" OFF)

if(NOT ENABLE_DISCORD)
  if(EXISTS "${CMAKE_SOURCE_DIR}/lib/discord_social_sdk")
    set(ENABLE_DISCORD ON)
    message(STATUS "Discord SDK detected at lib/discord_social_sdk; enabling Discord support.")
  endif()
endif()

if(ENABLE_DISCORD)
  if(NOT DEFINED DISCORD_SDK_ROOT)
    if(EXISTS "${CMAKE_SOURCE_DIR}/lib/discord_social_sdk")
      set(DISCORD_SDK_ROOT "${CMAKE_SOURCE_DIR}/lib/discord_social_sdk")
    endif()
  endif()
  if(DEFINED DISCORD_SDK_ROOT)
    set(DISCORD_SDK_LIB_DIR "${DISCORD_SDK_ROOT}/lib/release")
    set(DISCORD_SDK_BIN_DIR "${DISCORD_SDK_ROOT}/bin/release")
    set(DISCORD_SDK_INCLUDE_DIR "${DISCORD_SDK_ROOT}/include")
    
    if(WIN32)
      set(DISCORD_LIB_PATH "${DISCORD_SDK_LIB_DIR}/discord_partner_sdk.lib")
      set(DISCORD_SHARED_LIB "${DISCORD_SDK_BIN_DIR}/discord_partner_sdk.dll")
    elseif(APPLE)
      set(DISCORD_LIB_PATH "${DISCORD_SDK_LIB_DIR}/libdiscord_partner_sdk.dylib")
      set(DISCORD_SHARED_LIB "${DISCORD_SDK_LIB_DIR}/libdiscord_partner_sdk.dylib")
    else()
      set(DISCORD_LIB_PATH "${DISCORD_SDK_LIB_DIR}/libdiscord_partner_sdk.so")
      set(DISCORD_SHARED_LIB "${DISCORD_SDK_LIB_DIR}/libdiscord_partner_sdk.so")
    endif()
    
    message(STATUS "Discord SDK found at: ${DISCORD_SDK_ROOT}")
  else()
    message(WARNING "ENABLE_DISCORD is ON but DISCORD_SDK_ROOT not defined and lib/discord_social_sdk not found. Discord support disabled.")
    set(ENABLE_DISCORD OFF)
  endif()
endif()

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

if(ENABLE_DISCORD)
  target_include_directories(${PROJECT_NAME} PRIVATE ${DISCORD_SDK_INCLUDE_DIR})
  target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_DISCORD)
endif()

target_link_libraries(${PROJECT_NAME} minhook)

if(ENABLE_DISCORD)
  target_link_libraries(${PROJECT_NAME} ${DISCORD_LIB_PATH})
  message(STATUS "Linking Discord SDK lib: ${DISCORD_LIB_PATH}")
endif()

if(TARGET plainlua)
  target_link_libraries(${PROJECT_NAME} plainlua)
else()
  target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/axmol/3rdparty/lua/plainlua)
endif()

if(WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:minhook>"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
  )
  
  if(ENABLE_DISCORD)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DISCORD_SHARED_LIB}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
    )
  endif()
endif()

include(AXGamePlatformSetup)

include(AXGameFinalSetup)