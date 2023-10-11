


# CMake Git submodule integration for auto-recurse when cloning
# https://cliutils.gitlab.io/modern-cmake/chapters/projects/submodule.html
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
  option(GIT_SUBMODULE "Check submodules during build" ON)
  if(GIT_SUBMODULE)
    message(STATUS "Submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
  endif()
endif()

# Add library target
add_library(libs "empty.cpp")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/unordered_dense/CMakeLists.txt")
  message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
else()
# Add additional libraries here as you see fit
  # Add unordered_dense submodule CMake
  add_subdirectory(unordered_dense)
  add_subdirectory(SPSCQueue)
  # find_package(unordered_dense CONFIG REQUIRED)
  # include_directories("${CMAKE_SOURCE_DIR}/unordered_dense/include")
  target_link_libraries(libs PUBLIC unordered_dense::unordered_dense SPSCQueue::SPSCQueue)
endif()