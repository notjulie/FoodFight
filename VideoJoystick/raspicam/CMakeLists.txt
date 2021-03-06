
# raspistill/raspivid/raspiyuv

SET(COMPILE_DEFINITIONS -Werror)

# Set --no-as-needed to stop the linker discarding mmal_vc_client
# as it can't see that the constructor registers a load of functionality
# with the MMAL core.
SET( CMAKE_EXE_LINKER_FLAGS "-Wl,--no-as-needed" )

include_directories(${PROJECT_SOURCE_DIR}/host_applications/linux/libs/bcm_host/include)
include_directories(${PROJECT_SOURCE_DIR}/host_applications/linux/apps/raspicam/)
include_directories(${PROJECT_SOURCE_DIR}/host_applications/linux/libs/sm)

set (GL_SCENE_SOURCES
   gl_scenes/models.c
   gl_scenes/mirror.c
   gl_scenes/yuv.c
   gl_scenes/sobel.c
   gl_scenes/square.c
   gl_scenes/teapot.c
   gl_scenes/vcsm_square.c)

set (COMMON_SOURCES
   RaspiCamControl.c
   RaspiCLI.c
   RaspiPreview.c)

add_executable(raspistill ${COMMON_SOURCES} RaspiStill.c  RaspiTex.c RaspiTexUtil.c tga.c ${GL_SCENE_SOURCES} libgps_loader.c)
add_executable(raspiyuv   ${COMMON_SOURCES} RaspiStillYUV.c)
add_executable(raspivid   ${COMMON_SOURCES} RaspiVid.c)
add_executable(raspividyuv  ${COMMON_SOURCES} RaspiVidYUV.c)

set (MMAL_LIBS mmal_core mmal_util mmal_vc_client)

target_link_libraries(raspistill ${MMAL_LIBS} vcos bcm_host brcmGLESv2 brcmEGL m dl)
target_link_libraries(raspiyuv   ${MMAL_LIBS} vcos bcm_host)
target_link_libraries(raspivid   ${MMAL_LIBS} vcos bcm_host)
target_link_libraries(raspividyuv   ${MMAL_LIBS} vcos bcm_host)

install(TARGETS raspistill raspiyuv raspivid raspividyuv RUNTIME DESTINATION bin)
