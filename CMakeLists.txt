cmake_minimum_required(VERSION 3.0)
project(visit_test CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)

FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG master)
FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
endif()

FetchContent_Declare(visit_struct
  GIT_REPOSITORY https://github.com/cbeck88/visit_struct.git
  GIT_TAG v1.0
  CONFIGURE_COMMAND ""
  BUILD_COMMAND "")
FetchContent_GetProperties(visit_struct)
if(NOT visit_struct_POPULATED)
  FetchContent_Populate(visit_struct)
endif()

# Adding our source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp") 
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/src/") 

# Declaring our executable
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE} ${visit_struct_SOURCE_DIR}/include/ ${json_SOURCE_DIR}/single_include/)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable)
