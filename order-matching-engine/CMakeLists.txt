cmake_minimum_required(VERSION 3.10)

# Taken from UIUC CS 225 course

# Project Information (these are the *only* things you need to change here between projects)
set(project_name "Order Book Generation") # Name of the project
set(project_version 1.0.0) # Version, Large Update, Small bug fixes
set(project_entrypoints "main" "dll_main" "unordered_dense_main" "orderbook_main" "timer_main" "byte_swap" "spscqueue_main" "ome_main" "tcp_server" "tcp_client" "multicast_sender" "multicast_receiver") # Entrypoints to run the program
set(project_clean_rm "") # Generated files that should be removed with "make clean"
# set(assignment_container "fa22") # Container we are targetting


#######################################################################################
# Add color support to our messages.
# See: https://stackoverflow.com/questions/18968979/how-to-get-colorized-output-with-cmake
if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColorReset   "${Esc}[m")
  set(ColorBold    "${Esc}[1m")
  set(ColorRed     "${Esc}[31m")
  set(ColorGreen   "${Esc}[32m")
  set(ColorYellow  "${Esc}[33m")
  set(ColorBlue    "${Esc}[34m")
  set(ColorMagenta "${Esc}[35m")
  set(ColorCyan    "${Esc}[36m")
  set(ColorWhite   "${Esc}[37m")
endif()

# Check to see if using vagrant
site_name(SITE_NAME)
if(SITE_NAME MATCHES "vagrant")
    set(VAGRANT TRUE)
    message(STATUS "${ColorGreen}You are using vagrant.${ColorReset}")
else()
    message(STATUS "${ColorYellow}You are not using vagrant.${ColorReset}")
endif()

# Set paths to C and CXX compilers
if (NOT DEFINED VAGRANT)
    set(CMAKE_C_COMPILER "/usr/bin/gcc")
    set(CMAKE_CXX_COMPILER "/usr/bin/g++")
    set(CMAKE_LINKER "/usr/bin/g++")
endif()

# Set C++20 and required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Path definitions
set(lib_dir ${CMAKE_SOURCE_DIR}/lib)
set(src_dir ${CMAKE_SOURCE_DIR}/src)
set(tests_dir ${CMAKE_SOURCE_DIR}/tests)
set(main_dir ${CMAKE_SOURCE_DIR}/main)
set(include_dir ${CMAKE_SOURCE_DIR}/include)

# Put output files in build/ directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Define project
project(${project_name} VERSION ${project_version})

# Specify Debug build.
set(CMAKE_BUILD_TYPE Debug)

# Specify warnings for all builds.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Werror -Wfatal-errors -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function")

# Specify debug symbols and no optimizations for Debug builds.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -O0 -gdwarf-4")

# Specify additional clean files.
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${project_clean_rm}")

# Add Catch2.
# Note: Ubuntu 20.04 LTS does not have Catch2 on apt
# See: https://github.com/catchorg/Catch2/issues/1383
if(DEFINED VAGRANT)
    find_package(Catch2 REQUIRED)
    find_package(Boost REQUIRED)
else()
    include(FetchContent)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.1.0
    )

    FetchContent_MakeAvailable(Catch2)

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif()


# Run CMakeLists in other directories
add_subdirectory(${lib_dir})
add_subdirectory(${include_dir})
add_subdirectory(${src_dir})
add_subdirectory(${tests_dir})

add_library(ExchangeOME::ExchangeOME ALIAS srclib)

# Add the project entrypoints using our libs and src code.
foreach(entrypoint IN LISTS project_entrypoints)
    add_executable(${entrypoint} ${main_dir}/${entrypoint}.cpp)
    target_link_libraries(${entrypoint} PRIVATE srclib)
endforeach()
