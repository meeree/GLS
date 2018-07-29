cmake_minimum_required (VERSION 2.6)
project (LSys)

set (DEBUG 1)
set (VERBOSE 1)
set (SYM_MAX 1000000)
set (PARAM_MAX 1000000)

configure_file (
    "${PROJECT_SOURCE_DIR}/src/lsys_config.hpp.in"
    "${PROJECT_BINARY_DIR}/lsys_config.hpp"
  )

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)

include_directories( "${PROJECT_BINARY_DIR}" "src/*.hpp" ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} )

file ( GLOB GLIB_SOURCE "src/*.cpp" ) 
 
# add the  executable
add_executable (lsys ${GLIB_SOURCE})
target_link_libraries (lsys ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLM_LIBRARIES} glfw)
