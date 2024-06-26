find_package(LibR REQUIRED)
message(STATUS "LIBR_CORE_LIBRARY " ${LIBR_CORE_LIBRARY})

file(
  GLOB_RECURSE R_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/src/*.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/*.c
)

# Use object library to expose symbols
add_library(xgboost-r OBJECT ${R_SOURCES})

if(ENABLE_ALL_WARNINGS)
  target_compile_options(xgboost-r PRIVATE -Wall -Wextra)
endif()

if(MSVC)
  # https://github.com/microsoft/LightGBM/pull/6061
  # MSVC doesn't work with anonymous types in structs. (R complex)
  #
  # syntax error: missing ';' before identifier 'private_data_c'
  #
  target_compile_definitions(xgboost-r PRIVATE -DR_LEGACY_RCOMPLEX)
endif()

target_compile_definitions(
  xgboost-r PUBLIC
  -DXGBOOST_STRICT_R_MODE=1
  -DDMLC_LOG_BEFORE_THROW=0
  -DDMLC_DISABLE_STDIN=1
  -DDMLC_LOG_CUSTOMIZE=1
)

target_include_directories(
  xgboost-r PRIVATE
  ${LIBR_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/dmlc-core/include
)

target_link_libraries(xgboost-r PUBLIC ${LIBR_CORE_LIBRARY})

if(USE_OPENMP)
  find_package(OpenMP REQUIRED)
  target_link_libraries(xgboost-r PUBLIC OpenMP::OpenMP_CXX OpenMP::OpenMP_C)
endif()

set_target_properties(
  xgboost-r PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  POSITION_INDEPENDENT_CODE ON
)

# Get compilation and link flags of xgboost-r and propagate to objxgboost
target_link_libraries(objxgboost PUBLIC xgboost-r)

# Add all objects of xgboost-r to objxgboost
target_sources(objxgboost INTERFACE $<TARGET_OBJECTS:xgboost-r>)

set(LIBR_HOME "${LIBR_HOME}" PARENT_SCOPE)
set(LIBR_EXECUTABLE "${LIBR_EXECUTABLE}" PARENT_SCOPE)
