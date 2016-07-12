cmake_minimum_required(VERSION 2.8)
project(main)
SET(GCC_COVERAGE_COMPILE_FLAGS "-std=c++11")
#SET(COMPILE_DEFINITIONS "-Werror")
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
include_directories(${PROJECT_SOURCE_DIR}/source/tdogl)
include_directories(${PROJECT_SOURCE_DIR}/../../network)
include_directories(${PROJECT_SOURCE_DIR}/../../network/crc)
#link_directories(${PROJECT_SOURCE_DIR}/..)
#include_directories(${PROJECT_SOURCE_DIR}/src)
set( project_sources
   source/main.cpp
   source/tdogl/Bitmap.cpp
   source/tdogl/Program.cpp
   source/tdogl/Shader.cpp
   source/tdogl/Texture.cpp
   ../../network/NetworkControl.cpp
   ../../network/DebugTransfer.cpp
   ../../network/Packetbuffer.cpp
   ../../network/ReliableTransfer.cpp
   ../../network/SerialCon.cpp
   ../../network/Topology.cpp
   ../../network/UnreliableTransfer.cpp
   ../../network/crc/crc.c
)
find_package(glfw3 3.2 REQUIRED) 
find_package(GLEW REQUIRED) 
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
add_executable(${PROJECT_NAME}
  ${project_sources}
)

target_link_libraries(main GL GLU GLEW glfw)


#install(TARGETS main RUNTIME DESTINATION bin)
