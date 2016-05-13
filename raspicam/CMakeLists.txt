
# raspistill/raspivid/raspiyuv
cmake_minimum_required(VERSION 3.0)

SET(COMPILE_DEFINITIONS -Werror)
SET(GCC_COVERAGE_LINK_FLAGS    "-pthread -lwiringPi")

SET(GCC_COVERAGE_COMPILE_FLAGS "-std=c++11")
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )


SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
include_directories(${PROJECT_SOURCE_DIR}/../libs/bcm_host/include)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/..)
include_directories(${PROJECT_SOURCE_DIR}/../interface/vcos/pthreads)
include_directories(${PROJECT_SOURCE_DIR}/../wiringPi)
include_directories(/opt/vc/include/interface/vmcs_host/linux)
include_directories(usr/include)
include_directories(/opt/vc/include)
link_directories(/opt/vc/lib)

#find_package(OpenCV REQUIRED core imgproc highgui video)
find_package(OpenCV REQUIRED)

set (GL_SCENE_SOURCES
   gl_scenes/own.c)

set (COMMON_SOURCES
   RaspiCamControl.c
   RaspiCLI.c
   RaspiPreview.c)

#add_executable(raspistill ${COMMON_SOURCES} RaspiStill.c  RaspiTex.c RaspiTexUtil.c tga.c ${GL_SCENE_SOURCES})
add_executable(main ${COMMON_SOURCES} main.cpp Image_Capture.cpp Low_Res_Worker.cpp High_Res_Worker.cpp Buffer.cpp  RaspiTex.c RaspiTexUtil.c tga.c ${GL_SCENE_SOURCES})

set (MMAL_LIBS mmal_core mmal_util mmal_vc_client)

#target_link_libraries(raspistill ${MMAL_LIBS} vcos bcm_host GLESv2 EGL m)
target_link_libraries(main ${MMAL_LIBS} vcos bcm_host GLESv2 EGL m wiringPi ${OpenCV_LIBS})


#install(TARGETS raspistill RUNTIME DESTINATION bin)
install(TARGETS main RUNTIME DESTINATION bin)

