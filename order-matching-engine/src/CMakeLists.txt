# Path definitions.
set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Add overall src library.
# file(GLOB_RECURSE src_sources CONFIGURE_DEPENDS ${src_dir}/*.hpp ${src_dir}/*.cxx)

set(DLL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/doublylinkedlist")
set(STRUCT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/struct")
set(OME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/order_matching_engine")
set(OB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/orderbook")
set(PAR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/parser")

set(DLL_SRCS "${DLL_DIR}/dll.h"
             "${DLL_DIR}/dll.hpp"
             "${DLL_DIR}/dll_iterator.hpp")
set(STRUCT_SRCS "${STRUCT_DIR}/structs.h")
set(OME_SRCS "${OME_DIR}/order_matching_engine.h"
             "${OME_DIR}/order_matching_engine.cpp"
             "${OME_DIR}/machine_time.h"
             "${OME_DIR}/machine_time.cpp"
             "${OME_DIR}/util.h"
             "${OME_DIR}/util.cpp")
set(OB_SRCS "${OB_DIR}/orderbook.h"
            "${OB_DIR}/orderbook.cpp"
            "${OB_DIR}/orderbook_print.cpp")
set(PAR_SRCS "${PAR_DIR}/parser.h"
             "${PAR_DIR}/parser.cpp")

add_library(
  # Sets the name of the library
  srclib 

  # Sets the library as public
  SHARED 

  # Provides a relative path to your source file(s)
  ${DLL_SRCS}
  ${STRUCT_SRCS}
  ${OME_SRCS}
  ${OB_SRCS}
  ${PAR_SRCS}
  "${DLL_DIR}/util_ptr.h"
)

target_include_directories(srclib PUBLIC ${src_dir})
set_target_properties(srclib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(srclib PUBLIC libs)