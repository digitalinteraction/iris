
# raspistill/raspivid/raspiyuv
cmake_minimum_required(VERSION 3.0)

SET(COMPILE_DEFINITIONS -Werror)
SET(GCC_COVERAGE_LINK_FLAGS    "-pthread -lwiringPi -g")

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

set (GL_SCENE_SOURCES
   gl_scenes/models.c
   gl_scenes/mirror.c
   gl_scenes/yuv.c
   gl_scenes/sobel.c
   gl_scenes/square.c
   gl_scenes/teapot.c
   gl_scenes/own.c)

set (COMMON_SOURCES
   RaspiCamControl.c
   RaspiCLI.c
   RaspiPreview.c)

#add_executable(raspistill ${COMMON_SOURCES} RaspiStill.c  RaspiTex.c RaspiTexUtil.c tga.c ${GL_SCENE_SOURCES})
add_executable(testmain ${COMMON_SOURCES} testmain.cpp  RaspiTex.c RaspiTexUtil.c tga.c ${GL_SCENE_SOURCES})

set (MMAL_LIBS mmal_core mmal_util mmal_vc_client)

#target_link_libraries(raspistill ${MMAL_LIBS} vcos bcm_host GLESv2 EGL m)
target_link_libraries(testmain ${MMAL_LIBS} vcos bcm_host GLESv2 EGL m wiringPi)


#install(TARGETS raspistill RUNTIME DESTINATION bin)
install(TARGETS testmain RUNTIME DESTINATION bin)

