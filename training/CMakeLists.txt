cmake_minimum_required(VERSION 2.8)
project( main )
find_package( OpenCV REQUIRED )
include_directories(${PROJECT_SOURCE_DIR}/../network)
include_directories(${PROJECT_SOURCE_DIR}/../network/crc)
include_directories(${PROJECT_SOURCE_DIR}/../raspicam)
set( project_sources
   main.cpp
)

add_executable(${PROJECT_NAME}
  ${project_sources}
)
target_link_libraries( main ${OpenCV_LIBS} )
